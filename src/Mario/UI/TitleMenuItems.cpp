/**
 * @file TitleMenuItems.cpp
 * @brief Concrete implementations of title menu options (Command Pattern).
 * @inheritance ITitleMenuItem -> StartGameMenuItem, ControlsMenuItem, QuitGameMenuItem
 */
#include "Mario/UI/TitleMenuItems.hpp"

#include "App.hpp"
#include "Mario/Scenes/MenuSceneHandlers.hpp"
#include "Util/Logger.hpp"

namespace Mario {

// ============================================================================
// StartGameMenuItem
// ============================================================================
std::string StartGameMenuItem::GetDisplayText(App&) const {
    return "1 PLAYER GAME";
}

void StartGameMenuItem::Execute(App& app, TitleSceneHandler&) {
    app.GetGameState().NewGame();
    app.TransitionTo(App::State::LOADING);
    LOG_INFO("Starting game - entering LOADING state");
}

// ============================================================================
// ControlsMenuItem
// ============================================================================
std::string ControlsMenuItem::GetDisplayText(App&) const {
    return "CONTROLS";
}

void ControlsMenuItem::Execute(App&, TitleSceneHandler& handler) {
    handler.SetSubState(TitleSceneHandler::SubState::CONTROLS);
    LOG_INFO("Opening controls instruction guide");
}

// ============================================================================
// QuitGameMenuItem
// ============================================================================
std::string QuitGameMenuItem::GetDisplayText(App&) const {
    return "QUIT GAME";
}

void QuitGameMenuItem::Execute(App& app, TitleSceneHandler&) {
    app.TransitionTo(App::State::END);
    LOG_INFO("Exiting game from title screen selection");
}

}  // namespace Mario
