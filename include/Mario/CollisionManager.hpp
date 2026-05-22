/**
 * @file CollisionManager.hpp
 * @brief Manages collision detection between Player and Blocks.
 *        Logic ported from C# Form1.cs: FallDetect → ceiling trigger →
 *        per-block resolution in C# order (DOWN, RIGHT, LEFT, DOWN, UP, LEFT).
 * @inheritance None (manager class)
 */
#ifndef MARIO_COLLISION_MANAGER_HPP
#define MARIO_COLLISION_MANAGER_HPP

#include <memory>
#include <vector>

#include "Mario/Level.hpp"
#include "Mario/Player.hpp"

namespace Mario {

// Forward declarations
class GameStateManager;
class UIManager;
class Camera;
class Entity;

/**
 * Handles all collision detection and resolution between the player
 * and the level's block grid.
 *
 * Collision logic ported from C# Form1.cs onTick() collision section.
 * Per-frame pipeline (matching C# exactly):
 *   1. FallDetect  — thin strip below feet; if no solid block →
 * SetGrounded(false)
 *   2. Ceiling trigger — head-bump against narrow hitbox, spawns block contents
 *   3. Per-block resolution (for each intersecting block):
 *        Airborne  : DOWN → RIGHT → LEFT → DOWN → UP → LEFT
 *        Grounded  : RIGHT or LEFT only
 */
class CollisionManager {
   public:
    CollisionManager() = default;

    /**
     * Check and resolve all Player-Block collisions for one frame.
     * @param player The player to check
     * @param level The current level with block data
     * @param camera Camera for coordinate conversion (world to screen)
     * @param gameState Game state manager for handling coin blocks (CoinGet)
     * @param uiManager UI manager for displaying floating text effects
     */
    void CheckPlayerBlockCollision(
        Player& player, Level& level, Camera& camera,
        GameStateManager& gameState, UIManager& uiManager,
        std::vector<Level::SpawnPoint>* outSpawns = nullptr);

    /**
     * Check if the player has fallen into a pit.
     * @param player The player
     * @return true if player is below the level
     */
    bool CheckPitFall(const Player& player) const;

    /**
     * ✨ Check Player-Entity collision (e.g., Goomba, KoopaTroopa).
     * Handles enemy stomping, damage, power-up collection, and scoring.
     * @param player The player
     * @param entities All entities in level
     * @param camera Camera for screen coordinate conversion
     * @param gameState For score/life management
     * @param uiManager For floating text effects
     */
    void CheckPlayerEntityCollision(
        Player& player, std::vector<std::shared_ptr<Entity>>& entities,
        Camera& camera, GameStateManager& gameState, UIManager& uiManager);

    /**
     * ✨ Check Entity-Entity collision (e.g., Fire vs Enemy, Shell vs Enemy).
     * Handles special collisions between projectiles and enemies.
     * @param entities All entities in level
     * @param gameState For score management
     */
    void CheckEntityEntityCollision(
        std::vector<std::shared_ptr<Entity>>& entities,
        GameStateManager& gameState);

    /**
     * Check and resolve Entity-Block collision for one entity per frame.
     * Handles ground, wall, and pit-fall for non-player entities.
     * Ported from the old App::CheckEntityBlockCollision().
     * @param entity Entity to check
     * @param level Current level data
     */
    void CheckEntityBlockCollision(Entity& entity, Level& level);

   private:
    // ---- Per-direction resolution helpers (matching C# CheckCollisionsXxx)
    // ----

    /**
     * Snap player feet to block top when falling onto it.
     * C#: if (Bottom > b.Top && Bottom < b.Top + size*0.75 && movingDown)
     */
    void ResolveDown(PlayerState& state, const AABB& bb, bool& movingDown);

    /**
     * Snap player head to block bottom when jumping into it.
     * C#: if (Top < b.Bottom && Top > b.Bottom - size*0.75 && movingUp)
     */
    void ResolveUp(PlayerState& state, const AABB& bb, bool& movingUp);

    /**
     * Snap player right edge to block left edge when moving right.
     * C#: if (Right > b.Left && Right < b.Left + size*0.75 && movingRight)
     */
    void ResolveRight(PlayerState& state, const AABB& bb, bool& movingRight);

    /**
     * Snap player left edge to block right edge when moving left.
     * C#: if (Left < b.Right && Left > b.Right - size*0.75 && movingLeft)
     */
    void ResolveLeft(PlayerState& state, const AABB& bb, bool& movingLeft);

    /**
     * Trigger a block hit: play sound, spawn contents, break bricks.
     * Called when the player's head bumps the bottom of a block.
     */
    void TriggerBlockHit(Block& block, PlayerState& state, Camera& camera,
                         GameStateManager& gameState, UIManager& uiManager,
                         std::vector<Level::SpawnPoint>* outSpawns);

    // Consecutive stomp combo counter.
    // Incremented each time Mario stomps an enemy while airborne.
    // Reset to 0 whenever Mario is grounded (between stomp chains).
    // Used to compute the NES-authentic score multiplier:
    //   1st stomp = 100, 2nd = 200, 3rd = 400, 4th = 800, 5th+ = 1000.
    int m_StompCombo = 0;
};

}  // namespace Mario

#endif  // MARIO_COLLISION_MANAGER_HPP
