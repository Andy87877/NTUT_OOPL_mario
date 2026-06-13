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
        if (es.IsDead()) continue;    // Skip dead/dying entities playing death animations

        AABB entityBox = entity->GetHitbox();
        if (!playerBox.Intersects(entityBox)) continue;

        auto* behavior = entity->GetBehavior();
        if (behavior) {
            behavior->HandlePlayerCollision(es, player, isStomp, gameState, uiManager, camera, m_StompCombo);
        }
    }
}

}  // namespace Mario

