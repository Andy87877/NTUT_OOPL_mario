/**
 * @file ESCMenuSceneHandler.hpp
 * @brief Pause/ESC menu scene handler (App::State::ESC_MENU).
 *        Navigate with UP/DOWN, confirm with ENTER, ESC resumes.
 * @inheritance ISceneHandler <- ESCMenuSceneHandler
 */
#ifndef MARIO_ESC_MENU_SCENE_HANDLER_HPP
#define MARIO_ESC_MENU_SCENE_HANDLER_HPP

#include "Mario/ISceneHandler.hpp"

namespace Mario {

class ESCMenuSceneHandler : public ISceneHandler {
   public:
    ESCMenuSceneHandler() = default;

    void Update(App& app) override;
    void OnRender(App& app) override;
    const char* GetName() const override { return "ESCMenuScene"; }
};

}  // namespace Mario

#endif  // MARIO_ESC_MENU_SCENE_HANDLER_HPP
