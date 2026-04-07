/**
 * @file DeathSceneHandler.hpp
 * @brief Death scene handler - shows Mario death animation and "GAME OVER"
 * text.
 * @inheritance ISceneHandler
 */
#ifndef MARIO_DEATH_SCENE_HANDLER_HPP
#define MARIO_DEATH_SCENE_HANDLER_HPP

#include "Mario/GameStateManager.hpp"
#include "Mario/ISceneHandler.hpp"

namespace Mario {

/**
 * Manages the death scene when Mario dies.
 * Displays the death animation overlay and waits before transitioning.
 */
class DeathSceneHandler : public ISceneHandler {
   public:
    DeathSceneHandler(GameStateManager* gameState);
    virtual ~DeathSceneHandler() = default;

    void OnEnter() override;
    bool Update() override;
    void OnExit() override;

    const char* GetNextSceneName() const override;
    const char* GetName() const override { return "DeathScene"; }

   private:
    GameStateManager* m_GameState;
    int m_DisplayTimer = 0;
    static constexpr int DISPLAY_DURATION = 90;  // ~1.5 seconds at 60fps
    const char* m_NextScene = nullptr;
};

}  // namespace Mario

#endif  // MARIO_DEATH_SCENE_HANDLER_HPP
