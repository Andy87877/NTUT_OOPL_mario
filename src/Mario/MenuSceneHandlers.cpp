/**
 * @file MenuSceneHandlers.cpp
 * @brief Implementations for all simple menu/transition scene handlers.
 *        Also includes ISceneHandler's default OnRender() implementation.
 *        Merged from: ISceneHandler.cpp, TitleSceneHandler.cpp,
 *        DeathSceneHandler.cpp, GameOverSceneHandler.cpp,
 *        GameWonSceneHandler.cpp.
 * @inheritance ISceneHandler <- TitleSceneHandler
 *              ISceneHandler <- DeathSceneHandler
 *              ISceneHandler <- GameOverSceneHandler
 *              ISceneHandler <- GameWonSceneHandler
 */
#include "Mario/MenuSceneHandlers.hpp"

#include "App.hpp"
#include "Mario/AudioManager.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

namespace Mario {

// ============================================================================
// ISceneHandler — default OnRender() (was ISceneHandler.cpp)
// ============================================================================

void ISceneHandler::OnRender(App& app) {
    // Default: flush the renderer with no UI override.
    // Concrete handlers call app.ApplyBackground() + UIManager as needed.
    app.GetRenderer().Update();
}

// ============================================================================
// TitleSceneHandler
// ============================================================================

void TitleSceneHandler::Update(App& app) {
    if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        app.GetGameState().NewGame();
        app.TransitionTo(App::State::LOADING);
        LOG_INFO("Starting game - entering LOADING state");
    }
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        app.TransitionTo(App::State::END);
    }
}

void TitleSceneHandler::OnRender(App& app) {
    app.ApplyBackground(false);  // Sky-blue title screen
    app.GetRenderer().Update();
    app.GetUIManager().Update(Mario::UIManager::State::TITLE);
}

// ============================================================================
// DeathSceneHandler
// ============================================================================

void DeathSceneHandler::Update(App& app) {
    if (app.GetTimer() > app.GetDeathTimer()) {
        if (!app.GetGameState().IsGameOver()) {
            app.TransitionTo(App::State::LOADING);
        } else {
            app.TransitionTo(App::State::GAME_OVER);
            Mario::AudioManager::GetInstance().PlayBGM(
                Mario::BGMName::GameOverTheme);
            LOG_INFO("No lives remaining - GAME_OVER");
        }
    }
}

void DeathSceneHandler::OnRender(App& app) {
    app.GetRenderer().Update();
    app.GetUIManager().Update(Mario::UIManager::State::PLAYING);
}

// ============================================================================
// GameOverSceneHandler
// ============================================================================

void GameOverSceneHandler::Update(App& app) {
    if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        app.TransitionTo(App::State::TITLE);
        LOG_INFO("Game Over - returning to TITLE");
    }
}

void GameOverSceneHandler::OnRender(App& app) {
    app.ApplyBackground(true);  // Black screen
    app.GetRenderer().Update();
    app.GetUIManager().Update(Mario::UIManager::State::GAME_OVER);
}

// ============================================================================
// GameWonSceneHandler
// ============================================================================

void GameWonSceneHandler::Update(App& app) {
    if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        app.TransitionTo(App::State::TITLE);
        LOG_INFO("Game Won! Returning to TITLE screen.");
    }
}

void GameWonSceneHandler::OnRender(App& app) {
    app.GetRenderer().Update();
    app.GetUIManager().Update(Mario::UIManager::State::GAME_WON);
}

}  // namespace Mario
