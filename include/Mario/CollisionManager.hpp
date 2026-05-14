/**
 * @file CollisionManager.hpp
 * @brief Manages collision detection between Player and Blocks.
 *        Handles ground detection, wall collision, and head bumping.
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
    /**
     * Check ground collision: is the player standing on a solid block?
     */
    void CheckGroundCollision(PlayerState& state, Level& level);

    /**
     * Check ceiling collision: did the player hit a block from below?
     * Directly grants coins for CoinGet blocks instead of spawning entities.
     * Applies camera offset to display floating text at correct screen
     * position.
     */
    void CheckCeilingCollision(
        PlayerState& state, Level& level, Camera& camera,
        GameStateManager& gameState, UIManager& uiManager,
        std::vector<Level::SpawnPoint>* outSpawns = nullptr);

    /**
     * Check horizontal wall collision: left and right sides.
     */
    void CheckWallCollision(PlayerState& state, Level& level,
                            float cameraOffset);
};

}  // namespace Mario

#endif  // MARIO_COLLISION_MANAGER_HPP
