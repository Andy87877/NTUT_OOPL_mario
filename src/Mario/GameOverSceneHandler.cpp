/**
 * @file GameOverSceneHandler.cpp
 * @brief Implementation of game over scene handler.
 * @inheritance ISceneHandler <- GameOverSceneHandler
 */
#include "Mario/GameOverSceneHandler.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

namespace Mario {

GameOverSceneHandler::GameOverSceneHandler() : m_DisplayTimer(0) {}

void GameOverSceneHandler::OnEnter() {
    LOG_INFO("Entered Game Over Scene");
    m_DisplayTimer = WAIT_FOR_INPUT;
}

bool GameOverSceneHandler::Update() {
    // Wait for input to restart
    if (Util::Input::IsKeyPressed(Util::Keycode::RETURN) ||
        Util::Input::IsKeyPressed(Util::Keycode::SPACE)) {
        m_NextScene = "TitleScene";
        return false;  // Signal transition
    }

    return true;  // Keep waiting
}

void GameOverSceneHandler::OnExit() { LOG_INFO("Exited Game Over Scene"); }

const char* GameOverSceneHandler::GetNextSceneName() const {
    return m_NextScene ? m_NextScene : "TitleScene";
}

}  // namespace Mario
