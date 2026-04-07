/**
 * @file DeathSceneHandler.cpp
 * @brief Implementation of death scene handler.
 * @inheritance ISceneHandler <- DeathSceneHandler
 */
#include "Mario/DeathSceneHandler.hpp"

#include "Util/Logger.hpp"

namespace Mario {

DeathSceneHandler::DeathSceneHandler(GameStateManager* gameState)
    : m_GameState(gameState), m_DisplayTimer(0) {}

void DeathSceneHandler::OnEnter() {
    LOG_INFO("Entered Death Scene - Lives remaining: {}",
             m_GameState->GetLives());
    m_DisplayTimer = 0;
}

bool DeathSceneHandler::Update() {
    m_DisplayTimer++;

    // Display death screen for fixed duration
    if (m_DisplayTimer >= DISPLAY_DURATION) {
        if (m_GameState->IsGameOver()) {
            m_NextScene = "GameOverScene";
        } else {
            m_NextScene = "LoadingScene";
        }
        return false;  // Signal transition
    }

    return true;  // Keep displaying
}

void DeathSceneHandler::OnExit() { LOG_INFO("Exited Death Scene"); }

const char* DeathSceneHandler::GetNextSceneName() const {
    return m_NextScene ? m_NextScene : "LoadingScene";
}

}  // namespace Mario
