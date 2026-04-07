/**
 * @file GameTheater.cpp
 * @brief Implementation of game theater.
 * @inheritance None (Orchestrator)
 */
#include "Mario/GameTheater.hpp"

#include "Util/Logger.hpp"

namespace Mario {

GameTheater::GameTheater() { LOG_DEBUG("GameTheater initialized"); }

void GameTheater::Initialize(const std::string& startScene) {
    LOG_INFO("GameTheater initializing with scene: {}", startScene);
    m_SceneManager.PushScene(startScene);
}

bool GameTheater::Update() {
    if (m_SceneManager.IsEmpty()) {
        LOG_INFO("Scene stack is empty, exiting theater");
        return false;
    }

    return m_SceneManager.Update();
}

void GameTheater::TransitionToScene(const std::string& sceneName) {
    LOG_INFO("Requesting scene transition to: {}", sceneName);
    m_SceneManager.ReplaceScene(sceneName);
}

}  // namespace Mario
