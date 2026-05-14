/**
 * @file PodobooBehavior.cpp
 * @brief Implementation of Podoboo (Lava Bubble) AI behavior.
 *        Jumps out of lava on a timer, damages Mario on contact,
 *        and is completely immune to all damage sources.
 * @inheritance IEntityBehavior <- PodobooBehavior
 */
#include "Mario/Behaviors/PodobooBehavior.hpp"

#include "Mario/EntityState.hpp"
#include "Mario/Player.hpp"
#include "Mario/PlayerState.hpp"

namespace Mario {

PodobooBehavior::PodobooBehavior()
    : m_Phase(Phase::WAITING),
      m_WaitTimer(0),
      m_BaseY(0.0f),
      m_BaseYSet(false),
      m_AnimTimer(0) {}

void PodobooBehavior::Update(EntityState& state,
                             [[maybe_unused]] const Level& level,
                             [[maybe_unused]] const Player& player,
                             [[maybe_unused]] int gameTimer) {
    if (state.IsDead()) return;

    // Record spawn Y on first frame and stagger initial timer
    if (!m_BaseYSet) {
        m_BaseY = state.GetY();
        m_BaseYSet = true;
        // Offset initial wait so multiple Podoboos don't fire simultaneously
        m_WaitTimer = static_cast<int>(state.GetX()) % WAIT_FRAMES;
        state.SetGrounded(true);
        state.SetHidden(true);
    }

    // Podoboo never moves horizontally
    state.SetVelX(0.0f);

    switch (m_Phase) {
        case Phase::WAITING: {
            // Stay hidden and motionless at the lava surface
            state.SetHidden(true);
            state.SetGrounded(true);
            state.SetY(m_BaseY);

            m_WaitTimer++;
            if (m_WaitTimer >= WAIT_FRAMES) {
                // Launch: physics engine handles the parabolic arc
                m_WaitTimer = 0;
                m_AnimTimer = 0;
                state.SetHidden(false);
                state.SetGrounded(false);
                state.SetFallHeight(JUMP_HEIGHT);
                m_Phase = Phase::JUMPING;
            }
            break;
        }

        case Phase::JUMPING: {
            state.SetHidden(false);

            // Animate while airborne (toggle every 6 frames)
            m_AnimTimer++;
            if (m_AnimTimer % 6 == 0) {
                state.AdvanceAnimationFrame();
            }

            // Detect return to lava: entity has fallen back to or below base
            // m_VelY > 0 means the entity is descending
            if (state.GetY() >= m_BaseY && state.GetVelY() > 0.0) {
                state.SetY(m_BaseY);
                state.SetGrounded(true);
                state.SetHidden(true);
                m_Phase = Phase::WAITING;
                m_WaitTimer = 0;
            }
            break;
        }
    }
}

bool PodobooBehavior::OnPlayerCollision(EntityState& state, Player& player,
                                        [[maybe_unused]] bool isFromAbove) {
    // Podoboo always damages Mario — cannot be stomped
    if (state.IsHidden()) return false;

    PlayerState& ps = player.GetState();
    if (!ps.IsInvincible()) {
        ps.TakeDamage();
    }
    // Return false: Podoboo is never removed on player contact
    return false;
}

bool PodobooBehavior::OnFireballHit([[maybe_unused]] EntityState& state) {
    // Podoboo is immortal — fireballs are absorbed without destroying it
    return true;  // true = caller should NOT delete this entity
}

std::unique_ptr<IEntityBehavior> PodobooBehavior::Clone() const {
    return std::make_unique<PodobooBehavior>(*this);
}

}  // namespace Mario
