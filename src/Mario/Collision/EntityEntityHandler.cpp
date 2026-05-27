/**
 * @file EntityEntityHandler.cpp
 * @brief Entity-Entity collision: Fireball vs Enemy and moving Shell vs Enemy.
 *        Ported from C# Form1.cs entity-entity interaction section.
 * @inheritance EntityEntityHandler : ICollisionHandler
 */
#include "Mario/Collision/EntityEntityHandler.hpp"

#include "Mario/Services/AudioManager.hpp"
#include "Mario/Behaviors/IEntityBehavior.hpp"
#include "Mario/Level/EntityDef.hpp"
#include "Mario/Core/GameConfig.hpp"
#include "Mario/Level/GameStateManager.hpp"

namespace Mario {

// ============================================================================
// Resolve — public entry point
// ============================================================================
void EntityEntityHandler::Resolve(
    std::vector<std::shared_ptr<Entity>>& entities, GameStateManager& gameState,
    float cameraOffset) {
    const float leftBound = cameraOffset - 200.0f;
    const float rightBound = cameraOffset + GameConfig::WINDOW_WIDTH + 200.0f;
    const bool enableCulling = (cameraOffset > -9000.0f);

    // Pre-filter active on-screen entities to optimize double-loop lookup from O(N^2) to O(M^2)
    // Using thread_local to avoid repeated heap allocation inside the game loop
    thread_local std::vector<size_t> activeIndices;
    activeIndices.clear();
    for (size_t i = 0; i < entities.size(); ++i) {
        if (!entities[i]) continue;
        EntityState& es = entities[i]->GetState();
        if (!es.IsActive()) continue;

        if (enableCulling) {
            float x = es.GetX();
            if (x < leftBound || x > rightBound) continue;
        }
        activeIndices.push_back(i);
    }

    for (size_t idx1 = 0; idx1 < activeIndices.size(); ++idx1) {
        size_t i = activeIndices[idx1];
        EntityState& e1 = entities[i]->GetState();

        for (size_t idx2 = idx1 + 1; idx2 < activeIndices.size(); ++idx2) {
            size_t j = activeIndices[idx2];
            EntityState& e2 = entities[j]->GetState();

            if (!entities[i]->GetHitbox().Intersects(entities[j]->GetHitbox())) continue;

            auto* b1 = entities[i]->GetBehavior();
            auto* b2 = entities[j]->GetBehavior();

            // --- Fireball vs Enemy -------------------------------------------
            if (b1 && b1->IsPlayerFireball() && e2.IsEnemy()) {
                if (b2 && b2->IsEnemyProjectile()) continue; // Bowser fire is immune to player fireballs
                bool handled = b2 && b2->OnFireballHit(e2);
                e1.Delete();  // always remove fireball on contact
                if (!handled && !e2.IsDead()) {
                    e2.TriggerDeath(EnemyDeathCause::FIREBALL);
                    AudioManager::GetInstance().PlaySFX(SFXName::Kick);
                    gameState.AddScore(e2.GetScoreWorth());
                }
            } else if (b2 && b2->IsPlayerFireball() && e1.IsEnemy()) {
                if (b1 && b1->IsEnemyProjectile()) continue; // Bowser fire is immune to player fireballs
                bool handled = b1 && b1->OnFireballHit(e1);
                e2.Delete();  // always remove fireball on contact
                if (!handled && !e1.IsDead()) {
                    e1.TriggerDeath(EnemyDeathCause::FIREBALL);
                    AudioManager::GetInstance().PlaySFX(SFXName::Kick);
                    gameState.AddScore(e1.GetScoreWorth());
                }
            }

            // --- Moving Koopa Shell vs Enemy ---------------------------------
            if (IsMovingShell(entities[i], e1) && e2.IsEnemy()) {
                if ((!b2 || !b2->IsShell()) && (!b2 || !b2->IsEnemyProjectile())) {
                    e2.TriggerDeath(EnemyDeathCause::SHELL_HIT);
                    AudioManager::GetInstance().PlaySFX(SFXName::Kick);
                    gameState.AddScore(e2.GetScoreWorth());
                }
            } else if (IsMovingShell(entities[j], e2) && e1.IsEnemy()) {
                if ((!b1 || !b1->IsShell()) && (!b1 || !b1->IsEnemyProjectile())) {
                    e1.TriggerDeath(EnemyDeathCause::SHELL_HIT);
                    AudioManager::GetInstance().PlaySFX(SFXName::Kick);
                    gameState.AddScore(e1.GetScoreWorth());
                }
            }
        }
    }
}

// ============================================================================
// IsMovingShell
// ============================================================================
bool EntityEntityHandler::IsMovingShell(const std::shared_ptr<Entity>& entity,
                                        const EntityState& state) {
    auto* behavior = entity->GetBehavior();
    bool isShell = (behavior && behavior->IsShell()) || state.IsInShellMode();
    return isShell && std::abs(state.GetVelX()) > 0.1f;
}

}  // namespace Mario
