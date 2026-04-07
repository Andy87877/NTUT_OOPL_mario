/**
 * @file GameOverSceneHandler.hpp
 * @brief Game Over scene handler - final screen when all lives are lost.
 * @inheritance ISceneHandler
 */
#ifndef MARIO_GAME_OVER_SCENE_HANDLER_HPP
#define MARIO_GAME_OVER_SCENE_HANDLER_HPP

#include "Mario/ISceneHandler.hpp"

namespace Mario {

/**
 * Manages the game over scene when the player runs out of lives.
 * Displays "GAME OVER" message and waits for restart input.
 */
class GameOverSceneHandler : public ISceneHandler {
   public:
    GameOverSceneHandler();
    virtual ~GameOverSceneHandler() = default;

    void OnEnter() override;
    bool Update() override;
    void OnExit() override;

    const char* GetNextSceneName() const override;
    const char* GetName() const override { return "GameOverScene"; }

   private:
    int m_DisplayTimer = 0;
    static constexpr int WAIT_FOR_INPUT = -1;
    const char* m_NextScene = nullptr;
};

}  // namespace Mario

#endif  // MARIO_GAME_OVER_SCENE_HANDLER_HPP
