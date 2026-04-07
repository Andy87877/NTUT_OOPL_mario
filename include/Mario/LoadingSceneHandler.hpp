/**
 * @file LoadingSceneHandler.hpp
 * @brief Loading screen scene handler.
 *        Shows "WORLD X-X" banner and lives display before level starts.
 * @inheritance ISceneHandler
 */
#ifndef MARIO_LOADING_SCENE_HANDLER_HPP
#define MARIO_LOADING_SCENE_HANDLER_HPP

#include "Mario/GameStateManager.hpp"
#include "Mario/ISceneHandler.hpp"

namespace Mario {

/**
 * Manages the loading/intro scene before each level.
 * Displays level info, then transitions to PLAYING scene.
 */
class LoadingSceneHandler : public ISceneHandler {
   public:
    LoadingSceneHandler(GameStateManager* gameState);
    virtual ~LoadingSceneHandler() = default;

    void OnEnter() override;
    bool Update() override;
    void OnExit() override;

    const char* GetNextSceneName() const override;
    const char* GetName() const override { return "LoadingScene"; }

   private:
    GameStateManager* m_GameState;
    int m_DisplayTimer = 0;
    static constexpr int DISPLAY_DURATION = 120;  // 2 seconds at 60fps
    const char* m_NextScene = nullptr;
};

}  // namespace Mario

#endif  // MARIO_LOADING_SCENE_HANDLER_HPP
