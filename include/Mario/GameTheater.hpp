/**
 * @file GameTheater.hpp
 * @brief Theater for managing scene stack and transitions.
 *        Central orchestrator for all scene changes and updates.
 * @inheritance None (Orchestrator)
 */
#ifndef MARIO_GAME_THEATER_HPP
#define MARIO_GAME_THEATER_HPP

#include <memory>

#include "Mario/ISceneHandler.hpp"
#include "Mario/SceneManager.hpp"

namespace Mario {

/**
 * Game theater manages the overall game loop using scene stack.
 * Acts as the primary orchestrator for:
 * - Scene initialization
 * - Updates
 * - Transitions
 * - Cleanup
 */
class GameTheater {
   public:
    GameTheater();
    virtual ~GameTheater() = default;

    /**
     * Initialize the theater with the starting scene.
     */
    void Initialize(const std::string& startScene);

    /**
     * Update current scene.
     * @return false if application should exit
     */
    bool Update();

    /**
     * Get current active scene manager.
     */
    SceneManager& GetSceneManager() { return m_SceneManager; }

    /**
     * Request scene transition.
     */
    void TransitionToScene(const std::string& sceneName);

    /**
     * Check if theater is running.
     */
    bool IsRunning() const { return !m_SceneManager.IsEmpty(); }

   private:
    SceneManager m_SceneManager;
};

}  // namespace Mario

#endif  // MARIO_GAME_THEATER_HPP
