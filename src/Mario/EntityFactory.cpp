/**
 * @file EntityFactory.cpp
 * @brief Implementation of EntityFactory.
 *        Creates Entity instances from Level spawn data.
 *        Configures behavior strategies for enemies via Strategy Pattern.
 * @inheritance None (factory pattern)
 */
#include "Mario/EntityFactory.hpp"

#include "Mario/Behaviors/AxeBehavior.hpp"
#include "Mario/Behaviors/AxeKoopaBehavior.hpp"
#include "Mario/Behaviors/BowserBehavior.hpp"
#include "Mario/Behaviors/DefaultEntityBehavior.hpp"
#include "Mario/Behaviors/EnemyBehavior.hpp"
#include "Mario/Behaviors/FireballBehavior.hpp"
#include "Mario/Behaviors/IEntityBehavior.hpp"
#include "Mario/Behaviors/ItemBehavior.hpp"
#include "Mario/Behaviors/KoopaBehavior.hpp"
#include "Mario/Behaviors/ParaKoopaBehavior.hpp"
#include "Mario/Behaviors/ParticleDebris.hpp"
#include "Mario/Behaviors/PrincessBehavior.hpp"
#include "Util/Logger.hpp"

namespace Mario {

std::vector<std::shared_ptr<Entity>> EntityFactory::SpawnFromLevel(
    const Level& level) {
    std::vector<std::shared_ptr<Entity>> entities;
    std::string levelName = level.GetLevelName();

    for (const auto& sp : level.GetSpawnPoints()) {
        // Handle Flag entity - should only appear in 1-1
        if (sp.entityName == "Flag") {
            // Only create Flag in 1-1 level
            if (levelName == "1-1" ||
                levelName.find("1-1") != std::string::npos) {
                const EntityDef& def = level.GetEntityDefByName("Flag");
                if (!def.name.empty()) {
                    auto flag = SpawnEntity(def, sp.worldX, sp.worldY, 0, false,
                                            levelName);
                    if (flag) {
                        LOG_DEBUG("Spawned Flag at worldX={}, worldY={}",
                                  sp.worldX, sp.worldY);
                        entities.push_back(flag);
                    }
                } else {
                    LOG_WARN(
                        "Flag entity definition not found in EntityList.csv");
                }
            }
            continue;
        }

        // Skip UnderCoin and other non-entity spawn points
        if (sp.entityName == "UnderCoin") {
            continue;
        }

        // Look up entity definition from EntityList.csv
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
            LOG_DEBUG("Spawned {} at worldX={}, worldY={}", sp.entityName,
                      sp.worldX, sp.worldY);
            entities.push_back(entity);
        }
    }

    LOG_DEBUG("EntityFactory: Spawned {} entities from level", entities.size());
    return entities;
}

std::shared_ptr<Entity> EntityFactory::SpawnEntity(
    const EntityDef& def, float worldX, float worldY, int direction,
    bool fromBlock, const std::string& levelName) {
    if (def.name.empty()) return nullptr;

    auto entity = std::make_shared<Entity>(def, worldX, worldY, direction,
                                           fromBlock, levelName);

    if (!entity) return nullptr;

    // Configure behavior strategy based on entity type (loaded from CSV)
    std::unique_ptr<IEntityBehavior> behavior;

    switch (def.type) {
        case EntityType::GOOMBA:
            behavior = std::make_unique<EnemyBehavior>(
                EnemyBehavior::EnemyType::GOOMBA);
            break;
        case EntityType::KOOPA_TROOPA:
            behavior = std::make_unique<KoopaBehavior>(
                KoopaBehavior::KoopaType::TROOPA);
            break;
        case EntityType::KOOPA_SHELL:
            behavior = std::make_unique<KoopaBehavior>(
                KoopaBehavior::KoopaType::SHELL);
            break;
        case EntityType::AXE_KOOPA:
            behavior = std::make_unique<AxeKoopaBehavior>();
            break;
        case EntityType::BOWSER:
            behavior = std::make_unique<BowserBehavior>();
            break;
        case EntityType::FIRE:
            behavior = std::make_unique<FireballBehavior>(
                FireballBehavior::FireballType::PLAYER);
            break;
        case EntityType::PRINCESS:
            behavior = std::make_unique<PrincessBehavior>();
            break;
        case EntityType::MUSHROOM:
        case EntityType::FIRE_FLOWER:
        case EntityType::STAR:
        case EntityType::ONE_UP:
        case EntityType::COIN:
            behavior = std::make_unique<ItemBehavior>();
            break;
        case EntityType::PARTICLE_DEBRIS:
            behavior = std::make_unique<ParticleDebris>();
            break;
        case EntityType::AXE:
            // Static axe trigger on Bowser's bridge — AxeBehavior handles
            // animation and deletion on contact; App::CheckAxeCollision()
            // triggers the bridge-collapse sequence.
            behavior = std::make_unique<AxeBehavior>();
            break;
        case EntityType::AXE_PROJECTILE:
            behavior = std::make_unique<AxeBehavior>();
            break;
        default:
            behavior = std::make_unique<DefaultEntityBehavior>();
            break;
    }

    // Attach behavior to entity
    if (behavior) {
        entity->SetBehavior(std::move(behavior));
    }

    return entity;
}

}  // namespace Mario
