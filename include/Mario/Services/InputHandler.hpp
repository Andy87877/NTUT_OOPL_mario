/**
 * @file InputHandler.hpp
 * @brief Keyboard implementation of IInputHandler (Controller in MVC).
 *        Reads PTSD Util::Input and translates to PlayerState commands.
 *        Swap this class for a gamepad or AI implementation via IInputHandler.
 * @inheritance IInputHandler -> InputHandler
 */
#ifndef MARIO_INPUT_HANDLER_HPP
#define MARIO_INPUT_HANDLER_HPP

#include <memory>
#include "Mario/Services/IInputHandler.hpp"
#include "Mario/Services/IInputProfile.hpp"
#include "Mario/Player/PlayerState.hpp"

namespace Mario {

class Level;

/**
 * Concrete keyboard controller in MVC.
 * Reads keyboard input via the injected IInputProfile strategy and applies commands to
 * PlayerState (Model). Inherits IInputHandler for DIP.
 */
class InputHandler : public IInputHandler {
   public:
    /**
     * Constructor injecting an input profile strategy.
     * @param profile Injected strategy profile. If nullptr, defaults to KeyboardInputProfile.
     */
    explicit InputHandler(std::unique_ptr<IInputProfile> profile = nullptr);
    ~InputHandler() override = default;

    /**
     * Read current input state and translate to a sequence of Commands.
     * @param state The PlayerState (Model) to reference
     * @param speed Current movement speed
     * @param level Current level block grid
     * @return List of command objects to execute
     */
    std::vector<std::shared_ptr<ICommand>> HandleInput(
        PlayerState& state, float speed, Level& level) override;

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

    int m_LastDirectionPressed = 0;  // 1 = Right, -1 = Left, 0 = None

    std::unique_ptr<IInputProfile> m_Profile;
};

}  // namespace Mario

#endif  // MARIO_INPUT_HANDLER_HPP
