/**
 * @file EntityFactory.cpp
 * @brief Implementation of EntityFactory.
 *        Creates Entity instances from Level spawn data.
 *        Configures behavior strategies for enemies via Strategy Pattern.
 * @inheritance None (factory pattern)
 */
#include "Mario/Level/EntityFactory.hpp"

#include <cmath>

#include "Mario/Level/LevelConfig.hpp"

#include "Mario/Level/BehaviorRegistry.hpp"
#include "Mario/Behaviors/IEntityBehavior.hpp"
#include "Mario/Core/GameConfig.hpp"
#include "Mario/Level/EnemyDeathStyleFactory.hpp"
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
        // Handle Flag entity - only created if flagpole sequence exists in this level
        if (sp.entityName == "Flag") {
            if (LevelConfig::HasFlag(levelName)) {
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

            if (LevelConfig::HasBoss(levelName)) {
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

    // Spawn Podoboos based on configuration data (decoupled from hardcoded level checks)
    auto podobooSpawns = LevelConfig::GetPodobooSpawns(levelName);
    if (!podobooSpawns.empty()) {
        const EntityDef& podobooDef = level.GetEntityDefByName("Podoboo");
        if (!podobooDef.name.empty()) {
            for (auto [col, row] : podobooSpawns) {
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
    }

    if (LevelConfig::HasBoss(levelName)) {
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
    }

    if (LevelConfig::SpawnCastleFireSpawner(levelName)) {
        // Automatically spawn off-screen CastleFireSpawner if configured
        EntityDef spawnerDef;
        spawnerDef.id = -1;
        spawnerDef.name = "CastleFireSpawner";
        spawnerDef.type = EntityType::CASTLE_FIRE_SPAWNER;
        spawnerDef.isStatic = true;
        spawnerDef.doesCollide = false;

        auto spawner = SpawnEntity(spawnerDef, 0.0f, 0.0f, 0, false, levelName);
        if (spawner) {
            LOG_INFO(
                "Castle: Automatically spawned off-screen "
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
    float widthOverride = LevelConfig::GetRenderTargetWidthOverride(levelName, localDef.type, localDef.isEnemy);
    if (widthOverride > 0.0f) {
        localDef.renderTargetWidth = widthOverride;
    }

    // Mark entities that must render behind blocks (Z_BLOCK-1 layer).
    // Keeping this in the Factory preserves OCP: Entity.cpp never checks type.
    if (localDef.type == EntityType::PIRANHA_PLANT ||
        localDef.type == EntityType::COIN) {
        localDef.rendersBehindBlocks = true;
    }

    // PiranhaPlant always has a 2×2 tile hitbox regardless of sprite size.
    // Storing it here keeps the EntityType knowledge inside the Factory (OCP).
    if (localDef.type == EntityType::PIRANHA_PLANT) {
        localDef.fixedHitboxTiles = 2;
    }

    // Dynamic config overrides for Princess
    if (localDef.type == EntityType::PRINCESS) {
        localDef.animBuffer = 30; // Slow down Princess animation rate (once every 30 ticks)
    }

    auto entity = std::make_shared<Entity>(localDef, worldX, worldY, direction,
                                           fromBlock, levelName);

    if (!entity) return nullptr;
    // Configure behavior strategy based on behavior registry (OCP)
    auto behavior = BehaviorRegistry::Create(localDef.type, localDef);

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

    // Use polymorphic identity query — no EntityType switch needed here (OCP).
    // Bowser, AxeKoopa, and CastleFireSpawner all override
    // IsEnemySpawner()=true.
    auto* spawnBehavior = spawner->GetBehavior();
    bool isEnemyProjectile = spawnBehavior && spawnBehavior->IsEnemySpawner();

    EntityDef def = MakeProjectileDef(spawnType, isEnemyProjectile, level);
    if (def.name.empty()) return nullptr;

    auto spawned = SpawnEntity(def, spawnX, spawnY, spawnDir, false, levelName);
    if (!spawned) return nullptr;

    if (spawnType == EntityType::FIRE) {
        float speed = isEnemyProjectile ? 3.0f : 4.0f;
        
        // Randomize speed and add diagonal angles if spawned by the 8-4 off-screen CastleFireSpawner
        if (spawnBehavior && std::string(spawnBehavior->GetName()) == "CastleFireSpawnerBehavior") {
            // Speed varies dynamically between 2.5f (slow) and 6.0f (extremely fast)
            speed = 2.5f + static_cast<float>(std::rand() % 350) / 100.0f;
            
            // Allow vertical slope trajectory (diagonal movement): -0.8f to +0.8f
            float velY = (static_cast<float>(std::rand() % 200) - 100.0f) / 100.0f * 0.8f;
            spawned->GetState().SetVelY(velY);
        }
        
        if (isEnemyProjectile) {
            spawned->GetState().SetGravity(false);
        }
        spawned->GetState().SetVelX(spawnDir == 1 ? speed : -speed);
    } else if (spawnType == EntityType::AXE_PROJECTILE) {
        // High-force, punchy parabolic arc authentic to classic NES SMB
        constexpr float kThrowSpeed = 3.5f;
        constexpr float kLaunchVelY = 10.0f;
        spawned->GetState().SetVelX(spawnDir == 1 ? kThrowSpeed : -kThrowSpeed);
        spawned->GetState().SetFallHeight(kLaunchVelY);
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
