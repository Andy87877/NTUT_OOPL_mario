/**
 * @file CollisionManager.hpp
 * @brief Manages collision detection between Player and Blocks.
 *        Handles ground detection, wall collision, and head bumping.
 * @inheritance None (manager class)
 */
#ifndef MARIO_COLLISION_MANAGER_HPP
#define MARIO_COLLISION_MANAGER_HPP

#include "Mario/Player.hpp"
#include "Mario/Level.hpp"

#include <memory>
#include <vector>

namespace Mario {

/**
 * Handles all collision detection and resolution between the player
 * and the level's block grid.
 *
 * Collision logic ported from C# Form1.cs onTick() collision section.
 * Uses AABB overlap checks and resolves in priority order:
 *   1. Ground (below player)
 *   2. Ceiling (above player - head bump)
 *   3. Wall (left/right - block movement)
 */
class CollisionManager {
public:
    CollisionManager() = default;

    /**
     * Check and resolve all Player-Block collisions for one frame.
     * @param player The player to check
     * @param level The current level with block data
     * @param cameraOffset Current camera offset
     */
    void CheckPlayerBlockCollision(Player& player, Level& level,
                                   float cameraOffset, std::vector<Level::SpawnPoint>* outSpawns = nullptr);

    /**
     * Check if the player has fallen into a pit.
     * @param player The player
     * @return true if player is below the level
     */
    bool CheckPitFall(const Player& player) const;

private:
    /**
     * Check ground collision: is the player standing on a solid block?
     */
    void CheckGroundCollision(PlayerState& state, Level& level);

    /**
     * Check ceiling collision: did the player hit a block from below?
     */
    void CheckCeilingCollision(PlayerState& state, Level& level, std::vector<Level::SpawnPoint>* outSpawns = nullptr);

    /**
     * Check horizontal wall collision: left and right sides.
     */
    void CheckWallCollision(PlayerState& state, Level& level,
                            float cameraOffset);
};

} // namespace Mario

#endif // MARIO_COLLISION_MANAGER_HPP
