/**
 * @file EntityFactory.cpp
 * @brief Implementation of EntityFactory.
 *        Creates Entity instances from Level spawn data.
 *        Matches C# Form1.cs entity spawning logic.
 * @inheritance None (factory pattern)
 */
#include "Mario/EntityFactory.hpp"

#include "Util/Logger.hpp"

namespace Mario {

std::vector<std::shared_ptr<Entity>> EntityFactory::SpawnFromLevel(
    const Level& level) {

    std::vector<std::shared_ptr<Entity>> entities;

    for (const auto& sp : level.GetSpawnPoints()) {
        // Skip non-entity spawn points (Flag, UnderCoin, etc.)
        if (sp.entityName == "Flag" || sp.entityName == "UnderCoin") {
            continue;
        }

        // Look up entity definition
        const EntityDef& def = level.GetEntityDefByName(sp.entityName);
        if (def.name.empty()) {
            LOG_WARN("EntityFactory: Unknown entity '{}' at ({}, {})",
                     sp.entityName, sp.gridX, sp.gridY);
            continue;
        }

        // Determine direction: most enemies start moving left
        int direction = 0; // Left by default for enemies

        auto entity = SpawnEntity(def, sp.worldX, sp.worldY, direction, false);
        if (entity) {
            entities.push_back(entity);
            LOG_DEBUG("Spawned {} at ({}, {})", sp.entityName, sp.worldX, sp.worldY);
        }
    }

    LOG_INFO("EntityFactory: Spawned {} entities", entities.size());
    return entities;
}

std::shared_ptr<Entity> EntityFactory::SpawnEntity(
    const EntityDef& def, float worldX, float worldY,
    int direction, bool fromBlock) {

    if (def.name.empty()) return nullptr;

    return std::make_shared<Entity>(def, worldX, worldY, direction, fromBlock);
}

} // namespace Mario
