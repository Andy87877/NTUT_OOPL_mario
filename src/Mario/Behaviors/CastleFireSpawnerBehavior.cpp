/**
 * @file CastleFireSpawnerBehavior.cpp
 * @brief Implementation of Stage 8-4 periodic off-screen fire spawner.
 * @inheritance IEntityBehavior <- CastleFireSpawnerBehavior
 */
#include "Mario/Behaviors/CastleFireSpawnerBehavior.hpp"

#include "Mario/Services/AudioManager.hpp"
#include "Mario/Level/EntityState.hpp"
#include "Mario/Core/GameConfig.hpp"
#include "Mario/Player/Player.hpp"
#include "Util/Logger.hpp"

namespace Mario {

CastleFireSpawnerBehavior::CastleFireSpawnerBehavior()
    : m_FireballTimer(0), m_AttackCounter(0) {}

void CastleFireSpawnerBehavior::Update(EntityState& state, const Level& /*level*/,
                                       const Player& player, int /*gameTimer*/) {
    // Spawn fireballs continuously in 8-4 until Bowser is active/visible on screen (X >= 13000.0f)
    float playerX = player.GetWorldX();
    if (playerX >= 13000.0f) {
        state.Delete();  // Clean up this spawner once Bowser takes over
        return;
    }

    m_FireballTimer++;
    if (m_FireballTimer >= FIREBALL_INTERVAL) {
        m_FireballTimer = 0;

        // Determine spawn coordinates (just off-screen to the right of Mario)
        float spawnX = player.GetWorldX() + static_cast<float>(GameConfig::WINDOW_WIDTH) / 2.0f + 100.0f;

        // Cycle through three heights: 0 = high, 1 = medium, 2 = low
        int heightChoice = m_AttackCounter % 3;
        m_AttackCounter++;

        // Base spawner row height = 9 * TILE_SIZE = 405.0f
        float baseSpawnY = 9.0f * static_cast<float>(GameConfig::TILE_SIZE);
        float spawnY = baseSpawnY;

        if (heightChoice == 0) {
            spawnY += 0.2f * static_cast<float>(GameConfig::TILE_SIZE);  // High fire (414.0f)
        } else if (heightChoice == 1) {
            spawnY += 1.0f * static_cast<float>(GameConfig::TILE_SIZE);  // Medium fire (450.0f)
        } else {
            spawnY += 1.8f * static_cast<float>(GameConfig::TILE_SIZE);  // Low fire (486.0f)
        }

        // Add spawn request to queue: EntityType::FIRE, direction 0 = Left
        m_PendingSpawns.push_back({EntityType::FIRE, spawnX, spawnY, 0});

        // Play warning fire sound effect
        AudioManager::GetInstance().PlaySFX(SFXName::EnemyFire);
        LOG_DEBUG("CastleFireSpawner: Spawned off-screen fireball at ({}, {}) for playerX={}", spawnX, spawnY, playerX);
    }
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
