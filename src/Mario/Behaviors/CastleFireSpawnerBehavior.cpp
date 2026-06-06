/**
 * @file CastleFireSpawnerBehavior.cpp
 * @brief Implementation of Stage 8-4 periodic off-screen fire spawner.
 * @inheritance IEntityBehavior <- CastleFireSpawnerBehavior
 */
#include "Mario/Behaviors/CastleFireSpawnerBehavior.hpp"

#include <cstdlib>

#include "Mario/Services/AudioManager.hpp"
#include "Mario/Level/EntityState.hpp"
#include "Mario/Core/GameConfig.hpp"
#include "Mario/Player/Player.hpp"
#include "Mario/Level/Level.hpp"
#include "Mario/Level/Block.hpp"
#include "Util/Logger.hpp"

namespace Mario {

CastleFireSpawnerBehavior::CastleFireSpawnerBehavior()
    : m_FireballTimer(0), m_AttackCounter(0) {
    m_NextSpawnInterval = 60 + std::rand() % 120; // 1 to 3 seconds initially
}

void CastleFireSpawnerBehavior::Update(EntityState& state, const Level& level,
                                       const Player& player, int /*gameTimer*/) {
    // Spawn fireballs continuously in 8-4 until Mario enters Bowser room (Room 5: X >= 12000.0f)
    float playerX = player.GetWorldX();
    if (playerX >= 12000.0f) {
        state.Delete();  // Clean up this spawner once Bowser takes over
        return;
    }

    m_FireballTimer++;
    if (m_FireballTimer >= m_NextSpawnInterval) {
        m_FireballTimer = 0;

        // Determine spawn coordinates: randomized off-screen distance ahead of Mario
        float spawnX = player.GetWorldX() + 900.0f + static_cast<float>(std::rand() % 400);

        // Target height targeting with randomized offset to keep it highly unpredictable (-200px to +200px)
        float targetY = player.GetWorldY();
        float randomOffset = static_cast<float>((std::rand() % 400) - 200);
        float spawnY = targetY + randomOffset;

        // Clamp to visible play bounds so fires do not spawn too high in the ceiling or in deep lava
        spawnY = std::max(180.0f, std::min(550.0f, spawnY));

        // Scan and adjust Y coordinate to ensure fireball does not travel inside solid block structures
        float checkY = spawnY;
        for (int attempt = 0; attempt < 8; ++attempt) {
            if (!IsYInsideBlocks(checkY, player.GetWorldX(), spawnX, level)) {
                spawnY = checkY;
                break;
            }
            // Try shifting vertically in tile increments alternately up and down
            float shift = static_cast<float>((attempt + 1) * GameConfig::TILE_SIZE * (attempt % 2 == 0 ? 1 : -1));
            checkY = spawnY + shift;
            checkY = std::max(180.0f, std::min(550.0f, checkY));
        }

        m_AttackCounter++;

        // Add spawn request to queue: EntityType::FIRE, direction 0 = Left
        m_PendingSpawns.push_back({EntityType::FIRE, spawnX, spawnY, 0});

        // Set next randomized CD interval: between 60 frames (1s) and 180 frames (3s)
        m_NextSpawnInterval = 60 + std::rand() % 120;

        // Play warning fire sound effect
        AudioManager::GetInstance().PlaySFX(SFXName::EnemyFire);
        LOG_DEBUG("CastleFireSpawner: Spawned off-screen fireball at ({}, {}) for playerX={}, nextCD={}", 
                  spawnX, spawnY, playerX, m_NextSpawnInterval);
    }
}

bool CastleFireSpawnerBehavior::IsYInsideBlocks(float y, float startX, float endX, const Level& level) const {
    // Sample along the fireball's travel path to check for solid block intersections
    for (float x = startX; x <= endX; x += static_cast<float>(GameConfig::TILE_SIZE)) {
        const Block* b = level.GetBlockAtWorld(x, y);
        if (b && b->IsSolid()) {
            return true;
        }
    }
    return false;
}

void CastleFireSpawnerBehavior::ConfigureSpawnedProjectile(EntityState& projectileState, int spawnDir) const {
    // Speed varies dynamically between 2.5f (slow) and 6.0f (extremely fast)
    float speed = 2.5f + static_cast<float>(std::rand() % 350) / 100.0f;
    
    // Allow vertical slope trajectory (diagonal movement): -0.8f to +0.8f
    float velY = (static_cast<float>(std::rand() % 200) - 100.0f) / 100.0f * 0.8f;
    projectileState.SetVelY(velY);
    projectileState.SetVelX(spawnDir == 1 ? speed : -speed);
}

bool CastleFireSpawnerBehavior::OnPlayerCollision(EntityState& /*state*/, Player& /*player*/,
                                                 bool /*isFromAbove*/) {
    return false;
}

bool CastleFireSpawnerBehavior::ConsumeSpawnRequest(EntityType& outType, float& outX,
                                                   float& outY, int& outDir) {
    if (m_PendingSpawns.empty()) return false;
    auto req = m_PendingSpawns.front();
    m_PendingSpawns.erase(m_PendingSpawns.begin());
    outType = req.type;
    outX = req.x;
    outY = req.y;
    outDir = req.dir;
    return true;
}

std::unique_ptr<IEntityBehavior> CastleFireSpawnerBehavior::Clone() const {
    return std::make_unique<CastleFireSpawnerBehavior>(*this);
}

}  // namespace Mario
