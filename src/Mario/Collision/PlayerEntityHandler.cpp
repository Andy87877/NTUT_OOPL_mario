/**
 * @file PlayerEntityHandler.cpp
 * @brief Player-Entity collision: stomp, damage, power-up, coin collection.
 *        Logic ported from C# Form1.cs entity interaction loop.
 * @inheritance PlayerEntityHandler : ICollisionHandler
 */
#include "Mario/Collision/PlayerEntityHandler.hpp"

#include "Mario/Services/AudioManager.hpp"
#include "Mario/Behaviors/IEntityBehavior.hpp"
#include "Mario/Core/Camera.hpp"
#include "Mario/Level/EntityDef.hpp"
#include "Mario/Core/GameConfig.hpp"
#include "Mario/Level/GameStateManager.hpp"
#include "Mario/Core/PhysicsEngine.hpp"
#include "Mario/UI/UIManager.hpp"

namespace Mario {

// ============================================================================
// Resolve — public entry point
// ============================================================================
void PlayerEntityHandler::Resolve(
    Player& player, std::vector<std::shared_ptr<Entity>>& entities,
    Camera& camera, GameStateManager& gameState, UIManager& uiManager) {
    if (player.GetState().IsDead()) return;

    PlayerState& ps = player.GetState();
    AABB playerBox = ps.GetHitbox();

    // Reset stomp combo the frame Mario lands — stomp detection requires
    // !ps.IsGrounded(), so the reset can never race with a new stomp.
    if (ps.IsGrounded()) {
        m_StompCombo = 0;
    }

    // Pre-calculate stomp state at the beginning of the frame so that multiple stomps
    // in a single frame are correctly processed (instead of subsequent stomps turning into
    // damage because of the bounce upward velocity change).
    bool isStomp = !ps.IsGrounded() && ps.GetFallHeight() <= 0.0;

    for (auto& entity : entities) {
        EntityState& es = entity->GetState();
        if (!es.IsActive()) continue;
        if (es.IsHidden()) continue;  // PiranhaPlant inside pipe — not hittable

        AABB entityBox = entity->GetHitbox();
        if (!playerBox.Intersects(entityBox)) continue;

        if (es.IsEnemy()) {
            HandleEnemyCollision(player, *entity, camera, gameState, uiManager, isStomp);
        } else if (es.IsPowerUp() || es.IsCoin()) {
            HandleItemCollision(player, *entity, camera, gameState, uiManager);
        }
    }
}

// ============================================================================
// HandleEnemyCollision
// Covers: star power instant-kill, stomp, shell kick, side damage.
// ============================================================================
void PlayerEntityHandler::HandleEnemyCollision(Player& player, Entity& entity,
                                               Camera& camera,
                                               GameStateManager& gameState,
                                               UIManager& uiManager, bool isStomp) {
    PlayerState& ps = player.GetState();
    EntityState& es = entity.GetState();

    // Helper: floating text at the entity's world centre.
    auto addScore = [&](int score) {
        float ptsdX = GameConfig::TopLeftToPTSDX(
            es.GetWorldX(), static_cast<float>(es.GetWidth()),
            camera.GetOffset());
        float ptsdY = GameConfig::TopLeftToPTSDY(
            es.GetWorldY(), static_cast<float>(es.GetHeight()));
        uiManager.AddFloatingText(ptsdX, ptsdY, "+" + std::to_string(score),
                                  60);
        gameState.AddScore(score);
    };

    auto* behavior = entity.GetBehavior();

    // --- Star power: instant kill ------------------------------------------
    if (ps.GetStarTimer() > 0) {
        // Bowser fire is invincible and immune to Star power - it still damages
        // Mario and disappears!
        if (behavior && behavior->IsEnemyProjectile() && behavior->IsImmuneToStarPower()) {
            ps.TakeDamage();
            behavior->OnPlayerCollision(es, player, false, gameState, uiManager, camera);
            if (es.IsActive()) es.Delete();
            return;
        }

        es.TriggerDeath(EnemyDeathCause::STAR_HIT);
        AudioManager::GetInstance().PlaySFX(SFXName::Kick);
        addScore(es.GetScoreWorth());
        return;
    }

    if (isStomp) {
        // --- Stomp-immune enemies (Bowser, Podoboo, etc.) ------------------
        if (behavior && behavior->IsImmuneToStomp()) {
            behavior->OnPlayerCollision(es, player, true, gameState, uiManager, camera);
            return;
        }
    }

    // --- Delegate stomp / side-collisions to the Behavior ---
    if (behavior) {
        bool handled = behavior->OnPlayerCollision(es, player, isStomp, gameState, uiManager, camera);
        if (handled && isStomp) {
            // NES consecutive-stomp score combo:
            m_StompCombo++;
            int base = es.GetScoreWorth();
            int score =
                (m_StompCombo >= 5) ? 1000 : base * (1 << (m_StompCombo - 1));
            addScore(score);

            // Bounce Mario upward (half normal jump height).
            ps.SetFallHeight(PhysicsEngine::GetJumpHeight(0) * 0.5);
            ps.SetGrounded(false);
        }
    }
}

// ============================================================================
// HandleItemCollision
// Covers: power-ups (mushroom, fire flower, star) and coins.
// ============================================================================
void PlayerEntityHandler::HandleItemCollision(Player& player, Entity& entity,
                                              Camera& camera,
                                              GameStateManager& gameState,
                                              UIManager& uiManager) {
    auto* behavior = entity.GetBehavior();
    if (behavior) {
        behavior->OnItemCollected(entity.GetState(), player, gameState, uiManager, camera);
    }
}

}  // namespace Mario
