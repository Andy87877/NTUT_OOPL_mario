/**
 * @file InputHandler.hpp
 * @brief Keyboard implementation of IInputHandler (Controller in MVC).
 *        Reads PTSD Util::Input and translates to PlayerState commands.
 *        Swap this class for a gamepad or AI implementation via IInputHandler.
 * @inheritance IInputHandler -> InputHandler
 */
#ifndef MARIO_INPUT_HANDLER_HPP
#define MARIO_INPUT_HANDLER_HPP

#include "Mario/Services/IInputHandler.hpp"
#include "Mario/Player/PlayerState.hpp"

namespace Mario {

class Level;

/**
 * Concrete keyboard controller in MVC.
 * Reads keyboard input via PTSD Util::Input and applies commands to
 * PlayerState (Model).  Inherits IInputHandler for DIP.
 *
 * Key bindings (matching C# reference + WASD extension):
 *   Arrow Right / D  = Move right
 *   Arrow Left  / A  = Move left
 *   Space / Z / UP / W = Jump
 *   Arrow Down / S   = Crouch
 *   E / LShift       = Run / Fire
 */
class InputHandler : public IInputHandler {
   public:
    InputHandler() = default;

    /**
     * Read current input state and apply to the player's Model.
     * @param state The PlayerState (Model) to modify
     * @param speed Current movement speed
     * @param level Current level block grid
     */
    void HandleInput(PlayerState& state, float speed, Level& level) override;

    // -- IInputHandler overrides (input state queries) --
    bool IsMovingRight() const override { return m_Right; }
    bool IsMovingLeft() const override { return m_Left; }
    bool IsJumpPressed() const override { return m_Jump; }
    bool IsCrouchPressed() const override { return m_Crouch; }
    bool IsRunPressed() const override { return m_Run; }

   private:
    bool m_Right = false;
    bool m_Left = false;
    bool m_Jump = false;
    bool m_Crouch = false;
    bool m_Run = false;
};

}  // namespace Mario

#endif  // MARIO_INPUT_HANDLER_HPP
