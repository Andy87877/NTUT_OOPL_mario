/**
 * @file ESCMenuSceneHandler.cpp
 * @brief Pause/ESC menu scene handler implementation using the Command Pattern.
 * @inheritance ISceneHandler <- ESCMenuSceneHandler
 */
#include "Mario/Scenes/ESCMenuSceneHandler.hpp"

#include "App.hpp"
#include "Mario/UI/ESCMenuItems.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

namespace Mario {

ESCMenuSceneHandler::ESCMenuSceneHandler() {
    m_MenuItems.push_back(std::make_unique<ResumeMenuItem>());
    m_MenuItems.push_back(std::make_unique<ControlsESCMenuItem>());
    m_MenuItems.push_back(std::make_unique<LevelWarpMenuItem>(1, 1));
    m_MenuItems.push_back(std::make_unique<LevelWarpMenuItem>(1, 2));
    m_MenuItems.push_back(std::make_unique<LevelWarpMenuItem>(8, 4));
    m_MenuItems.push_back(std::make_unique<PowerCheatMenuItem>());
    m_MenuItems.push_back(std::make_unique<CheatToggleMenuItem>());
}

void ESCMenuSceneHandler::OnEnter(App&) {
    // Dynamically queries states when drawing, no caching needed.
}

void ESCMenuSceneHandler::Update(App& app) {
    if (m_SubState == SubState::MENU) {
        int& sel = app.GetESCMenuSelection();
        int size = static_cast<int>(m_MenuItems.size());

        if (Util::Input::IsKeyDown(Util::Keycode::UP))
            sel = (sel - 1 + size) % size;
        if (Util::Input::IsKeyDown(Util::Keycode::DOWN))
            sel = (sel + 1) % size;

        if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
            if (sel >= 0 && sel < size) {
                m_MenuItems[sel]->Execute(app, *this);
            }
        }

        if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
            app.TransitionTo(App::State::PLAYING);
            app.PlayCurrentBGM();
            LOG_INFO("ESC pressed again - resuming game");
        }
    } else if (m_SubState == SubState::CONTROLS) {
        if (Util::Input::IsKeyDown(Util::Keycode::RETURN) ||
            Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
            m_SubState = SubState::MENU;
            LOG_INFO("Returning to Pause Menu options");
        }
    }
}

void ESCMenuSceneHandler::OnRender(App& app) {
    app.GetRenderer().Update();

    std::vector<std::string> displayTexts;
    for (const auto& item : m_MenuItems) {
        displayTexts.push_back(item->GetDisplayText(app));
    }
    int sel = app.GetESCMenuSelection();
    std::string desc = (sel >= 0 && sel < static_cast<int>(m_MenuItems.size())) 
                       ? m_MenuItems[sel]->GetDescriptionText() 
                       : "";

    auto& panel = app.GetUIManager().GetESCMenuPanel();
    panel.SetShowControls(m_SubState == SubState::CONTROLS);
    panel.SetMenuContext(sel, displayTexts, desc);
    app.GetUIManager().Update(Mario::UIManager::State::ESC_MENU);
}

}  // namespace Mario
