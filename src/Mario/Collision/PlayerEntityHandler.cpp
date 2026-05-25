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

    for (auto& entity : entities) {
        EntityState& es = entity->GetState();
        if (!es.IsActive()) continue;
        if (es.IsHidden()) continue;  // PiranhaPlant inside pipe — not hittable

        AABB entityBox = es.GetHitbox();
        if (!playerBox.Intersects(entityBox)) continue;

        if (es.IsEnemy()) {
            HandleEnemyCollision(player, *entity, camera, gameState, uiManager);
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
                                               UIManager& uiManager) {
    PlayerState& ps = player.GetState();
    EntityState& es = entity.GetState();
    AABB playerBox = ps.GetHitbox();
    AABB entityBox = es.GetHitbox();

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

    // --- Star power: instant kill ------------------------------------------
    if (ps.GetStarTimer() > 0) {
        // Bowser fire is invincible and immune to Star power - it still damages
        // Mario and disappears!
        if (entity.GetDef().type == EntityType::FIRE && es.IsEnemy()) {
            ps.TakeDamage();
            auto* behavior = entity.GetBehavior();
            if (behavior) behavior->OnPlayerCollision(es, player, false);
            if (es.IsActive()) es.Delete();
            return;
        }

        es.TriggerDeath(EnemyDeathCause::STAR_HIT);
        AudioManager::GetInstance().PlaySFX(SFXName::Kick);
        addScore(es.GetScoreWorth());
        return;
    }

    // --- PiranhaPlant: always damages Mario, cannot be stomped -------------
    // Reference: NES Mario — Piranha Plant is lethal from any direction.
    if (entity.GetDef().type == EntityType::PIRANHA_PLANT) {
        ps.TakeDamage();
        return;
    }

    // --- Enemy projectiles: always damage Mario, consumed on contact -------
    // Bowser fire (FIRE + isEnemy) and thrown axes (AXE_PROJECTILE) cannot
    // be stomped; they are destroyed on contact just like in the NES game.
    bool isEnemyProjectile =
        (entity.GetDef().type == EntityType::FIRE && es.IsEnemy()) ||
        entity.GetDef().type == EntityType::AXE_PROJECTILE;
    if (isEnemyProjectile) {
        ps.TakeDamage();
        // Consume the projectile: let the behavior clean up first; if the
        // behavior does not delete it, fall back to a direct delete.
        auto* behavior = entity.GetBehavior();
        if (behavior) behavior->OnPlayerCollision(es, player, false);
        if (es.IsActive()) es.Delete();
        return;
    }

    // --- Stomp: player falling from above ----------------------------------
    // C# reference (Form1.cs): stomp fires when tempMovingDown is true.
    // tempMovingDown = !grounded && previousJumpHeight <= 0
    // C++ equivalent: !IsGrounded() && GetFallHeight() <= 0.0
    // The old overlapY < TILE_SIZE*0.5 check caused false negatives after
    // ~20 frames of free fall (VelY accumulates past 22.5 px/frame), and
    // ps.GetVelY() >= 0 could transiently fail at the rise→fall transition.
    bool isStomp = !ps.IsGrounded() && ps.GetFallHeight() <= 0.0;

    if (isStomp) {
        // --- Stomp-immune enemies (Bowser, Podoboo, etc.) ------------------
        // Query via virtual method — adding a new immune enemy only requires
        // overriding IsImmuneToStomp(); no changes needed here (OCP).
        auto* behavior = entity.GetBehavior();
        if (behavior && behavior->IsImmuneToStomp()) {
            // Mario cannot stomp them — contact damages Mario instead.
            behavior->OnPlayerCollision(es, player, true);
            return;
        }

        bool handledByBehavior = false;

        if (entity.GetDef().type == EntityType::KOOPA_SHELL ||
            es.IsInShellMode()) {
            // Stomp on shell: kick/propel it.
            float playerCX =
                playerBox.left + (playerBox.right - playerBox.left) * 0.5f;
            float shellCX =
                entityBox.left + (entityBox.right - entityBox.left) * 0.5f;
            float kickSpeed = (playerCX > shellCX) ? -GameConfig::SCALED_SPEED
                                                   : GameConfig::SCALED_SPEED;
            es.KickShell(kickSpeed);
            ps.SetInvTimer(5);
            handledByBehavior = true;
        } else if (entity.GetDef().type == EntityType::PARAKOOPA) {
            auto* behavior = entity.GetBehavior();
            if (behavior) {
                handledByBehavior =
                    behavior->OnPlayerCollision(es, player, true);
            }
        }

        if (!handledByBehavior) {
            if (es.IsSquishable()) {
                es.TriggerDeath(EnemyDeathCause::STOMP);
                AudioManager::GetInstance().PlaySFX(SFXName::Squish);
            } else if (es.IsKoopaSquash()) {
                es.TriggerDeath(EnemyDeathCause::STOMP);
                AudioManager::GetInstance().PlaySFX(SFXName::Squish);
            } else {
                AudioManager::GetInstance().PlaySFX(SFXName::Squish);
            }
        } else if (entity.GetDef().type == EntityType::PARAKOOPA) {
            AudioManager::GetInstance().PlaySFX(SFXName::Squish);
        }

        // NES consecutive-stomp score: ×1, ×2, ×4, ×8, then cap at 1000.
        m_StompCombo++;
        int base = es.GetScoreWorth();
        int score =
            (m_StompCombo >= 5) ? 1000 : base * (1 << (m_StompCombo - 1));
        addScore(score);

        // Bounce Mario upward (half normal jump height).
        ps.SetFallHeight(PhysicsEngine::GetJumpHeight(0) * 0.5);
        ps.SetGrounded(false);
        return;
    }

    // --- Shell side collision (moving shell hurts; stationary = kick) ------
    if (entity.GetDef().type == EntityType::KOOPA_SHELL || es.IsInShellMode()) {
        if (std::abs(es.GetVelX()) < 0.1f) {
            // Stationary shell: kick it.
            AABB eBox = entityBox;
            float playerCX =
                playerBox.left + (playerBox.right - playerBox.left) * 0.5f;
            float shellCX = eBox.left + (eBox.right - eBox.left) * 0.5f;
            float kickSpeed = GameConfig::SCALED_SPEED;
            if (playerCX > shellCX)
                kickSpeed = -kickSpeed;
            else
                kickSpeed = std::abs(kickSpeed);
            es.KickShell(kickSpeed);
            ps.SetInvTimer(5);
        } else {
            // Moving shell: damages player.
            ps.TakeDamage();
        }
        return;
    }

    // --- Regular side damage -----------------------------------------------
    if (!es.IsSquished()) {
        ps.TakeDamage();
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
    PlayerState& ps = player.GetState();
    EntityState& es = entity.GetState();

    float ptsdX = GameConfig::TopLeftToPTSDX(
        es.GetWorldX(), static_cast<float>(es.GetWidth()), camera.GetOffset());
    float ptsdY = GameConfig::TopLeftToPTSDY(
        es.GetWorldY(), static_cast<float>(es.GetHeight()));

    if (es.IsPowerUp()) {
        int puState = es.GetPowerUpState();
        int curState = ps.GetState();

        if (puState >= 1 && puState <= 3) {
            if (curState == 3 || curState == 4) {
                // Collection during star invincibility.
                if (curState == 4) {  // Big Star
                    if (puState == 2) {
                        ps.SetMemoryState(PowerState::FIRE);
                    }
                } else if (curState == 3) {  // Small Star
                    if (puState == 1 || puState == 2) {
                        ps.SetY(ps.GetY() - GameConfig::TILE_SIZE);
                        ps.SetPowerState(PowerState::BIG_STAR);
                        ps.SetMemoryState(puState == 1 ? PowerState::BIG
                                                       : PowerState::FIRE);
                    }
                }
            } else {
                // Regular power-up collection.
                if (puState > curState) {
                    if (curState == 0 && puState < 3) {
                        ps.SetY(ps.GetY() - GameConfig::TILE_SIZE);
                    }
                    if (puState == 1)
                        ps.PowerUp(PowerState::BIG);
                    else if (puState == 2)
                        ps.PowerUp(PowerState::FIRE);
                    else if (puState == 3)
                        ps.StartStar();
                }
            }
            AudioManager::GetInstance().PlaySFX(SFXName::Powerup);
        } else if (puState == 5) {
            gameState.AddLife();
            AudioManager::GetInstance().PlaySFX(SFXName::_1up);
            uiManager.AddFloatingText(ptsdX, ptsdY, "+1UP", 60);
        }

        int score = es.GetScoreWorth();
        gameState.AddScore(score);
        uiManager.AddFloatingText(ptsdX, ptsdY, "+" + std::to_string(score),
                                  60);
        es.Delete();

    } else if (es.IsCoin()) {
        gameState.AddCoin();
        int score = es.GetScoreWorth();
        gameState.AddScore(score);
        AudioManager::GetInstance().PlaySFX(SFXName::Coin);
        uiManager.AddFloatingText(ptsdX, ptsdY, "+" + std::to_string(score),
                                  60);
        es.Delete();
    }
}

}  // namespace Mario
