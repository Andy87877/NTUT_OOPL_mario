/**
 * @file DefaultEntityBehavior.cpp
 * @brief Implementation of DefaultEntityBehavior passive entity behavior.
 * @inheritance IEntityBehavior <- DefaultEntityBehavior
 */
#include "Mario/Behaviors/DefaultEntityBehavior.hpp"

#include "Mario/Level/EntityState.hpp"
#include "Mario/Level/GameStateManager.hpp"
#include "Mario/Level/Level.hpp"
#include "Mario/Player/Player.hpp"
#include "Mario/Player/PlayerState.hpp"
#include "Mario/Services/AudioManager.hpp"
#include "Mario/Core/GameConfig.hpp"
#include "Mario/Core/PhysicsEngine.hpp"
#include "Mario/UI/UIManager.hpp"
#include "Mario/Core/Camera.hpp"
#include "Util/Logger.hpp"

namespace Mario {

void DefaultEntityBehavior::Update([[maybe_unused]] EntityState& state, [[maybe_unused]] const Level& level,
                                   [[maybe_unused]] const Player& player, [[maybe_unused]] int gameTimer) {
    // Passive entities don't move or update actively
    // Just let animation play if animated
    if (state.IsAnimated()) {
        // Animation frame update is handled by EntityState
        // This method does nothing for default behavior
    }
}

bool DefaultEntityBehavior::OnPlayerCollision(EntityState& state,
                                              [[maybe_unused]] Player& player,
                                              [[maybe_unused]] bool isFromAbove,
                                              [[maybe_unused]] GameStateManager& gameState,
                                              [[maybe_unused]] UIManager& uiManager,
                                              [[maybe_unused]] Camera& camera) {
    // For coins: add score, mark for removal
    if (state.IsCoin()) {
        state.Delete();
        // Score is handled by GameStateManager in App
        return true;  // Consumed
    }

    // For power-ups: apply power state to player
    if (state.IsPowerUp()) {
        [[maybe_unused]] int powerUpState = state.GetPowerUpState();
        // Power-up application logic handled in App collision manager
        state.Delete();  // Remove the item
        return true;     // Consumed
    }

    return false;  // No special collision
}

std::unique_ptr<IEntityBehavior> DefaultEntityBehavior::Clone() const {
    return std::make_unique<DefaultEntityBehavior>(*this);
}

// ============================================================================
// EnemyBehavior implementation
// ============================================================================
void EnemyBehavior::HandlePlayerCollision(EntityState& state, Player& player,
                                          bool isStomp, GameStateManager& gameState,
                                          UIManager& uiManager, Camera& camera,
                                          int& stompCombo) {
    if (state.IsDead()) return;

    PlayerState& ps = player.GetState();

    auto addScore = [&](int score) {
        float ptsdX = GameConfig::TopLeftToPTSDX(
            state.GetWorldX(), static_cast<float>(state.GetWidth()),
            camera.GetOffset());
        float ptsdY = GameConfig::TopLeftToPTSDY(
            state.GetWorldY(), static_cast<float>(state.GetHeight()));
        uiManager.AddFloatingText(ptsdX, ptsdY, "+" + std::to_string(score),
                                  60);
        gameState.AddScore(score);
    };

    // --- Star power: instant kill ------------------------------------------
    if (ps.GetStarTimer() > 0) {
        // Bowser fire is invincible and immune to Star power - it still damages
        // Mario and disappears!
        if (IsEnemyProjectile() && IsImmuneToStarPower()) {
            ps.TakeDamage();
            OnPlayerCollision(state, player, false, gameState, uiManager, camera);
            if (state.IsActive()) state.Delete();
            return;
        }

        state.TriggerDeath(EnemyDeathCause::STAR_HIT);
        AudioManager::GetInstance().PlaySFX(SFXName::Kick);
        addScore(state.GetScoreWorth());
        return;
    }

    if (isStomp) {
        // --- Stomp-immune enemies (Bowser, Podoboo, etc.) ------------------
        if (IsImmuneToStomp()) {
            OnPlayerCollision(state, player, true, gameState, uiManager, camera);
            return;
        }
    }

    // --- Delegate stomp / side-collisions to the Behavior ---
    bool handled = OnPlayerCollision(state, player, isStomp, gameState, uiManager, camera);
    if (handled && isStomp) {
        // NES consecutive-stomp score combo:
        stompCombo++;
        int base = state.GetScoreWorth();
        int score = (stompCombo >= 5) ? 1000 : base * (1 << (stompCombo - 1));
        addScore(score);

        // Bounce Mario upward (half normal jump height).
        ps.SetFallHeight(PhysicsEngine::GetJumpHeight(0) * 0.5);
        ps.SetGrounded(false);
    }
}

// ============================================================================
// ItemBehavior implementation
// ============================================================================
void ItemBehavior::HandlePlayerCollision(EntityState& state, Player& player,
                                         bool /*isStomp*/, GameStateManager& gameState,
                                         UIManager& uiManager, Camera& camera,
                                         int& stompCombo) {
    (void)stompCombo;
    OnItemCollected(state, player, gameState, uiManager, camera);
}

}  // namespace Mario

