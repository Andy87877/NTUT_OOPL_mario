/**
 * @file EntityFactory.hpp
 * @brief Factory for creating Entity instances from spawn point data.
 *        Reads EntityDef from Level lookup tables and spawns entities.
 * @inheritance None (factory pattern)
 */
#ifndef MARIO_ENTITY_FACTORY_HPP
#define MARIO_ENTITY_FACTORY_HPP

#include <memory>
#include <vector>

#include "Mario/Level/Entity.hpp"
#include "Mario/Level/Level.hpp"

namespace Mario {

/**
 * Creates Entity GameObjects from level spawn point data.
 * Called by App when entering PLAYING state.
 */
class EntityFactory {
   public:
    /**
     * Spawn all entities defined by the level's spawn points.
     * Ignores non-entity IDs (Flag, UnderCoin, etc.).
     * @param level The loaded level with spawn point data
     * @return Vector of Entity shared_ptrs ready for renderer
     */
    static std::vector<std::shared_ptr<Entity>> SpawnFromLevel(
        const Level& level);

    /**
     * Spawn a single entity (e.g. from block hit).
     * @param def Entity definition
     * @param worldX World X position
     * @param worldY World Y position
     * @param direction 0=Left, 1=Right, 2=None
     * @param fromBlock Whether spawned from a block
     * @param levelName Level name for sprite path resolution (default: "1-1")
     * @return Entity shared_ptr or nullptr if def is invalid
     */
    static std::shared_ptr<Entity> SpawnEntity(
        const EntityDef& def, float worldX, float worldY, int direction = 1,
        bool fromBlock = false, const std::string& levelName = "1-1");

    /**
     * Build a fully-configured EntityDef for a runtime-spawned projectile.
     *
     * Centralises all the inline EntityDef construction that was previously
     * scattered across PlayingSceneHandler so adding a new projectile type
     * only requires changing this one factory method.
     *
     * @param spawnType  EntityType::FIRE or EntityType::AXE_PROJECTILE
     * @param isEnemy    true = enemy projectile (Bowser fire / AxeKoopa axe)
     * @param level      Used to look up an existing CSV definition first
     * @return           Populated EntityDef; name is empty on unknown type
     */
    static EntityDef MakeProjectileDef(EntityType spawnType, bool isEnemy,
                                       const Level& level);

    /**
     * Spawn and configure a projectile entity (e.g. fireballs, thrown axes).
     * Calculates targets and sets speed, gravity, velocities based on
     * projectile type. For entity-spawned projectiles (Bowser fire, AxeKoopa
     * axe).
     *
     * @param spawner   The entity that is spawning the projectile
     * @param spawnType EntityType of the projectile (e.g. FIRE, AXE_PROJECTILE)
     * @param spawnX    World X position
     * @param spawnY    World Y position
     * @param spawnDir  Spawn direction (0=Left, 1=Right)
     * @param player    Reference to player for targeting
     * @param level     Level reference to check definitions
     * @param levelName Current level name
     * @return          Configured Entity shared_ptr or nullptr on failure
     */
    static std::shared_ptr<Entity> SpawnProjectile(
        const std::shared_ptr<Entity>& spawner, EntityType spawnType,
        float spawnX, float spawnY, int spawnDir, const Player& player,
        const Level& level, const std::string& levelName);

    /**
     * Spawn a player-fired projectile (e.g. fireball from Fire Mario).
     *
     * Separates the player-projectile path from the entity-projectile path so
     * PlayingSceneHandler does not need to build EntityDef inline.
     * Uses MakeProjectileDef internally — the single source of truth for all
     * projectile construction.
     *
     * @param player    Player that is firing (reserved for future targeting)
     * @param spawnType EntityType of the projectile (currently only FIRE)
     * @param spawnX    World X spawn position
     * @param spawnY    World Y spawn position
     * @param dir       Direction: 1 = right, 0 = left
     * @param level     Level reference for CSV lookup
     * @param levelName Current level name for sprite resolution
     * @return          Configured Entity shared_ptr, or nullptr on failure
     */
    static std::shared_ptr<Entity> SpawnFromPlayer(
        const Player& player, EntityType spawnType, float spawnX, float spawnY,
        int dir, const Level& level, const std::string& levelName);
};

}  // namespace Mario

#endif  // MARIO_ENTITY_FACTORY_HPP
