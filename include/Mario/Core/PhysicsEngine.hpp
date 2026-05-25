/**
 * @file PhysicsEngine.hpp
 * @brief Physics engine handling gravity, jumping, and velocity calculations.
 * @inheritance None (standalone service)
 */
#ifndef MARIO_PHYSICS_ENGINE_HPP
#define MARIO_PHYSICS_ENGINE_HPP

#include "Mario/Core/GameConfig.hpp"

namespace Mario {

/**
 * Handles gravity and vertical physics for players and entities.
 * Uses the same physics model as the C# reference.
 */
class PhysicsEngine {
public:
    PhysicsEngine() = default;

    /**
     * Apply gravity to a vertical velocity, returning the new Y position delta.
     * @param velY Current vertical velocity (positive = downward)
     * @param fallHeight Current fall/jump height (decrements to zero)
     * @param isGrounded Whether the object is on the ground
     * @return The Y position delta to apply
     */
    static float ApplyGravity(double& fallHeight, double& velY, bool isGrounded);

    /**
     * Calculate the Y velocity for a jump.
     * @param jumpMultiplier -1 = low, 0 = normal, 1 = high
     * @return The initial fall height for the jump
     */
    static double GetJumpHeight(int jumpMultiplier);
};

} // namespace Mario

#endif // MARIO_PHYSICS_ENGINE_HPP
