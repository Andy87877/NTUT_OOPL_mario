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

    for (size_t i = 0; i < entities.size(); ++i) {
        EntityState& e1 = entities[i]->GetState();
        if (!e1.IsActive()) continue;

        bool e1Near = true;
        if (enableCulling) {
            float x1 = e1.GetX();
            e1Near = (x1 >= leftBound && x1 <= rightBound);
        }

        for (size_t j = i + 1; j < entities.size(); ++j) {
            EntityState& e2 = entities[j]->GetState();
            if (!e2.IsActive()) continue;

            if (enableCulling) {
                float x2 = e2.GetX();
                bool e2Near = (x2 >= leftBound && x2 <= rightBound);
                if (!e1Near && !e2Near) continue;  // both far off-screen
            }

            if (!e1.GetHitbox().Intersects(e2.GetHitbox())) continue;

            // --- Fireball vs Enemy -------------------------------------------
            if (entities[i]->GetDef().type == EntityType::FIRE &&
                !e1.IsEnemy() && e2.IsEnemy()) {
                if (entities[j]->GetDef().type == EntityType::FIRE) continue; // Bowser fire is immune to player fireballs
                bool handled = entities[j]->GetBehavior() &&
                               entities[j]->GetBehavior()->OnFireballHit(e2);
                e1.Delete();  // always remove fireball on contact
                if (!handled && !e2.IsDead()) {
                    e2.TriggerDeath(EnemyDeathCause::FIREBALL);
                    AudioManager::GetInstance().PlaySFX(SFXName::Kick);
                    gameState.AddScore(e2.GetScoreWorth());
                }
            } else if (entities[j]->GetDef().type == EntityType::FIRE &&
                       !e2.IsEnemy() && e1.IsEnemy()) {
                if (entities[i]->GetDef().type == EntityType::FIRE) continue; // Bowser fire is immune to player fireballs
                bool handled = entities[i]->GetBehavior() &&
                               entities[i]->GetBehavior()->OnFireballHit(e1);
                e2.Delete();  // always remove fireball on contact
                if (!handled && !e1.IsDead()) {
                    e1.TriggerDeath(EnemyDeathCause::FIREBALL);
                    AudioManager::GetInstance().PlaySFX(SFXName::Kick);
                    gameState.AddScore(e1.GetScoreWorth());
                }
            }

            // --- Moving Koopa Shell vs Enemy ---------------------------------
            if (IsMovingShell(entities[i], e1) && e2.IsEnemy()) {
                if (entities[j]->GetDef().type != EntityType::KOOPA_SHELL &&
                    entities[j]->GetDef().type != EntityType::FIRE) {
                    e2.TriggerDeath(EnemyDeathCause::SHELL_HIT);
                    AudioManager::GetInstance().PlaySFX(SFXName::Kick);
                    gameState.AddScore(e2.GetScoreWorth());
                }
            } else if (IsMovingShell(entities[j], e2) && e1.IsEnemy()) {
                if (entities[i]->GetDef().type != EntityType::KOOPA_SHELL &&
                    entities[i]->GetDef().type != EntityType::FIRE) {
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
    bool shellType = (entity->GetDef().type == EntityType::KOOPA_SHELL);
    bool koopaShellMode = state.IsInShellMode();
    return (shellType || koopaShellMode) && std::abs(state.GetVelX()) > 0.1f;
}

}  // namespace Mario
