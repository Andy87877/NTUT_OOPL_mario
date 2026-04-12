/**
 * @file KoopaBehavior.cpp
 * @brief Implementation of Koopa Troopa and Shell behavior.
 *        Logic ported from C# Entity.cs Update() method.
 *        Shell spawning handled by App::CheckPlayerEntityCollision (C# Squish).
 * @inheritance IEntityBehavior <- KoopaBehavior
 */
#include "Mario/Behaviors/KoopaBehavior.hpp"

#include "Mario/Collider.hpp"
#include "Mario/EntityState.hpp"
#include "Mario/GameConfig.hpp"
#include "Mario/Level.hpp"
#include "Mario/PhysicsEngine.hpp"
#include "Mario/Player.hpp"
#include "Util/Logger.hpp"

namespace Mario {

KoopaBehavior::KoopaBehavior(KoopaType type) : m_Type(type) {}

void KoopaBehavior::Update(EntityState& state, const Level& level,
                           const Player& player, int gameTimer) {
    if (state.IsSquished() || state.IsDead()) {
        // Dead koopas don't move
        if (!state.IsAnimated()) {
            state.Delete();
        }
        return;
    }

    // Apply gravity (for both TROOPA and SHELL)
    double fallHeight = state.GetFallHeight();
    double velY = state.GetVelY();
    float yDelta =
        PhysicsEngine::ApplyGravity(fallHeight, velY, state.IsGrounded());
    state.SetFallHeight(fallHeight);
    state.SetVelY(velY);
    state.SetWorldY(state.GetWorldY() + yDelta);

    // Horizontal movement: velocity is set by constructor or App layer
    if (!state.IsStatic()) {
        state.SetWorldX(state.GetWorldX() + state.GetVelX());
    }

    if (m_Type == KoopaType::SHELL) {
        return;
    }

    // Update animation
    m_DirectionChangeCounter++;
    if (state.IsAnimated() && m_DirectionChangeCounter % 10 == 0) {
        state.AdvanceAnimationFrame();
    }
}

bool KoopaBehavior::OnPlayerCollision(EntityState& state, Player& player,
                                      bool isFromAbove) {
    // Shell spawning is handled by App::CheckPlayerEntityCollision (C# Squish)
    // This method just returns false to let App handle all collision logic
    return false;
}

std::unique_ptr<IEntityBehavior> KoopaBehavior::Clone() const {
    return std::make_unique<KoopaBehavior>(*this);
}

}  // namespace Mario
