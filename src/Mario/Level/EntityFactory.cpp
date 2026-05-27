/**
 * @file EntityFactory.cpp
 * @brief Implementation of EntityFactory.
 *        Creates Entity instances from Level spawn data.
 *        Configures behavior strategies for enemies via Strategy Pattern.
 * @inheritance None (factory pattern)
 */
#include "Mario/Level/EntityFactory.hpp"

#include <cmath>

#include "Mario/Behaviors/BowserBehavior.hpp"
#include "Mario/Behaviors/CastleFireSpawnerBehavior.hpp"
#include "Mario/Behaviors/DefaultEntityBehavior.hpp"
#include "Mario/Behaviors/GoombaBehavior.hpp"
#include "Mario/Behaviors/FireballBehavior.hpp"
#include "Mario/Behaviors/IEntityBehavior.hpp"
#include "Mario/Behaviors/ItemBehaviors.hpp"
#include "Mario/Behaviors/KoopaFamily.hpp"
#include "Mario/Behaviors/ParticleDebris.hpp"
#include "Mario/Behaviors/PiranhaPlantBehavior.hpp"
#include "Mario/Behaviors/PodobooBehavior.hpp"
#include "Mario/Behaviors/StaticEntityBehaviors.hpp"
#include "Mario/Level/EnemyDeathStyleFactory.hpp"
#include "Mario/Core/GameConfig.hpp"
#include "Mario/Player/Player.hpp"
#include "Util/Logger.hpp"

namespace Mario {

std::vector<std::shared_ptr<Entity>> EntityFactory::SpawnFromLevel(
    const Level& level) {
    std::vector<std::shared_ptr<Entity>> entities;
    std::string levelName = level.GetLevelName();
    bool hasBowser = false;
    float bowserAnchorX = -1.0f;
    float bowserAnchorY = -1.0f;

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

            if (levelName == "8-4") {
                if (sp.entityName == "Bowser") {
                    hasBowser = true;
                }
                if (sp.entityName == "Axe" && bowserAnchorX < 0.0f) {
                    bowserAnchorX = sp.worldX - static_cast<float>(
                                                    GameConfig::TILE_SIZE * 7);
                    bowserAnchorY = sp.worldY;
                }
                if (sp.entityName == "Princess" && bowserAnchorX < 0.0f) {
                    bowserAnchorX = sp.worldX - static_cast<float>(
                                                    GameConfig::TILE_SIZE * 7);
                    bowserAnchorY = sp.worldY;
                }
            }
        }
    }

    // 8-4 castle: Spawn Podoboos at hardcoded lava pit positions.
    // Not in CSV spawn data — baked into 8-4 level design.
    // row 13 = main castle lava (y = 13 * TILE_SIZE)
    // row 11 = boss pit lava   (y = 11 * TILE_SIZE)
    if (levelName == "8-4") {
        const EntityDef& podobooDef = level.GetEntityDefByName("Podoboo");
        if (!podobooDef.name.empty()) {
            static constexpr std::pair<int, int> kPodobooTiles[] = {
                {68, 13}, {145, 13}, {222, 13}, {336, 11}};
            for (auto [col, row] : kPodobooTiles) {
                float wx = static_cast<float>(col * GameConfig::TILE_SIZE);
                float wy = static_cast<float>(row * GameConfig::TILE_SIZE);
                auto pb = SpawnEntity(podobooDef, wx, wy, 2, false, levelName);
                if (pb) {
                    LOG_DEBUG("Spawned Podoboo at world ({}, {})", wx, wy);
                    entities.push_back(pb);
                }
            }
        } else {
            LOG_WARN("Podoboo entity definition not found in EntityList.csv");
        }

        if (!hasBowser) {
            const EntityDef& bowserDef = level.GetEntityDefByName("Bowser");
            if (!bowserDef.name.empty()) {
                if (bowserAnchorX < 0.0f) {
                    bowserAnchorX =
                        level.GetPlayerSpawnX() +
                        static_cast<float>(GameConfig::TILE_SIZE * 24);
                    bowserAnchorY = level.GetPlayerSpawnY();
                }

                auto fallbackBowser =
                    SpawnEntity(bowserDef, bowserAnchorX, bowserAnchorY, 0,
                                false, levelName);
                if (fallbackBowser) {
                    LOG_WARN(
                        "8-4 fallback: Bowser spawner missing, inserted Bowser "
                        "at ({}, {}).",
                        bowserAnchorX, bowserAnchorY);
                    entities.push_back(fallbackBowser);
                }
            }
        }

        // Automatically spawn off-screen CastleFireSpawner at the start of 8-4
        EntityDef spawnerDef;
        spawnerDef.id = -1;
        spawnerDef.name = "CastleFireSpawner";
        spawnerDef.type = EntityType::CASTLE_FIRE_SPAWNER;
        spawnerDef.isStatic = true;
        spawnerDef.doesCollide = false;

        auto spawner = SpawnEntity(spawnerDef, 0.0f, 0.0f, 0, false, levelName);
        if (spawner) {
            LOG_INFO(
                "8-4 Castle: Automatically spawned off-screen "
                "CastleFireSpawner.");
            entities.push_back(spawner);
        }
    }

    LOG_DEBUG("EntityFactory: Spawned {} entities from level", entities.size());
    return entities;
}

std::shared_ptr<Entity> EntityFactory::SpawnEntity(
    const EntityDef& def, float worldX, float worldY, int direction,
    bool fromBlock, const std::string& levelName) {
    if (def.name.empty()) return nullptr;

    // -------------------------------------------------------------------------
    // Build a local def copy and inject level-specific rendering overrides
    // (renderTargetWidth) so Entity.cpp is free of level/name string checks.
    // This is the single place where "8-4 needs bigger Bowser" knowledge lives.
    // -------------------------------------------------------------------------
    EntityDef localDef = def;
    if (levelName == "8-4") {
        // Default for all 8-4 entities: 1-tile-wide sprite scaling.
        localDef.renderTargetWidth = 32.0f;
        switch (localDef.type) {
            case EntityType::BOWSER:
                localDef.renderTargetWidth = 64.0f;  // 2-tile boss
                break;
            case EntityType::FIRE:
                // Bowser's fire is larger than a player fireball.
                if (localDef.isEnemy) localDef.renderTargetWidth = 48.0f;
                break;
            case EntityType::PIRANHA_PLANT:
                localDef.renderTargetWidth = 64.0f;  // 2-tile-wide pipe plant
                break;
            default:
                break;  // stays at 32.0f
        }
    } else if (localDef.type == EntityType::PIRANHA_PLANT) {
        // PiranhaPlant in 1-1/1-2: same visual scale as the 8-4 path.
        localDef.renderTargetWidth = 64.0f;
    }

    auto entity = std::make_shared<Entity>(localDef, worldX, worldY, direction,
                                           fromBlock, levelName);

    if (!entity) return nullptr;

    // Configure behavior strategy based on entity type (loaded from CSV)
    std::unique_ptr<IEntityBehavior> behavior;

    switch (localDef.type) {
        case EntityType::GOOMBA:
            behavior = std::make_unique<GoombaBehavior>();
            break;
        case EntityType::KOOPA_TROOPA:
            behavior = std::make_unique<KoopaBehavior>(
                KoopaBehavior::KoopaType::TROOPA);
            break;
        case EntityType::PARAKOOPA:
            behavior = std::make_unique<ParaKoopaBehavior>();
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
        case EntityType::CASTLE_FIRE_SPAWNER:
            behavior = std::make_unique<CastleFireSpawnerBehavior>();
            break;
        case EntityType::FIRE:
            behavior = std::make_unique<FireballBehavior>(
                localDef.isEnemy ? FireballBehavior::FireballType::BOWSER
                                 : FireballBehavior::FireballType::PLAYER);
            break;
        case EntityType::PRINCESS:
            behavior = std::make_unique<PrincessBehavior>();
            break;
        case EntityType::MUSHROOM:
            behavior = std::make_unique<MushroomBehavior>();
            break;
        case EntityType::FIRE_FLOWER:
            behavior = std::make_unique<FireFlowerBehavior>();
            break;
        case EntityType::STAR:
            behavior = std::make_unique<StarBehavior>();
            break;
        case EntityType::ONE_UP:
            behavior = std::make_unique<OneUpBehavior>();
            break;
        case EntityType::COIN:
            behavior = std::make_unique<CoinBehavior>();
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
            // Thrown axe projectile: gravity + velocity applied by EntityState Tick()
            behavior = std::make_unique<AxeProjectileBehavior>();
            break;
        case EntityType::PIRANHA_PLANT:
            behavior = std::make_unique<PiranhaPlantBehavior>();
            break;
        case EntityType::PODOBOO:
            behavior = std::make_unique<PodobooBehavior>();
            break;
        default:
            behavior = std::make_unique<DefaultEntityBehavior>();
            break;
    }

    // Attach behavior to entity
    if (behavior) {
        entity->SetBehavior(std::move(behavior));
    }

    // Inject death style via dedicated factory to keep responsibilities clean.
    entity->GetState().SetDeathAnimationStrategy(
        EnemyDeathStyleFactory::CreateFor(localDef.type));

    return entity;
}

// ============================================================================
// MakeProjectileDef
// Builds a fully-configured EntityDef for a runtime projectile.
// All inline EntityDef construction that was in PlayingSceneHandler lives here,
// so adding/changing a projectile type only requires editing this method.
// ============================================================================
EntityDef EntityFactory::MakeProjectileDef(EntityType spawnType, bool isEnemy,
                                           const Level& level) {
    // Map EntityType -> canonical lookup name used in EntityList.csv
    std::string lookupName;
    switch (spawnType) {
        case EntityType::FIRE:
            lookupName = isEnemy ? "Bowser_fire" : "Fire";
            break;
        case EntityType::AXE_PROJECTILE:
            lookupName = "Axe_throw";
            break;
        default:
            return EntityDef{};  // Unknown type — return empty def
    }

    // Try to find a matching CSV definition first
    EntityDef def = level.GetEntityDefByName(lookupName);

    if (def.name.empty()) {
        // CSV entry missing — build a minimal fallback definition
        def.id = -1;
        def.name = lookupName;
        def.type = spawnType;
        def.doesCollide = true;
        def.isStatic = false;
        def.isEnemy = isEnemy;

        switch (spawnType) {
            case EntityType::FIRE:
                def.isAnimated = true;
                def.animFrames = isEnemy ? 2 : 4;
                def.animBuffer = isEnemy ? 6 : 3;
                break;
            case EntityType::AXE_PROJECTILE:
                def.isAnimated = false;
                def.animFrames = 0;
                break;
            default:
                break;
        }
    } else {
        // Patch fields that differ at runtime from the static CSV values
        def.isEnemy = isEnemy;
        def.type = spawnType;
        if (spawnType == EntityType::AXE_PROJECTILE) {
            def.isStatic = false;
            def.doesCollide = true;
        }
    }

    return def;
}

std::shared_ptr<Entity> EntityFactory::SpawnProjectile(
    const std::shared_ptr<Entity>& spawner, EntityType spawnType, float spawnX,
    float spawnY, int spawnDir, const Player& player, const Level& level,
    const std::string& levelName) {
    if (!spawner) return nullptr;

    bool isEnemyProjectile =
        spawner->GetDef().type == EntityType::BOWSER ||
        spawner->GetDef().type == EntityType::AXE_KOOPA ||
        spawner->GetDef().type == EntityType::CASTLE_FIRE_SPAWNER;

    EntityDef def = MakeProjectileDef(spawnType, isEnemyProjectile, level);
    if (def.name.empty()) return nullptr;

    auto spawned = SpawnEntity(def, spawnX, spawnY, spawnDir, false, levelName);
    if (!spawned) return nullptr;

    if (spawnType == EntityType::FIRE) {
        float speed = isEnemyProjectile ? 3.0f : 4.0f;
        if (isEnemyProjectile) {
            spawned->GetState().SetGravity(false);
        }
        spawned->GetState().SetVelX(spawnDir == 1 ? speed : -speed);
    } else if (spawnType == EntityType::AXE_PROJECTILE) {
        // Parabolic arc targeted at Mario's current position
        float dx = std::abs(player.GetWorldX() - spawnX);
        float launchVelY =
            std::max(8.0f, std::min(15.0f, 8.0f + (dx / 100.0f) * 1.5f));
        float flightTime = launchVelY * 7.5f;
        float throwSpeed = std::max(1.5f, std::min(6.5f, dx / flightTime));
        spawned->GetState().SetVelX(spawnDir == 1 ? throwSpeed : -throwSpeed);
        spawned->GetState().SetFallHeight(launchVelY);
    }

    return spawned;
}

// ============================================================================
// SpawnFromPlayer
// Spawns a player-fired projectile (e.g. fireball from Fire Mario).
// Separates the player-projectile path from the entity-projectile path so
// PlayingSceneHandler does not need to construct EntityDef inline.
// ============================================================================
std::shared_ptr<Entity> EntityFactory::SpawnFromPlayer(
    const Player& /*player*/, EntityType spawnType, float spawnX, float spawnY,
    int dir, const Level& level, const std::string& levelName) {
    // MakeProjectileDef is the single source-of-truth for all projectile defs.
    // isEnemy=false because this projectile is always fired by the player.
    EntityDef def = MakeProjectileDef(spawnType, false, level);
    if (def.name.empty()) return nullptr;

    auto entity = SpawnEntity(def, spawnX, spawnY, dir, false, levelName);
    if (!entity) return nullptr;

    if (spawnType == EntityType::FIRE) {
        // Player fireball speed matches C# reference (Form1.cs fireball logic).
        constexpr float kPlayerFireSpeed = 5.0f;
        entity->GetState().SetVelX(dir == 1 ? kPlayerFireSpeed
                                            : -kPlayerFireSpeed);
    }

    return entity;
}

}  // namespace Mario
