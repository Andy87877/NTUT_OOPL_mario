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
#include "Mario/Scenes/AxeSequenceSceneHandler.hpp"
#include "Mario/Scenes/ESCMenuSceneHandler.hpp"
#include "Mario/Scenes/FlagpoleSceneHandler.hpp"
#include "Mario/Services/InputHandler.hpp"  // concrete keyboard controller (DIP)
#include "Mario/Scenes/LoadingSceneHandler.hpp"
#include "Mario/Scenes/MenuSceneHandlers.hpp"
#include "Mario/Core/PhysicsEngine.hpp"
#include "Mario/Scenes/PipeWarpSceneHandler.hpp"
#include "Mario/Scenes/PlayingSceneHandler.hpp"
#include "Mario/Services/ServiceLocator.hpp"
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
    // Concrete keyboard controller injected here (DIP: App.hpp only knows
    // IInputHandler)
    m_InputHandler = std::make_unique<Mario::InputHandler>();
    m_UIManager = std::make_unique<Mario::UIManager>(&m_GameState);

    // Register IAudioService singleton with the ServiceLocator (DIP / Service
    // Locator pattern)
    auto audioService = std::shared_ptr<Mario::IAudioService>(
        &Mario::AudioManager::GetInstance(), [](Mario::IAudioService*) {});
    Mario::ServiceLocator::GetInstance().RegisterService<Mario::IAudioService>(
        audioService);

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
    if (next == State::TITLE) {
        m_Player = nullptr;
        m_Level = nullptr;
        m_Entities.clear();
        m_Renderer = Util::Renderer();
        m_Camera.Reset();
    }
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
    //   GameConfig::Z_BACKGROUND = background tiles (-10.0f)
    //   GameConfig::Z_BLOCK      = solid tiles (-5.0f)
    //   1.0f = entities / enemies (in front of tiles)
    //   2.0f = player (always in front of everything except UI)
    // During pipe warp entry the player is dropped to GameConfig::Z_BLOCK
    // - 1.0f (-6.0f) so it sinks behind the pipe tiles, giving the correct
    // "enter pipe" visual.
    m_Renderer = Util::Renderer();
    for (const auto& block : m_Level->GetAllBlocks()) {
        m_Renderer.AddChild(block);
    }
    m_Renderer.AddChild(m_Player);
    for (const auto& entity : m_Entities) {
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

    // Level knows its own background type; GameStateManager tracks the runtime
    // pipe-warp underground flag. App::IsUnderground() merges both.
    ApplyBackground();

    LOG_INFO("Level {} loaded: {} blocks, {} entities, player at ({}, {})",
             levelName, m_Level->GetAllBlocks().size(), m_Entities.size(),
             spawnX, spawnY);
}

void App::PlayCurrentBGM() {
    // BGM selection logic lives in AudioManager — App is just the coordinator.
    Mario::AudioManager::GetInstance().PlayBGMForLevel(
        m_CurrentLevelName, m_GameState.GetTimeRemaining());
}

void App::AddEntityToGame(std::shared_ptr<Mario::Entity> entity) {
    if (!entity) return;
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
// IsUnderground — combines Level name detection + runtime pipe-warp flag.
// Single authoritative query; scene handlers should use this instead of
// duplicating the level-name check.
// ============================================================================
bool App::IsUnderground() const {
    if (m_GameState.IsUnderground()) return true;
    if (m_Level) return m_Level->IsUnderground();
    return false;
}

// ============================================================================
// ApplyBackground — sets the OpenGL clear colour for the current frame.
// The no-arg overload is the preferred form for scene handlers.
// ============================================================================
void App::ApplyBackground() { ApplyBackground(IsUnderground()); }

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
        LOG_INFO("Game Complete! Entering victory screen.");
        TransitionTo(State::GAME_WON);
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
