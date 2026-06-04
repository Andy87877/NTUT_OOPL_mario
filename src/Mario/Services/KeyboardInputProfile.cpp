/**
 * @file KeyboardInputProfile.cpp
 * @brief Default keyboard mapping implementation.
 * @inheritance IInputProfile -> KeyboardInputProfile
 */
#include "Mario/Services/KeyboardInputProfile.hpp"
#include "Util/Input.hpp"

namespace Mario {

bool KeyboardInputProfile::IsButtonPressed(GameButton btn) const {
    switch (btn) {
        case GameButton::RIGHT:
            return Util::Input::IsKeyPressed(Util::Keycode::RIGHT) ||
                   Util::Input::IsKeyPressed(Util::Keycode::D);
        case GameButton::LEFT:
            return Util::Input::IsKeyPressed(Util::Keycode::LEFT) ||
                   Util::Input::IsKeyPressed(Util::Keycode::A);
        case GameButton::JUMP:
            return Util::Input::IsKeyPressed(Util::Keycode::SPACE) ||
                   Util::Input::IsKeyPressed(Util::Keycode::Z) ||
                   Util::Input::IsKeyPressed(Util::Keycode::UP) ||
                   Util::Input::IsKeyPressed(Util::Keycode::W);
        case GameButton::CROUCH:
            return Util::Input::IsKeyPressed(Util::Keycode::DOWN) ||
                   Util::Input::IsKeyPressed(Util::Keycode::S);
        case GameButton::RUN:
            return Util::Input::IsKeyPressed(Util::Keycode::LSHIFT) ||
                   Util::Input::IsKeyPressed(Util::Keycode::E);
        case GameButton::FIRE:
            return Util::Input::IsKeyPressed(Util::Keycode::E) ||
                   Util::Input::IsKeyPressed(Util::Keycode::LSHIFT);
        default:
            return false;
    }
}

bool KeyboardInputProfile::IsButtonDown(GameButton btn) const {
    switch (btn) {
        case GameButton::RIGHT:
            return Util::Input::IsKeyDown(Util::Keycode::RIGHT) ||
                   Util::Input::IsKeyDown(Util::Keycode::D);
        case GameButton::LEFT:
            return Util::Input::IsKeyDown(Util::Keycode::LEFT) ||
                   Util::Input::IsKeyDown(Util::Keycode::A);
        case GameButton::JUMP:
            return Util::Input::IsKeyDown(Util::Keycode::SPACE) ||
                   Util::Input::IsKeyDown(Util::Keycode::Z) ||
                   Util::Input::IsKeyDown(Util::Keycode::UP) ||
                   Util::Input::IsKeyDown(Util::Keycode::W);
        case GameButton::CROUCH:
            return Util::Input::IsKeyDown(Util::Keycode::DOWN) ||
                   Util::Input::IsKeyDown(Util::Keycode::S);
        case GameButton::RUN:
            return Util::Input::IsKeyDown(Util::Keycode::LSHIFT) ||
                   Util::Input::IsKeyDown(Util::Keycode::E);
        case GameButton::FIRE:
            return Util::Input::IsKeyDown(Util::Keycode::E) ||
                   Util::Input::IsKeyDown(Util::Keycode::LSHIFT);
        default:
            return false;
    }
}

}  // namespace Mario
