/**
 * @file MenuSceneHandlers.hpp
 * @brief All simple menu/transition ISceneHandler implementations.
 *        TitleSceneHandler, DeathSceneHandler, GameOverSceneHandler, and
 *        GameWonSceneHandler are consolidated here because each is under
 *        35 lines and contains only player-input detection + state transition
 *        logic — no complex game play.
 *
 *        ISceneHandler.cpp's default OnRender() is also compiled together
 *        in MenuSceneHandlers.cpp.
 *
 * @inheritance ISceneHandler <- TitleSceneHandler
 *              ISceneHandler <- DeathSceneHandler
 *              ISceneHandler <- GameOverSceneHandler
 *              ISceneHandler <- GameWonSceneHandler
 */
#ifndef MARIO_MENU_SCENE_HANDLERS_HPP
#define MARIO_MENU_SCENE_HANDLERS_HPP

#include "Mario/ISceneHandler.hpp"

namespace Mario {

// ============================================================================
// TitleSceneHandler — title / start screen (App::State::TITLE)
// ============================================================================
class TitleSceneHandler : public ISceneHandler {
   public:
    TitleSceneHandler() = default;
    void Update(App& app) override;
    void OnRender(App& app) override;
    const char* GetName() const override { return "TitleScene"; }
};

// ============================================================================
// DeathSceneHandler — death animation pause (App::State::DEATH)
// ============================================================================
class DeathSceneHandler : public ISceneHandler {
   public:
    DeathSceneHandler() = default;
    void Update(App& app) override;
    void OnRender(App& app) override;
    const char* GetName() const override { return "DeathScene"; }
};

// ============================================================================
// GameOverSceneHandler — "GAME OVER" screen (App::State::GAME_OVER)
// ============================================================================
class GameOverSceneHandler : public ISceneHandler {
   public:
    GameOverSceneHandler() = default;
    void Update(App& app) override;
    void OnRender(App& app) override;
    const char* GetName() const override { return "GameOverScene"; }
};

// ============================================================================
// GameWonSceneHandler — "YOU WIN" screen (App::State::GAME_WON)
// ============================================================================
class GameWonSceneHandler : public ISceneHandler {
   public:
    GameWonSceneHandler() = default;
    void Update(App& app) override;
    void OnRender(App& app) override;
    const char* GetName() const override { return "GameWonScene"; }
};

}  // namespace Mario

#endif  // MARIO_MENU_SCENE_HANDLERS_HPP
