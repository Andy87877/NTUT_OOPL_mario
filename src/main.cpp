/**
 * @file main.cpp
 * @brief Application entry point for Super Mario Bros.
 *        Drives the main game loop via App state machine.
 * @inheritance None
 */
#include "App.hpp"
#include "Core/Context.hpp"

int main(int, char**) {
    auto context = Core::Context::GetInstance();
    App app;

    while (!context->GetExit()) {
        switch (app.GetCurrentState()) {
            case App::State::START:
                app.Start();
                break;

            case App::State::TITLE:
            case App::State::LOADING:
            case App::State::PLAYING:
            case App::State::FLAGPOLE:
            case App::State::PIPE_WARP:
            case App::State::DEATH:
            case App::State::GAME_OVER:
            case App::State::ESC_MENU:
                app.Update();
                break;

            case App::State::END:
                app.End();
                context->SetExit(true);
                break;
        }
        context->Update();
    }
    return 0;
}
