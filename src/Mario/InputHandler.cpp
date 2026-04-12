/**
 * @file InputHandler.cpp
 * @brief Implementation of InputHandler (Controller layer).
 *        Reads PTSD Util::Input and applies to PlayerState.
 *        Key bindings match the C# reference (Right/D, Left/A, Space/Z, Down/S,
 * E/LShift).
 * @inheritance None (Controller)
 */
#include "Mario/InputHandler.hpp"

#include "Mario/AudioManager.hpp"
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
    m_Left = Util::Input::IsKeyPressed(Util::Keycode::LEFT) ||
             Util::Input::IsKeyPressed(Util::Keycode::A);

    if (m_Right && state.GetVelX() < -0.1f && state.IsGrounded()) {
        Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Skid);
    } else if (m_Left && state.GetVelX() > 0.1f && state.IsGrounded()) {
        Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Skid);
    }

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
    if (state.GetState() > 0) {  // Only big/fire mario can crouch
        bool wasCrouching = state.IsCrouching();
        state.SetCrouching(m_Crouch);

        // Adjust Y position when crouch state changes (to keep feet in same
        // place) When crouching: height decrease by TILE_SIZE, so move down by
        // TILE_SIZE/2 When standing: height increase by TILE_SIZE, so move up
        // by TILE_SIZE/2
        if (wasCrouching != m_Crouch && state.GetState() > 0) {
            if (m_Crouch) {
                // Entering crouch: height decreases, move down
                state.SetY(state.GetY() + GameConfig::TILE_SIZE / 2.0f);
            } else {
                // Exiting crouch: height increases, move up
                state.SetY(state.GetY() - GameConfig::TILE_SIZE / 2.0f);
            }
        }
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

}  // namespace Mario
