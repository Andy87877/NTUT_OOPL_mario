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
        // Update input state, time delta, and swap buffers at the start of frame
        // to ensure zero input-to-physics update latency.
        context->Update();

        switch (app.GetCurrentState()) {
            case App::State::START:
                app.Start();
                break;

            case App::State::TITLE:
            case App::State::LOADING:
            case App::State::PLAYING:
            case App::State::FLAGPOLE:
            case App::State::PIPE_WARP:
            case App::State::AXE_SEQUENCE:
            case App::State::DEATH:
            case App::State::GAME_OVER:
            case App::State::GAME_WON:
            case App::State::ESC_MENU:
                app.Update();
                break;

            case App::State::END:
                app.End();
                context->SetExit(true);
                break;
        }
    }
    return 0;
}
