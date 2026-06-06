/**
 * @file ESCMenuSceneHandler.hpp
 * @brief Pause/ESC menu scene handler (App::State::ESC_MENU).
 *        Navigate with UP/DOWN, confirm with ENTER, ESC resumes.
 * @inheritance ISceneHandler <- ESCMenuSceneHandler
 */
#ifndef MARIO_ESC_MENU_SCENE_HANDLER_HPP
#define MARIO_ESC_MENU_SCENE_HANDLER_HPP

#include <memory>
#include <vector>

#include "Mario/Scenes/ISceneHandler.hpp"
#include "Mario/UI/IESCMenuItem.hpp"

namespace Mario {

class ESCMenuSceneHandler : public ISceneHandler {
   public:
    enum class SubState { MENU, CONTROLS };

    ESCMenuSceneHandler();
    ~ESCMenuSceneHandler() override = default;

    void OnEnter(App& app) override;
    void Update(App& app) override;
    void OnRender(App& app) override;
    const char* GetName() const override { return "ESCMenuScene"; }

    void SetSubState(SubState state) { m_SubState = state; }
    SubState GetSubState() const { return m_SubState; }

   private:
    SubState m_SubState = SubState::MENU;
    std::vector<std::unique_ptr<IESCMenuItem>> m_MenuItems;
};

}  // namespace Mario

#endif  // MARIO_ESC_MENU_SCENE_HANDLER_HPP
