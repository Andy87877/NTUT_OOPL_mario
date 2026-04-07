/**
 * @file SceneManager.cpp
 * @brief Implementation of scene manager.
 * @inheritance None (Service)
 */
#include "Mario/SceneManager.hpp"

#include "Util/Logger.hpp"

namespace Mario {

void SceneManager::RegisterScene(const std::string& name,
                                 std::unique_ptr<ISceneHandler> handler) {
    if (!handler) {
        LOG_ERROR("Attempted to register null scene handler for '{}'", name);
        return;
    }
    m_RegisteredScenes[name] = std::move(handler);
    LOG_DEBUG("Registered scene: {}", name);
}

void SceneManager::PushScene(const std::string& name) {
    auto it = m_RegisteredScenes.find(name);
    if (it == m_RegisteredScenes.end()) {
        LOG_ERROR("Scene not registered: {}", name);
        return;
    }

    // Move scene from registry to stack (ownership transfer)
    auto scene = std::move(it->second);
    m_RegisteredScenes.erase(it);

    scene->OnEnter();
    m_SceneStack.push(std::move(scene));
    LOG_DEBUG("Pushed scene: {}", name);
}

void SceneManager::PopScene() {
    if (m_SceneStack.empty()) {
        LOG_WARN("Attempted to pop from empty scene stack");
        return;
    }

    auto scene = std::move(m_SceneStack.top());
    m_SceneStack.pop();
    scene->OnExit();
    LOG_DEBUG("Popped scene: {}", scene->GetName());
}

void SceneManager::ReplaceScene(const std::string& name) {
    if (!m_SceneStack.empty()) {
        PopScene();
    }
    PushScene(name);
}

bool SceneManager::Update() {
    if (m_SceneStack.empty()) {
        return false;  // No active scene, exit
    }

    auto& currentScene = m_SceneStack.top();
    if (!currentScene->Update()) {
        // Scene signals transition
        const char* nextScene = currentScene->GetNextSceneName();
        if (nextScene) {
            ReplaceScene(nextScene);
        }
        return !m_SceneStack.empty();
    }

    return true;  // Scene still active
}

ISceneHandler* SceneManager::GetCurrentScene() const {
    if (m_SceneStack.empty()) {
        return nullptr;
    }
    return m_SceneStack.top().get();
}

}  // namespace Mario
