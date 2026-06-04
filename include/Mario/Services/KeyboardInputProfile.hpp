/**
 * @file KeyboardInputProfile.hpp
 * @brief Default keyboard implementation of IInputProfile mapping to WASD and Arrows.
 * @inheritance IInputProfile -> KeyboardInputProfile
 */
#ifndef MARIO_KEYBOARD_INPUT_PROFILE_HPP
#define MARIO_KEYBOARD_INPUT_PROFILE_HPP

#include "Mario/Services/IInputProfile.hpp"

namespace Mario {

/**
 * Concrete keyboard layout profile.
 * Maps GameButton enum values to physical Util::Keycode values.
 */
class KeyboardInputProfile : public IInputProfile {
   public:
    KeyboardInputProfile() = default;
    ~KeyboardInputProfile() override = default;

    bool IsButtonPressed(GameButton btn) const override;
    bool IsButtonDown(GameButton btn) const override;
};

}  // namespace Mario

#endif  // MARIO_KEYBOARD_INPUT_PROFILE_HPP
