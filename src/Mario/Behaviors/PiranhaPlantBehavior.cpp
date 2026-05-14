/**
 * @file PiranhaPlantBehavior.cpp
 * @brief Implementation of Piranha Plant (食人花) AI behavior.
 *        Emerges from pipe on a timer, retreats when Mario is nearby.
 *        Damages Mario on contact.
 * @inheritance IEntityBehavior <- PiranhaPlantBehavior
 */
#include "Mario/Behaviors/PiranhaPlantBehavior.hpp"

#include <cmath>

#include "Mario/EntityState.hpp"
#include "Mario/GameConfig.hpp"
#include "Mario/Player.hpp"
#include "Mario/PlayerState.hpp"
#include "Util/Logger.hpp"

namespace Mario {

PiranhaPlantBehavior::PiranhaPlantBehavior()
    : m_Phase(Phase::HIDING),
      m_PhaseTimer(0),
      m_BaseY(0.0f),
      m_BaseYSet(false) {}

void PiranhaPlantBehavior::Update(EntityState& state,
                                  [[maybe_unused]] const Level& level,
                                  const Player& player,
                                  [[maybe_unused]] int gameTimer) {
    if (state.IsDead()) return;

    // Record the pipe opening Y on the very first frame
    if (!m_BaseYSet) {
        m_BaseY = state.GetY();
        m_BaseYSet = true;
        // Record pipe mouth Y, then push the plant 2 tiles down to hide it.
        // No X shift needed: the plant is now 2 tiles wide and spawns at the
        // left edge of the 2-tile pipe — entity left = pipe left exactly.
        // Start fully hidden inside the pipe (2 tiles below mouth)
        state.SetY(m_BaseY + EMERGE_HEIGHT);
        state.SetHidden(true);
    }

    // Advance animation every 15 frames while visible
    m_PhaseTimer++;
    if ((m_Phase == Phase::EMERGING || m_Phase == Phase::VISIBLE) &&
        (m_PhaseTimer % 15 == 0)) {
        state.AdvanceAnimationFrame();
    }

    // --- Is Mario close enough to suppress emergence? ---
    float marioX = player.GetWorldX();
    float plantX = state.GetX();
    bool marioNearby = std::abs(marioX - plantX) < MARIO_SAFE_RADIUS;

    switch (m_Phase) {
        case Phase::HIDING: {
            state.SetHidden(true);
            // Reset timer while Mario stays close
            if (marioNearby) {
                m_PhaseTimer = 0;
                break;
            }
            if (m_PhaseTimer >= HIDE_FRAMES) {
                m_Phase = Phase::EMERGING;
                m_PhaseTimer = 0;
            }
            break;
        }

        case Phase::EMERGING: {
            float targetY = m_BaseY - EMERGE_HEIGHT;
            float currentY = state.GetY();
            if (currentY > targetY) {
                state.SetY(currentY - EMERGE_SPEED);
            } else {
                state.SetY(targetY);
                m_Phase = Phase::VISIBLE;
                m_PhaseTimer = 0;
            }
            // Show only once the plant head has cleared the pipe mouth.
            // While Y > m_BaseY the plant is still inside the pipe — keep
            // hidden.
            state.SetHidden(state.GetY() > m_BaseY);
            break;
        }

        case Phase::VISIBLE: {
            state.SetHidden(false);
            if (m_PhaseTimer >= VISIBLE_FRAMES) {
                m_Phase = Phase::RETREATING;
                m_PhaseTimer = 0;
            }
            break;
        }

        case Phase::RETREATING: {
            float currentY = state.GetY();
            if (currentY < m_BaseY) {
                state.SetY(currentY + EMERGE_SPEED);
            } else {
                state.SetY(m_BaseY + EMERGE_HEIGHT);  // reset to hidden pos
                m_Phase = Phase::HIDING;
                m_PhaseTimer = 0;
            }
            // Hide the plant once its base is at the pipe mouth.
            // Threshold: 1 tile above mouth ≈ when entity bottom enters pipe.
            state.SetHidden(state.GetY() >=
                            m_BaseY -
                                static_cast<float>(GameConfig::TILE_SIZE));
            break;
        }
    }

    // Piranha Plants are static: no horizontal velocity, no gravity
    state.SetVelX(0.0f);
    state.SetGrounded(true);  // Prevent gravity from pulling it down
}

bool PiranhaPlantBehavior::OnPlayerCollision(
    EntityState& state, Player& player, [[maybe_unused]] bool isFromAbove) {
    // Piranha Plant is not stompable — always damages Mario
    if (m_Phase == Phase::HIDING) return false;  // Can't be hit while hidden

    auto& ps = player.GetState();
    if (!ps.IsInvincible()) {
        ps.TakeDamage();
    }
    return false;  // Plant stays alive
}

std::unique_ptr<IEntityBehavior> PiranhaPlantBehavior::Clone() const {
    return std::make_unique<PiranhaPlantBehavior>(*this);
}

}  // namespace Mario
