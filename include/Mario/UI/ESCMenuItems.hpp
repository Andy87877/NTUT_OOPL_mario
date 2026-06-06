/**
 * @file ESCMenuItems.hpp
 * @brief Concrete implementations of pause menu options (Command Pattern).
 * @inheritance IESCMenuItem -> ResumeMenuItem, LevelWarpMenuItem, PowerCheatMenuItem, CheatToggleMenuItem
 */
#ifndef MARIO_ESC_MENU_ITEMS_HPP
#define MARIO_ESC_MENU_ITEMS_HPP

#include "Mario/UI/IESCMenuItem.hpp"

namespace Mario {

class ResumeMenuItem : public IESCMenuItem {
   public:
    std::string GetDisplayText(App& app) const override;
    std::string GetDescriptionText() const override;
    void Execute(App& app, ESCMenuSceneHandler& handler) override;
};

class LevelWarpMenuItem : public IESCMenuItem {
   public:
    LevelWarpMenuItem(int world, int level);
    std::string GetDisplayText(App& app) const override;
    std::string GetDescriptionText() const override;
    void Execute(App& app, ESCMenuSceneHandler& handler) override;

   private:
    int m_World;
    int m_Level;
};

class PowerCheatMenuItem : public IESCMenuItem {
   public:
    std::string GetDisplayText(App& app) const override;
    std::string GetDescriptionText() const override;
    void Execute(App& app, ESCMenuSceneHandler& handler) override;
};

class CheatToggleMenuItem : public IESCMenuItem {
   public:
    std::string GetDisplayText(App& app) const override;
    std::string GetDescriptionText() const override;
    void Execute(App& app, ESCMenuSceneHandler& handler) override;
};

class ControlsESCMenuItem : public IESCMenuItem {
   public:
    std::string GetDisplayText(App& app) const override;
    std::string GetDescriptionText() const override;
    void Execute(App& app, ESCMenuSceneHandler& handler) override;
};

}  // namespace Mario

#endif  // MARIO_ESC_MENU_ITEMS_HPP
