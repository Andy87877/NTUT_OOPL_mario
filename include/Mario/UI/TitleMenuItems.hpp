/**
 * @file TitleMenuItems.hpp
 * @brief Concrete implementations of title menu options (Command Pattern).
 * @inheritance ITitleMenuItem -> StartGameMenuItem, ControlsMenuItem, QuitGameMenuItem
 */
#ifndef MARIO_TITLE_MENU_ITEMS_HPP
#define MARIO_TITLE_MENU_ITEMS_HPP

#include "Mario/UI/ITitleMenuItem.hpp"

namespace Mario {

class StartGameMenuItem : public ITitleMenuItem {
   public:
    std::string GetDisplayText(App& app) const override;
    void Execute(App& app, TitleSceneHandler& handler) override;
};

class ControlsMenuItem : public ITitleMenuItem {
   public:
    std::string GetDisplayText(App& app) const override;
    void Execute(App& app, TitleSceneHandler& handler) override;
};

class QuitGameMenuItem : public ITitleMenuItem {
   public:
    std::string GetDisplayText(App& app) const override;
    void Execute(App& app, TitleSceneHandler& handler) override;
};

}  // namespace Mario

#endif  // MARIO_TITLE_MENU_ITEMS_HPP
