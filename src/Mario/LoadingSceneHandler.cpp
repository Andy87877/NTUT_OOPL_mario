/**
 * @file LoadingSceneHandler.cpp
 * @brief Implementation of loading scene handler.
 * @inheritance ISceneHandler <- LoadingSceneHandler
 */
#include "Mario/LoadingSceneHandler.hpp"

#include "Util/Logger.hpp"

namespace Mario {

LoadingSceneHandler::LoadingSceneHandler(GameStateManager* gameState)
    : m_GameState(gameState), m_DisplayTimer(0) {}

void LoadingSceneHandler::OnEnter() {
    LOG_INFO("Entered Loading Scene - World {}-{}", m_GameState->GetWorldNum(),
             m_GameState->GetLevelNum());
    m_DisplayTimer = 0;
}

bool LoadingSceneHandler::Update() {
    m_DisplayTimer++;

    // Display for fixed duration before transitioning
    if (m_DisplayTimer >= DISPLAY_DURATION) {
        m_NextScene = "PlayingScene";
        return false;  // Signal transition
    }

    return true;  // Keep displaying
}

void LoadingSceneHandler::OnExit() { LOG_INFO("Exited Loading Scene"); }

const char* LoadingSceneHandler::GetNextSceneName() const {
    return m_NextScene ? m_NextScene : "PlayingScene";
}

}  // namespace Mario
