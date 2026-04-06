/**
 * @file PhysicsEngine.cpp
 * @brief Implementation of gravity and jump physics.
 * @inheritance None
 */
#include "Mario/PhysicsEngine.hpp"

namespace Mario {

float PhysicsEngine::ApplyGravity(double& fallHeight, double& velY, bool isGrounded) {
    if (isGrounded) {
        velY = 0.0;
        fallHeight = 0.0;
        return 0.0f;
    }

    // Parabolic jump: rise while fallHeight > 0, then fall
    if (fallHeight > 0.0) {
        velY = -fallHeight;
        fallHeight -= GameConfig::GRAVITY * GameConfig::TICK_INTERVAL;
        if (fallHeight < 0.0) {
            fallHeight = 0.0;
        }
    } else {
        // Falling: accelerate downward
        velY += GameConfig::GRAVITY * GameConfig::TICK_INTERVAL;
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

} // namespace Mario
