/**
 * @file IInputProfile.hpp
 * @brief Strategy interface for mapping physical keys to abstract game buttons.
 *        Enables key re-binding, custom controls, and gamepad support.
 * @inheritance None (pure interface)
 */
#ifndef MARIO_I_INPUT_PROFILE_HPP
#define MARIO_I_INPUT_PROFILE_HPP

namespace Mario {

/**
 * Abstract game action buttons mapped from hardware keys.
 */
enum class GameButton {
    RIGHT,
    LEFT,
    JUMP,
    CROUCH,
    RUN,
    FIRE
};

/**
 * Interface representing an input mapping profile (Strategy Pattern).
 * Separates physical input device keycodes from logical game controllers.
 */
class IInputProfile {
   public:
    virtual ~IInputProfile() = default;

    /**
     * Check if a button is currently pressed/held.
     */
    virtual bool IsButtonPressed(GameButton btn) const = 0;

    /**
     * Check if a button was pressed down in this frame.
     */
    virtual bool IsButtonDown(GameButton btn) const = 0;
};

}  // namespace Mario

#endif  // MARIO_I_INPUT_PROFILE_HPP
