/**
 * @file EnemyBehavior.cpp
 * @brief Implementation of enemy AI behavior (Goomba, Koopa Troopa).
 *        Ported from C# Entity.cs Update() and collision logic.
 * @inheritance IEntityBehavior <- EnemyBehavior
 */
#include "Mario/Behaviors/EnemyBehavior.hpp"

#include "Mario/Collider.hpp"
#include "Mario/EntityState.hpp"
#include "Mario/Level.hpp"
#include "Mario/PhysicsEngine.hpp"
#include "Mario/Player.hpp"
#include "Mario/PlayerState.hpp"
#include "Util/Logger.hpp"

namespace Mario {

EnemyBehavior::EnemyBehavior(EnemyType type) : m_Type(type) {}

void EnemyBehavior::Update(EntityState& state, const Level& level,
                           const Player& player, int gameTimer) {
    if (state.IsSquished() || state.IsDead()) {
        // Dead enemies don't move, just animate squish state
        if (!state.IsAnimated()) {
            state.Delete();  // Remove after animation done
        }
        return;
    }

    // Apply gravity
    double fallHeight = state.GetFallHeight();
    double velY = state.GetVelY();
    float yDelta =
        PhysicsEngine::ApplyGravity(fallHeight, velY, state.IsGrounded());
    state.SetFallHeight(fallHeight);
    state.SetVelY(velY);
    state.SetWorldY(state.GetWorldY() + yDelta);

    // Horizontal movement: VelX already includes direction (set in
    // EntityState::Init)
    if (!state.IsStatic()) {
        state.SetWorldX(state.GetWorldX() + state.GetVelX());
    }

    // Update animation frame
    m_DirectionChangeCounter++;
    if (state.IsAnimated() && m_DirectionChangeCounter % 10 == 0) {
        // Advance animation frame every 10 ticks
        state.AdvanceAnimationFrame();
    }
}

bool EnemyBehavior::OnPlayerCollision(EntityState& state, Player& player,
                                      bool isFromAbove) {
    if (state.IsSquished() || state.IsDead()) {
        return false;
    }

    if (isFromAbove) {
        // Player jumped on enemy
        if (state.IsSquishable()) {
            state.Squish();  // Mark as squashed
            return true;
        } else if (m_Type == EnemyType::KOOPA_TROOPA) {
            // Koopa: kick it (spawn moving shell)
            state.Squish();  // Mark as squashed/kicked
            return true;
        }
    } else {
        // Player hit from side - enemy hurts player
        // This is handled at App level with star state check
        return true;  // Collision was processed
    }

    return false;
}

std::unique_ptr<IEntityBehavior> EnemyBehavior::Clone() const {
    return std::make_unique<EnemyBehavior>(*this);
}

}  // namespace Mario
