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

#include <memory>
#include <vector>
#include "Mario/Scenes/ISceneHandler.hpp"

namespace Mario {

class ITitleMenuItem;

// ============================================================================
// TitleSceneHandler — title / start screen (App::State::TITLE)
// ============================================================================
class TitleSceneHandler : public ISceneHandler {
   public:
    enum class SubState { MENU, CONTROLS };

    TitleSceneHandler();
    ~TitleSceneHandler() override;

    void OnEnter(App& app) override;
    void Update(App& app) override;
    void OnRender(App& app) override;
    const char* GetName() const override { return "TitleScene"; }

    void SetSubState(SubState state) { m_SubState = state; }
    SubState GetSubState() const { return m_SubState; }

   private:
    int m_Selection = 0;
    SubState m_SubState = SubState::MENU;
    std::vector<std::unique_ptr<ITitleMenuItem>> m_MenuItems;
};

// ============================================================================
// DeathSceneHandler — death animation pause (App::State::DEATH)
// ============================================================================
class DeathSceneHandler : public ISceneHandler {
   public:
    DeathSceneHandler() = default;
    void OnEnter(App& app) override;
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
    void OnEnter(App& app) override;
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
    void OnEnter(App& app) override;
    void Update(App& app) override;
    void OnRender(App& app) override;
    const char* GetName() const override { return "GameWonScene"; }
};

}  // namespace Mario

#endif  // MARIO_MENU_SCENE_HANDLERS_HPP
