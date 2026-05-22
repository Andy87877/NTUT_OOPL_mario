/**
 * @file PhysicsEngine.cpp
 * @brief Implementation of gravity and jump physics.
 * @inheritance None
 */
#include "Mario/PhysicsEngine.hpp"

namespace Mario {

float PhysicsEngine::ApplyGravity(double& fallHeight, double& velY,
                                  bool isGrounded) {
    if (isGrounded) {
        velY = 0.0;
        fallHeight = 0.0;
        return 0.0f;
    }

    // Parabolic jump: rise while fallHeight > 0, then fall.
    // Matches C# Form1.cs gravity formula exactly:
    //   Rising : previousJumpHeight -= 9.81 * 0.02 * 2  (2x multiplier)
    //   Falling: previousJumpHeight -= 9.81 * 0.02 * 4  (4x multiplier)
    // The 4x fall multiplier makes Mario drop faster than he rises,
    // giving the snappy feel of the original NES game.
    if (fallHeight > 0.0) {
        velY = -fallHeight;
        fallHeight -= GameConfig::GRAVITY * GameConfig::TICK_INTERVAL * 2.0;
        if (fallHeight < 0.0) {
            fallHeight = 0.0;
        }
    } else {
        // Falling: accelerate downward (4x compared to C# reference)
        velY += GameConfig::GRAVITY * GameConfig::TICK_INTERVAL * 4.0;
    }

    return static_cast<float>(velY);
}

double PhysicsEngine::GetJumpHeight(int jumpMultiplier) {
    if (jumpMultiplier == 1) {
        return GameConfig::JUMP_HIGH_VELOCITY;
    } else if (jumpMultiplier == -1) {
        return GameConfig::JUMP_LOW_VELOCITY;
    }
    return GameConfig::JUMP_VELOCITY;
}

}  // namespace Mario
