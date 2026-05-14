/**
 * @file ESCMenuSceneHandler.cpp
 * @brief Pause/ESC menu scene handler implementation.
 * @inheritance ISceneHandler <- ESCMenuSceneHandler
 */
#include "Mario/ESCMenuSceneHandler.hpp"

#include "App.hpp"
#include "Mario/AudioManager.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

namespace Mario {

void ESCMenuSceneHandler::Update(App& app) {
    int& sel = app.GetESCMenuSelection();

    if (Util::Input::IsKeyDown(Util::Keycode::UP)) sel = (sel - 1 + 4) % 4;
    if (Util::Input::IsKeyDown(Util::Keycode::DOWN)) sel = (sel + 1) % 4;

    if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        switch (sel) {
            case 0:
                app.TransitionTo(App::State::PLAYING);
                app.PlayCurrentBGM();
                LOG_INFO("Resuming game");
                break;
            case 1:
                app.GetGameState().SetLevel(1, 1);
                app.TransitionTo(App::State::LOADING);
                LOG_INFO("Jumping to World 1-1");
                break;
            case 2:
                app.GetGameState().SetLevel(1, 2);
                app.TransitionTo(App::State::LOADING);
                LOG_INFO("Jumping to World 1-2");
                break;
            case 3:
                app.GetGameState().SetLevel(8, 4);
                app.TransitionTo(App::State::LOADING);
                LOG_INFO("Jumping to World 8-4");
                break;
        }
    }

    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        app.TransitionTo(App::State::PLAYING);
        app.PlayCurrentBGM();
        LOG_INFO("ESC pressed again - resuming game");
    }
}

void ESCMenuSceneHandler::OnRender(App& app) {
    app.GetRenderer().Update();
    app.GetUIManager().Update(Mario::UIManager::State::ESC_MENU,
                              app.GetESCMenuSelection());
}

}  // namespace Mario
