/**
 * @file IInputHandler.hpp
 * @brief Abstract interface for the input controller layer in MVC architecture.
 *        Follows the Dependency Inversion Principle (DIP): App and scene
 *        handlers depend on IInputHandler (abstraction), not on the concrete
 *        KeyboardInputHandler implementation.
 *        Enables future extensions: gamepad, AI-driven input, replay system.
 * @inheritance None (pure interface)
 */
#ifndef MARIO_I_INPUT_HANDLER_HPP
#define MARIO_I_INPUT_HANDLER_HPP

namespace Mario {

class PlayerState;
class Level;

/**
 * Pure virtual interface for the input controller in the MVC pattern.
 *
 * The InputHandler (Controller) reads raw device input and translates it
 * into commands that modify the PlayerState (Model).  By depending on this
 * interface, the rest of the system is decoupled from any specific input
 * device (keyboard, gamepad, replay, AI demo mode, etc.).
 *
 * Key bindings are an implementation detail of the concrete class.
 */
class IInputHandler {
   public:
    virtual ~IInputHandler() = default;

    /**
     * Read current input and apply it to the player's Model.
     * Called once per frame from PlayingSceneHandler.
     * @param state  The PlayerState (Model) to modify
     * @param speed  Current movement speed (pixels/frame)
     * @param level  Current level for crouch-stand-up guard
     */
    virtual void HandleInput(PlayerState& state, float speed, Level& level) = 0;

    // -- Per-frame input state queries (read after HandleInput) --
    virtual bool IsMovingRight() const = 0;
    virtual bool IsMovingLeft() const = 0;
    virtual bool IsJumpPressed() const = 0;
    virtual bool IsCrouchPressed() const = 0;
    virtual bool IsRunPressed() const = 0;
};

}  // namespace Mario

#endif  // MARIO_I_INPUT_HANDLER_HPP
