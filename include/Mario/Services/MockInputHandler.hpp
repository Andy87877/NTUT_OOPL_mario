/**
 * @file MockInputHandler.hpp
 * @brief Mock/simulated implementation of IInputHandler (Controller in MVC).
 *        Allows queuing of button states or replaying commands for tests/demos.
 * @inheritance IInputHandler -> MockInputHandler
 */
#ifndef MARIO_MOCK_INPUT_HANDLER_HPP
#define MARIO_MOCK_INPUT_HANDLER_HPP

#include "Mario/Services/IInputHandler.hpp"

namespace Mario {

/**
 * Concrete mock input controller in MVC pattern.
 * Allows programmatic input simulation for testing or demo replay loops.
 */
class MockInputHandler : public IInputHandler {
   public:
    MockInputHandler() = default;
    ~MockInputHandler() override = default;

    /**
     * Read current mock input state and translate to a sequence of Commands.
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

    /**
     * Inject key states programmatically for simulations.
     */
    void SetInputs(bool right, bool left, bool jump, bool crouch, bool run, bool fire) {
        m_Right = right;
        m_Left = left;
        m_Jump = jump;
        m_Crouch = crouch;
        m_Run = run;
        m_Fire = fire;
    }

   private:
    bool m_Right = false;
    bool m_Left = false;
    bool m_Jump = false;
    bool m_Crouch = false;
    bool m_Run = false;
    bool m_Fire = false;
};

}  // namespace Mario

#endif  // MARIO_MOCK_INPUT_HANDLER_HPP
