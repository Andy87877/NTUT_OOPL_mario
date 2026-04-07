/**
 * @file SceneManager.hpp
 * @brief Manager for scene transitions using the Strategy pattern.
 *        Handles pushing/popping scenes and transitioning between them.
 * @inheritance None (Service locator)
 */
#ifndef MARIO_SCENE_MANAGER_HPP
#define MARIO_SCENE_MANAGER_HPP

#include <map>
#include <memory>
#include <stack>
#include <string>

#include "Mario/ISceneHandler.hpp"

namespace Mario {

/**
 * Centralized scene management using a stack-based approach.
 * Supports dynamic scene registration and transitions.
 */
class SceneManager {
   public:
    SceneManager() = default;
    virtual ~SceneManager() = default;

    /**
     * Register a scene handler by name.
     */
    void RegisterScene(const std::string& name,
                       std::unique_ptr<ISceneHandler> handler);

    /**
     * Push a scene onto the stack (makes it active).
     */
    void PushScene(const std::string& name);

    /**
     * Pop the current scene from the stack.
     */
    void PopScene();

    /**
     * Replace current scene with a new one.
     */
    void ReplaceScene(const std::string& name);

    /**
     * Update the current active scene.
     * @return false if transition should occur
     */
    bool Update();

    /**
     * Get currently active scene.
     */
    ISceneHandler* GetCurrentScene() const;

    /**
     * Check if scene stack is empty.
     */
    bool IsEmpty() const { return m_SceneStack.empty(); }

   private:
    std::stack<std::unique_ptr<ISceneHandler>> m_SceneStack;
    std::map<std::string, std::unique_ptr<ISceneHandler>> m_RegisteredScenes;
    std::string m_PendingTransition;
    bool m_ShouldPopScene = false;
};

}  // namespace Mario

#endif  // MARIO_SCENE_MANAGER_HPP
