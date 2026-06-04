/**
 * @file Commands.cpp
 * @brief Implementation of Player Commands (Command Pattern).
 * @inheritance ICommand -> MoveRightCommand, MoveLeftCommand, StopHorizontalMovementCommand,
 *             JumpCommand, CrouchCommand, StandUpCommand, RunCommand, StopRunningCommand,
 *             ShootFireballCommand, ApplyPhysicsMovementCommand
 */
#include "Mario/Services/Commands.hpp"

#include "Mario/Player/PlayerState.hpp"
#include "Mario/Level/Level.hpp"
#include "Mario/Level/MovingPlatform.hpp"
#include "Mario/Services/AudioManager.hpp"
#include "Mario/Core/Collider.hpp"
#include "Mario/Core/GameConfig.hpp"

namespace Mario {

void MoveRightCommand::Execute(PlayerState& state, Level& level) {
    if (state.GetVelX() < -0.1f && state.IsGrounded()) {
        Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Skid);
    }
    state.SetMovingRight(true);
}

void MoveLeftCommand::Execute(PlayerState& state, Level& level) {
    if (state.GetVelX() > 0.1f && state.IsGrounded()) {
        Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Skid);
    }
    state.SetMovingLeft(true);
}

void StopMovingRightCommand::Execute(PlayerState& state, Level& level) {
    state.SetMovingRight(false);
}

void StopMovingLeftCommand::Execute(PlayerState& state, Level& level) {
    state.SetMovingLeft(false);
}

void StopHorizontalMovementCommand::Execute(PlayerState& state, Level& level) {
    state.SetMovingRight(false);
    state.SetMovingLeft(false);
}

void JumpCommand::Execute(PlayerState& state, Level& level) {
    state.SetJumping(true);
}

void CrouchCommand::Execute(PlayerState& state, Level& level) {
    if (state.GetState() > 0) {  // Only Big/Fire Mario can crouch
        state.SetCrouching(true);
        if (state.IsGrounded()) {
            state.SetMovingRight(false);
            state.SetMovingLeft(false);
        }
    } else {
        state.SetCrouching(false);
    }
}

void StandUpCommand::Execute(PlayerState& state, Level& level) {
    if (state.GetState() > 0) {  // Only Big/Fire Mario can crouch
        bool blockAbove = false;
        if (state.IsCrouching()) {
            // Standing up would increase height to 90px (2 tiles).
            // Check if there is a solid block in the upper 45px space.
            float checkY = state.GetY() - GameConfig::TILE_SIZE;
            AABB upperHalf = AABB::FromPosSize(
                state.GetX(), checkY, static_cast<float>(GameConfig::TILE_SIZE),
                static_cast<float>(GameConfig::TILE_SIZE));

            int tileX = static_cast<int>(upperHalf.left) / GameConfig::TILE_SIZE;
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
            state.SetCrouching(true);
            if (state.IsGrounded()) {
                state.SetMovingRight(false);
                state.SetMovingLeft(false);
            }
        } else {
            state.SetCrouching(false);
        }
    } else {
        state.SetCrouching(false);
    }
}

void RunCommand::Execute(PlayerState& state, Level& level) {
    state.SetRunning(true);
}

void StopRunningCommand::Execute(PlayerState& state, Level& level) {
    state.SetRunning(false);
}

void ShootFireballCommand::Execute(PlayerState& state, Level& level) {
    state.SetFireShooting(true);
}

void ApplyPhysicsMovementCommand::Execute(PlayerState& state, Level& level) {
    state.ApplyMovement(m_Speed);
}

}  // namespace Mario
