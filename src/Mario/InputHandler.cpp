/**
 * @file InputHandler.cpp
 * @brief Implementation of InputHandler (Controller layer).
 *        Reads PTSD Util::Input and applies to PlayerState.
 *        Key bindings match the C# reference (Right/D, Left/A, Space/Z, Down/S, E/LShift).
 * @inheritance None (Controller)
 */
#include "Mario/InputHandler.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

namespace Mario {

void InputHandler::HandleInput(PlayerState& state, float speed) {
    if (!state.IsControllable() || state.IsDead()) {
        state.SetMovingRight(false);
        state.SetMovingLeft(false);
        return;
    }

    // -- Horizontal Movement --
    m_Right = Util::Input::IsKeyPressed(Util::Keycode::RIGHT) ||
              Util::Input::IsKeyPressed(Util::Keycode::D);
    m_Left  = Util::Input::IsKeyPressed(Util::Keycode::LEFT) ||
              Util::Input::IsKeyPressed(Util::Keycode::A);

    state.SetMovingRight(m_Right);
    state.SetMovingLeft(m_Left);

    // -- Jump --
    m_Jump = Util::Input::IsKeyDown(Util::Keycode::SPACE) ||
             Util::Input::IsKeyDown(Util::Keycode::Z) ||
             Util::Input::IsKeyDown(Util::Keycode::UP);
    if (m_Jump) {
        state.SetJumping(true);
    }

    // -- Crouch --
    m_Crouch = Util::Input::IsKeyPressed(Util::Keycode::DOWN) ||
               Util::Input::IsKeyPressed(Util::Keycode::S);
    if (state.GetState() > 0) { // Only big/fire mario can crouch
        state.SetCrouching(m_Crouch);
    } else {
        state.SetCrouching(false);
    }

    // -- Run / Fire --
    m_Run = Util::Input::IsKeyPressed(Util::Keycode::E) ||
            Util::Input::IsKeyPressed(Util::Keycode::LSHIFT);
    state.SetRunning(m_Run);

    // Fire shooting: only on key down (not held)
    if (Util::Input::IsKeyDown(Util::Keycode::E) ||
        Util::Input::IsKeyDown(Util::Keycode::LSHIFT)) {
        state.SetFireShooting(true);
    }

    // Apply horizontal movement velocity
    state.ApplyMovement(speed);
}

} // namespace Mario
