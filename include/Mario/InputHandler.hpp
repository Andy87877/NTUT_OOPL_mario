/**
 * @file InputHandler.hpp
 * @brief Controller layer for player input in MVC architecture.
 *        Reads PTSD Util::Input and translates to PlayerState commands.
 * @inheritance None (Controller)
 */
#ifndef MARIO_INPUT_HANDLER_HPP
#define MARIO_INPUT_HANDLER_HPP

#include "Mario/PlayerState.hpp"

namespace Mario {

/**
 * The Controller in MVC. Reads keyboard input via PTSD Util::Input
 * and applies corresponding commands to the PlayerState (Model).
 *
 * Key bindings (matching C# reference):
 *   Arrow Right / D  = Move right
 *   Arrow Left  / A  = Move left
 *   Space / Z        = Jump
 *   Arrow Down / S   = Crouch
 *   E / LShift       = Run / Fire
 */
class InputHandler {
public:
    InputHandler() = default;

    /**
     * Read current input state and apply to the player's Model.
     * @param state The PlayerState (Model) to modify
     * @param speed Current movement speed
     */
    void HandleInput(PlayerState& state, float speed);

    // -- Input state queries --
    bool IsMovingRight() const { return m_Right; }
    bool IsMovingLeft() const  { return m_Left; }
    bool IsJumpPressed() const { return m_Jump; }
    bool IsCrouchPressed() const { return m_Crouch; }
    bool IsRunPressed() const  { return m_Run; }

private:
    bool m_Right  = false;
    bool m_Left   = false;
    bool m_Jump   = false;
    bool m_Crouch = false;
    bool m_Run    = false;
};

} // namespace Mario

#endif // MARIO_INPUT_HANDLER_HPP
