/**
 * @file ESCMenuSceneHandler.cpp
 * @brief Implementation of ESC menu scene handler.
 * @inheritance ISceneHandler <- ESCMenuSceneHandler
 */
#include "Mario/ESCMenuSceneHandler.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

namespace Mario {

ESCMenuSceneHandler::ESCMenuSceneHandler(GameStateManager* gameState)
    : m_GameState(gameState), m_Selection(0) {}

void ESCMenuSceneHandler::OnEnter() {
    LOG_INFO("Entered ESC Menu Scene");
    m_Selection = 0;  // Default to Resume
}

bool ESCMenuSceneHandler::Update() {
    // Handle menu navigation
    if (Util::Input::IsKeyPressed(Util::Keycode::UP)) {
        m_Selection = (m_Selection - 1 + 4) % 4;
    }
    if (Util::Input::IsKeyPressed(Util::Keycode::DOWN)) {
        m_Selection = (m_Selection + 1) % 4;
    }

    // Handle selection
    if (Util::Input::IsKeyPressed(Util::Keycode::RETURN) ||
        Util::Input::IsKeyPressed(Util::Keycode::SPACE)) {
        if (m_Selection == 0) {
            // Resume game
            m_NextScene = "PlayingScene";
            return false;
        } else if (m_Selection == 1) {
            // Jump to 1-1
            m_GameState->SetLevel(1, 1);
            m_NextScene = "LoadingScene";
            return false;
        } else if (m_Selection == 2) {
            // Jump to 1-2
            m_GameState->SetLevel(1, 2);
            m_NextScene = "LoadingScene";
            return false;
        } else if (m_Selection == 3) {
            // Jump to 8-4
            m_GameState->SetLevel(8, 4);
            m_NextScene = "LoadingScene";
            return false;
        }
    }

    // ESC key to return to game
    if (Util::Input::IsKeyPressed(Util::Keycode::ESCAPE)) {
        m_NextScene = "PlayingScene";
        return false;
    }

    return true;  // Keep showing menu
}

void ESCMenuSceneHandler::OnExit() { LOG_INFO("Exited ESC Menu Scene"); }

const char* ESCMenuSceneHandler::GetNextSceneName() const {
    return m_NextScene ? m_NextScene : "PlayingScene";
}

}  // namespace Mario
