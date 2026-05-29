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
#include "Mario/Level/EntityDef.hpp"
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
#include "Mario/Services/LevelManager.hpp"
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
    // Instantiate and register LevelManager service
    m_LevelService = std::make_shared<Mario::LevelManager>();
    Mario::ServiceLocator::GetInstance().RegisterService<Mario::ILevelService>(m_LevelService);

    // Concrete keyboard controller injected here (DIP: App.hpp only knows
    // IInputHandler)
    m_InputHandler = std::make_unique<Mario::InputHandler>();
    m_UIManager = std::make_unique<Mario::UIManager>(&m_GameState);

    // Register GameStateManager in ServiceLocator (DIP / Service Locator)
    auto gameStateService = std::shared_ptr<Mario::GameStateManager>(&m_GameState, [](Mario::GameStateManager*) {});
    Mario::ServiceLocator::GetInstance().RegisterService<Mario::GameStateManager>(gameStateService);

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
        GetPlayer() = nullptr;
        GetLevel() = nullptr;
        GetEntities().clear();
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
// Level Management (Delegated to ILevelService / LevelManager)
// ============================================================================

void App::LoadLevel(const std::string& levelName) {
    m_LevelService->LoadLevel(*this, levelName);
}

void App::PlayCurrentBGM() {
    m_LevelService->PlayCurrentBGM(*this);
}

void App::AddEntityToGame(std::shared_ptr<Mario::Entity> entity) {
    m_LevelService->AddEntityToGame(*this, entity);
}

void App::StartLevel() {
    m_LevelService->StartLevel(*this);
}

bool App::IsUnderground() const {
    return m_LevelService->IsUnderground(const_cast<App&>(*this));
}

void App::ApplyBackground() {
    m_LevelService->ApplyBackground(*this);
}

void App::ApplyBackground(bool isUnderground) {
    m_LevelService->ApplyBackground(*this, isUnderground);
}

void App::AdvanceToNextLevel() {
    m_LevelService->AdvanceToNextLevel(*this);
}

// ============================================================================
// End
// ============================================================================
void App::End() { LOG_TRACE("End"); }

// NOTE: CheckFlagpoleCollision, CheckPipeCollision, CheckAxeCollision,
//       CheckPlayerEntityCollision, CheckEntityEntityCollision, and
//       CleanupDeadEntities have been moved to PlayingSceneHandler — they are
//       game-logic decisions that only the PLAYING state ever needs.
