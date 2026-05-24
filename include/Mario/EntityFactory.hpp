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

#include "Mario/Entity.hpp"
#include "Mario/Level.hpp"

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
};

}  // namespace Mario

#endif  // MARIO_ENTITY_FACTORY_HPP
