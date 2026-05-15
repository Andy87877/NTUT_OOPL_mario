/**
 * @file App.cpp
 * @brief Main application controller implementation.
 *        App is a thin coordinator: it owns all subsystems and exposes a
 *        clean public API that ISceneHandler subclasses use to drive the game.
 *
 *        Per-frame flow:
 *          1. m_CurrentHandler->Update(*this)    -- all game logic
 *          2. m_CurrentHandler->OnRender(*this)  -- background + renderer + UI
 *
 *        State Pattern (GoF): App::State enum + ISceneHandler hierarchy.
 *        MVC: InputHandler (C), PlayerState/EntityState (M), Player/Entity (V).
 * @inheritance None (top-level controller)
 */
#include "App.hpp"

// Scene handlers — included here so CreateSceneHandler() can instantiate them.
#include "Mario/AxeSequenceSceneHandler.hpp"
#include "Mario/ESCMenuSceneHandler.hpp"
#include "Mario/FlagpoleSceneHandler.hpp"
#include "Mario/LoadingSceneHandler.hpp"
#include "Mario/MenuSceneHandlers.hpp"
#include "Mario/PhysicsEngine.hpp"
#include "Mario/PipeWarpSceneHandler.hpp"
#include "Mario/PlayingSceneHandler.hpp"
#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

// OpenGL for background clear-colour
#include <GL/glew.h>

// ============================================================================
// Start
// ============================================================================
void App::Start() {
    LOG_TRACE("Start");
    m_UIManager = std::make_unique<Mario::UIManager>(&m_GameState);
    TransitionTo(State::TITLE);
    LOG_INFO("Game initialized - entering TITLE state");
}

// ============================================================================
// Update — entirely delegated to the active scene handler
// ============================================================================
void App::Update() {
    m_Timer++;
    if (m_CurrentHandler) m_CurrentHandler->Update(*this);
    // Each handler owns its own render: background color + renderer + UI.
    if (m_CurrentHandler)
        m_CurrentHandler->OnRender(*this);
    else
        m_Renderer.Update();
    if (Util::Input::IfExit()) TransitionTo(State::END);
}

// ============================================================================
// TransitionTo — swap active scene handler, fire lifecycle hooks
// ============================================================================
void App::TransitionTo(State next) {
    if (m_CurrentHandler) m_CurrentHandler->OnExit(*this);
    m_CurrentState = next;
    m_CurrentHandler = CreateSceneHandler(next);
    if (m_CurrentHandler) {
        m_CurrentHandler->OnEnter(*this);
        LOG_DEBUG("State -> {}", m_CurrentHandler->GetName());
    }
}

// ============================================================================
// CreateSceneHandler — factory for all ISceneHandler subclasses
// ============================================================================
std::unique_ptr<Mario::ISceneHandler> App::CreateSceneHandler(State s) {
    using namespace Mario;
    switch (s) {
        case State::TITLE:
            return std::make_unique<TitleSceneHandler>();
        case State::LOADING:
            return std::make_unique<LoadingSceneHandler>();
        case State::PLAYING:
            return std::make_unique<PlayingSceneHandler>();
        case State::FLAGPOLE:
            return std::make_unique<FlagpoleSceneHandler>();
        case State::PIPE_WARP:
            return std::make_unique<PipeWarpSceneHandler>();
        case State::AXE_SEQUENCE:
            return std::make_unique<AxeSequenceSceneHandler>();
        case State::DEATH:
            return std::make_unique<DeathSceneHandler>();
        case State::GAME_OVER:
            return std::make_unique<GameOverSceneHandler>();
        case State::GAME_WON:
            return std::make_unique<GameWonSceneHandler>();
        case State::ESC_MENU:
            return std::make_unique<ESCMenuSceneHandler>();
        default:
            return nullptr;  // START / END have no handler
    }
}

// ============================================================================
// Level Management
// ============================================================================

void App::LoadLevel(const std::string& levelName) {
    m_Camera.Reset();
    m_LevelCompleteCtrl.Reset();
    m_FlagEntity = nullptr;
    m_CurrentLevelName = levelName;

    // Create and load the level
    m_Level = std::make_shared<Mario::Level>();
    if (!m_Level->Load(levelName)) {
        LOG_ERROR("Failed to load level: {}", levelName);
        TransitionTo(State::TITLE);
        return;
    }

    // Create player at spawn position from level CSV
    float spawnX = m_Level->GetPlayerSpawnX();
    float spawnY = m_Level->GetPlayerSpawnY();

    // Restore saved power state across levels
    int savedState = m_GameState.GetSavedPowerState();
    m_Player = std::make_shared<Mario::Player>(spawnX, spawnY, savedState);

    // Spawn entities (Goomba, KoopaTroopa, etc.) from level data.
    // EntityFactory::SpawnFromLevel() also handles level-specific hardcoded
    // entities (e.g. 8-4 Podoboos) so App does not need per-level logic here.
    m_Entities = Mario::EntityFactory::SpawnFromLevel(*m_Level);

    // Bowser  = ID 847 (BowserSpawn)  at row=9, col=58
    // Axe     = ID 849 (AxeTrigger)   at row=9, col=65
    // Princess= ID 879 (PrincessSpawn) at row=9, col=72
    // Player  = ID 999 (MarioStart)   at row=9, col=3

    // Look for the Flag entity in spawn list
    for (auto& entity : m_Entities) {
        if (entity->GetState().GetName() == "Flag") {
            m_FlagEntity = entity;
            break;
        }
    }

    // Build renderer: clear and add all blocks + player + entities.
    // ZIndex hierarchy (higher = rendered in front):
    //   0.0f = tile blocks (background layer)
    //   1.0f = entities / enemies (in front of tiles)
    //   2.0f = player (always in front of everything except UI)
    // During pipe warp entry the player is dropped to -1.0f so it sinks behind
    // the pipe tiles, giving the correct "enter pipe" visual.
    m_Renderer = Util::Renderer();
    for (const auto& block : m_Level->GetAllBlocks()) {
        block->SetZIndex(0.0f);
        m_Renderer.AddChild(block);
    }
    m_Player->SetZIndex(2.0f);
    m_Renderer.AddChild(m_Player);
    for (const auto& entity : m_Entities) {
        entity->SetZIndex(1.0f);
        m_Renderer.AddChild(entity);
    }

    // Hide all game-world objects during the loading/transition screen.
    // The UIManager's MarioPreview sprite (a separate UIImage) is what shows
    // on the loading screen; the actual Player and Entities are revealed in
    // StartLevel() once gameplay begins.  This prevents enemies from being
    // visible in the background while "WORLD X-X" is displayed.
    m_Player->SetVisible(false);
    for (const auto& entity : m_Entities) {
        entity->SetVisible(false);
    }

    // Set OpenGL clear color for the level type (underground = black, surface =
    // sky blue). Delegated to ApplyBackground() to avoid duplicating the color
    // mapping here.
    bool isUnderground = m_GameState.IsUnderground() ||
                         levelName.find("u") != std::string::npos ||
                         levelName == "1-2" || levelName == "8-4";
    ApplyBackground(isUnderground);

    LOG_INFO("Level {} loaded: {} blocks, {} entities, player at ({}, {})",
             levelName, m_Level->GetAllBlocks().size(), m_Entities.size(),
             spawnX, spawnY);
}

void App::PlayCurrentBGM() {
    int time = m_GameState.GetTimeRemaining();
    bool hurry = time <= 100 && time > 0;
    std::string lvl = m_CurrentLevelName;

    Mario::BGMName bgm = Mario::BGMName::GroundTheme;
    if (lvl == "8-4" || lvl == "8-4_sub") {
        bgm = hurry ? Mario::BGMName::CastleThemeHurryUp
                    : Mario::BGMName::CastleTheme;
    } else if (lvl == "1-1u" || lvl == "1-2" || lvl == "1-2uu" ||
               lvl == "1-2_sub") {
        bgm = hurry ? Mario::BGMName::UndergroundThemeHurryUp
                    : Mario::BGMName::UndergroundTheme;
    } else {
        bgm = hurry ? Mario::BGMName::GroundThemeHurryUp
                    : Mario::BGMName::GroundTheme;
    }
    Mario::AudioManager::GetInstance().PlayBGM(bgm);
}

void App::AddEntityToGame(std::shared_ptr<Mario::Entity> entity) {
    // Register with the renderer so it is drawn, then track it in the entity
    // list so it receives Update/Tick calls.  This is the correct path for any
    // entity spawned AFTER LoadLevel() (brick debris, fireballs, etc.).
    if (!entity) return;
    entity->SetZIndex(1.0f);
    m_Renderer.AddChild(entity);
    m_Entities.push_back(entity);
}

void App::StartLevel() {
    m_GameState.ResetTime();
    m_GameState.StartTime();

    // Reveal game-world objects hidden during the loading screen.
    if (m_Player) {
        m_Player->SetVisible(true);
        m_Player->GetState().SetControllable(true);
    }
    for (const auto& entity : m_Entities) {
        entity->SetVisible(true);
    }

    PlayCurrentBGM();
}

// ============================================================================
// ApplyBackground — helper for ISceneHandler::OnRender() implementations.
// Concrete handlers call this to set the correct OpenGL clear color before
// flushing the renderer.  Centralises the level-type → color mapping so no
// handler needs to include <GL/glew.h> or duplicate this logic.
// ============================================================================
void App::ApplyBackground(bool isUnderground) {
    if (isUnderground) {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  // Castle / dungeon: black
    } else {
        glClearColor(92.0f / 255.0f, 148.0f / 255.0f, 252.0f / 255.0f,
                     0.0f);  // Surface: sky blue
    }
}

void App::AdvanceToNextLevel() {
    std::string nextLevel = m_GameState.AdvanceLevel();
    if (m_GameState.IsGameWon()) {
        LOG_INFO("Game Complete! Returning to title.");
        TransitionTo(State::TITLE);
        return;
    }
    LOG_INFO("Advancing to level {}", nextLevel);
    m_Loading = false;
    TransitionTo(State::LOADING);
}

// ============================================================================
// End
// ============================================================================
void App::End() { LOG_TRACE("End"); }
// NOTE: CheckFlagpoleCollision, CheckPipeCollision, CheckAxeCollision,
//       CheckPlayerEntityCollision, CheckEntityEntityCollision, and
//       CleanupDeadEntities have been moved to PlayingSceneHandler — they are
//       game-logic decisions that only the PLAYING state ever needs.
