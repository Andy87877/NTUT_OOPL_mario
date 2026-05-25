/**
 * @file CollisionManager.hpp
 * @brief Facade/coordinator for all collision subsystems.
 *        Owns one handler instance per collision type and delegates each public
 *        call to the appropriate handler, keeping the public API unchanged for
 *        all callers (PlayingSceneHandler etc.).
 *
 *        OOP architecture:
 *          CollisionManager  (facade — no logic of its own)
 *            has-a  PlayerBlockHandler   : ICollisionHandler
 *            has-a  PlayerEntityHandler  : ICollisionHandler
 *            has-a  EntityBlockHandler   : ICollisionHandler
 *            has-a  EntityEntityHandler  : ICollisionHandler
 *
 * @inheritance None (facade class)
 */
#ifndef MARIO_COLLISION_MANAGER_HPP
#define MARIO_COLLISION_MANAGER_HPP

#include <memory>
#include <vector>

#include "Mario/Collision/EntityBlockHandler.hpp"
#include "Mario/Collision/EntityEntityHandler.hpp"
#include "Mario/Collision/PlayerBlockHandler.hpp"
#include "Mario/Collision/PlayerEntityHandler.hpp"
#include "Mario/Level/Level.hpp"
#include "Mario/Player/Player.hpp"

namespace Mario {

// Forward declarations
class GameStateManager;
class UIManager;
class Camera;
class Entity;

/**
 * Coordinates the four collision subsystems.
 * Public API is identical to the old monolithic implementation so all call
 * sites in PlayingSceneHandler remain unchanged.
 */
class CollisionManager {
   public:
    CollisionManager() = default;

    /**
     * Run the Player-Block collision pipeline (FallDetect → CeilingTrigger
     * → BodyResolution). Delegates to PlayerBlockHandler.
     */
    void CheckPlayerBlockCollision(
        Player& player, Level& level, Camera& camera,
        GameStateManager& gameState, UIManager& uiManager,
        std::vector<Level::SpawnPoint>* outSpawns = nullptr);

    /**
     * Check whether the player has fallen into a pit.
     * @return true if player Y > LEVEL_HEIGHT_PX + TILE_SIZE.
     */
    bool CheckPitFall(const Player& player) const;

    /**
     * Run Player-Entity collision (stomp, damage, power-ups, coins).
     * Delegates to PlayerEntityHandler.
     */
    void CheckPlayerEntityCollision(
        Player& player, std::vector<std::shared_ptr<Entity>>& entities,
        Camera& camera, GameStateManager& gameState, UIManager& uiManager);

    /**
     * Run Entity-Entity collision (Fireball vs Enemy, Shell vs Enemy).
     * Delegates to EntityEntityHandler.
     */
    void CheckEntityEntityCollision(
        std::vector<std::shared_ptr<Entity>>& entities,
        GameStateManager& gameState, float cameraOffset = -9999.0f);

    /**
     * Run Entity-Block collision for one entity (ground, wall flip, pit).
     * Delegates to EntityBlockHandler.
     */
    void CheckEntityBlockCollision(
        Entity& entity, Level& level,
        std::vector<std::shared_ptr<Entity>>* outNewEntities = nullptr);

   private:
    // One handler per collision subsystem.  Each inherits ICollisionHandler.
    PlayerBlockHandler m_PlayerBlockHandler;
    PlayerEntityHandler m_PlayerEntityHandler;
    EntityBlockHandler m_EntityBlockHandler;
    EntityEntityHandler m_EntityEntityHandler;

    // m_StompCombo moved to PlayerEntityHandler.
};

}  // namespace Mario

#endif  // MARIO_COLLISION_MANAGER_HPP
