/**
 * @file EntityFactory.cpp
 * @brief Implementation of EntityFactory.
 *        Creates Entity instances from Level spawn data.
 *        Configures behavior strategies for enemies via Strategy Pattern.
 * @inheritance None (factory pattern)
 */
#include "Mario/EntityFactory.hpp"

#include "Mario/Behaviors/AxeKoopaBehavior.hpp"
#include "Mario/Behaviors/BowserBehavior.hpp"
#include "Mario/Behaviors/DefaultEntityBehavior.hpp"
#include "Mario/Behaviors/EnemyBehavior.hpp"
#include "Mario/Behaviors/IEntityBehavior.hpp"
#include "Mario/Behaviors/ItemBehavior.hpp"
#include "Mario/Behaviors/ParaKoopaBehavior.hpp"
#include "Mario/Behaviors/PrincessBehavior.hpp"
#include "Util/Logger.hpp"

namespace Mario {

std::vector<std::shared_ptr<Entity>> EntityFactory::SpawnFromLevel(
    const Level& level) {
    std::vector<std::shared_ptr<Entity>> entities;
    std::string levelName = level.GetLevelName();

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
        int direction = 0;  // Left by default for enemies

        auto entity =
            SpawnEntity(def, sp.worldX, sp.worldY, direction, false, levelName);
        if (entity) {
            entities.push_back(entity);
        }
    }

    LOG_DEBUG("EntityFactory: Spawned {} entities from level", entities.size());
    if (!entities.empty()) {
        LOG_WARN("EntityFactory: Successfully spawned {} entities",
                 entities.size());
    }
    return entities;
}

std::shared_ptr<Entity> EntityFactory::SpawnEntity(
    const EntityDef& def, float worldX, float worldY, int direction,
    bool fromBlock, const std::string& levelName) {
    if (def.name.empty()) return nullptr;

    auto entity = std::make_shared<Entity>(def, worldX, worldY, direction,
                                           fromBlock, levelName);

    if (!entity) return nullptr;

    // Configure behavior strategy based on entity type
    std::unique_ptr<IEntityBehavior> behavior;

    if (def.name == "Goomba") {
        behavior =
            std::make_unique<EnemyBehavior>(EnemyBehavior::EnemyType::GOOMBA);
    } else if (def.name == "Koopa" || def.name == "KoopaTroopa") {
        behavior = std::make_unique<EnemyBehavior>(
            EnemyBehavior::EnemyType::KOOPA_TROOPA);
    } else if (def.name == "ParaKoopa") {
        behavior = std::make_unique<ParaKoopaBehavior>();
    } else if (def.name == "AxeKoopa") {
        behavior = std::make_unique<AxeKoopaBehavior>();
    } else if (def.name == "Bowser") {
        behavior = std::make_unique<BowserBehavior>();
    } else if (def.name == "Princess") {
        behavior = std::make_unique<PrincessBehavior>();
    } else if (def.isEnemy) {
        // Generic enemy behavior fallback
        behavior =
            std::make_unique<EnemyBehavior>(EnemyBehavior::EnemyType::GOOMBA);
    } else if (def.isPowerUp || def.isCoin) {
        // Items and collectibles
        behavior = std::make_unique<ItemBehavior>();
    } else {
        // Default passive behavior
        behavior = std::make_unique<DefaultEntityBehavior>();
    }

    // Attach behavior to entity
    if (behavior) {
        entity->SetBehavior(std::move(behavior));
        LOG_DEBUG("Attached behavior to '{}'", def.name);
    }

    return entity;
}

}  // namespace Mario
