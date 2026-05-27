/**
 * @file InputHandler.cpp
 * @brief Implementation of InputHandler (Controller layer).
 *        Reads PTSD Util::Input and applies to PlayerState.
 *        Key bindings match the C# reference (Right/D, Left/A, Space/Z, Down/S,
 * E/LShift).
 * @inheritance IInputHandler -> InputHandler
 */
#include "Mario/Services/InputHandler.hpp"

#include "Mario/Services/AudioManager.hpp"
#include "Mario/Core/Collider.hpp"
#include "Mario/Level/Level.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

namespace Mario {

void InputHandler::HandleInput(PlayerState& state, float speed, Level& level) {
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
             Util::Input::IsKeyDown(Util::Keycode::UP) ||
             Util::Input::IsKeyDown(Util::Keycode::W);
    if (m_Jump) {
        state.SetJumping(true);
    }

    // -- Crouch --
    // Per C# reference (Player.cs): only state > 0 (Big/Fire) can crouch.
    // Additional rule: crouching locks horizontal movement — Mario cannot
    // walk left/right while ducking (reference: Form1.cs movement logic).
    m_Crouch = Util::Input::IsKeyPressed(Util::Keycode::DOWN) ||
               Util::Input::IsKeyPressed(Util::Keycode::S);
    if (state.GetState() > 0) {  // Only big/fire mario can crouch
        bool blockAbove = false;
        if (!m_Crouch && state.IsCrouching()) {
            // Standing up would increase height to 90px (2 tiles).
            // Check if there is a solid block in the upper 45px space.
            // Currently, top is at state.GetY(). The upper 45px space is
            // [state.GetY() - TILE_SIZE, state.GetY()].
            float checkY = state.GetY() - GameConfig::TILE_SIZE;
            AABB upperHalf = AABB::FromPosSize(
                state.GetX(), checkY, static_cast<float>(GameConfig::TILE_SIZE),
                static_cast<float>(GameConfig::TILE_SIZE));

            int tileX =
                static_cast<int>(upperHalf.left) / GameConfig::TILE_SIZE;
            int tileY = static_cast<int>(upperHalf.top) / GameConfig::TILE_SIZE;

            for (int gy = tileY; gy <= tileY + 1 && !blockAbove; gy++) {
                for (int gx = tileX - 1; gx <= tileX + 2 && !blockAbove; gx++) {
                    Block* blk = level.GetBlockAt(gx, gy);
                    if (blk && blk->IsSolid() &&
                        blk->GetAABB().Intersects(upperHalf)) {
                        blockAbove = true;
                    }
                }
            }

            // Check moving platforms
            for (auto* plat : level.GetMovingPlatforms()) {
                if (plat && plat->IsSolid() &&
                    plat->GetAABB().Intersects(upperHalf)) {
                    blockAbove = true;
                    break;
                }
            }
        }

        if (blockAbove) {
            m_Crouch = true;
        }

        state.SetCrouching(m_Crouch);

        // Block horizontal movement while crouching (grounded only)
        if (m_Crouch && state.IsGrounded()) {
            state.SetMovingRight(false);
            state.SetMovingLeft(false);
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
