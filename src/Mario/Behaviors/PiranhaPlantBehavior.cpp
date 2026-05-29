/**
 * @file PiranhaPlantBehavior.cpp
 * @brief Implementation of Piranha Plant (食人花) AI behavior.
 *        Emerges from pipe on a timer, retreats when Mario is nearby.
 *        Damages Mario on contact.
 * @inheritance IEntityBehavior <- PiranhaPlantBehavior
 */
#include "Mario/Behaviors/PiranhaPlantBehavior.hpp"

#include <cmath>

#include "Mario/Level/EntityState.hpp"
#include "Mario/Core/GameConfig.hpp"
#include "Mario/Player/Player.hpp"
#include "Mario/Player/PlayerState.hpp"
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

    float plantHeight = static_cast<float>(state.GetHeight());

    // Record the pipe opening Y on the very first frame
    if (!m_BaseYSet) {
        // Record the pipe mouth Y coordinate (spawn position).
        m_BaseY = state.GetY();
        
        // Shift X coordinate left by 1 tile so the 2-tile-wide plant centering is correct.
        // The spawner block (905) is at column C (the right side of the pipe),
        // so shifting X left by 1 tile places the plant's left edge at column C-1,
        // which perfectly covers both columns C-1 and C (the full pipe mouth).
        state.SetX(state.GetX() - static_cast<float>(GameConfig::TILE_SIZE));
        
        m_BaseYSet = true;
        // Start fully hidden inside the pipe (plantHeight below adjusted base)
        state.SetY(m_BaseY + plantHeight);
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
    // Since state.GetX() is shifted left by 1 tile (so it is the left edge of the pipe),
    // the pipe center is plantX + TILE_SIZE.
    // We suppress the plant if Mario is standing anywhere within the safe zone of the pipe.
    float pipeCenterX = plantX + static_cast<float>(GameConfig::TILE_SIZE);
    bool marioNearby = std::abs(marioX - pipeCenterX) < MARIO_SAFE_RADIUS;

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
            // Cancel emergence and retreat immediately if Mario gets close (prevent sneak attack)
            if (marioNearby) {
                m_Phase = Phase::RETREATING;
                m_PhaseTimer = 0;
                break;
            }
            float targetY = m_BaseY - plantHeight;
            float currentY = state.GetY();
            if (currentY > targetY) {
                state.SetY(currentY - EMERGE_SPEED);
            } else {
                state.SetY(targetY);
                m_Phase = Phase::VISIBLE;
                m_PhaseTimer = 0;
            }
            // Show only once the plant head has cleared the pipe mouth.
            // While Y > m_BaseY the plant is still inside the pipe — keep hidden.
            state.SetHidden(state.GetY() > m_BaseY);
            break;
        }

        case Phase::VISIBLE: {
            state.SetHidden(false);
            // Retreat immediately if Mario enters the pipe proximity radius.
            // This prevents the plant from damaging Mario during pipe entry,
            // matching NES behavior where the plant cannot hurt Mario while
            // he is standing directly over the pipe opening.
            if (marioNearby || m_PhaseTimer >= VISIBLE_FRAMES) {
                m_Phase = Phase::RETREATING;
                m_PhaseTimer = 0;
            }
            break;
        }

        case Phase::RETREATING: {
            float currentY = state.GetY();
            if (currentY < m_BaseY + plantHeight) {
                state.SetY(currentY + EMERGE_SPEED);
            } else {
                state.SetY(m_BaseY + plantHeight);  // reset to hidden pos
                m_Phase = Phase::HIDING;
                m_PhaseTimer = 0;
            }
            // Hide the plant once its base/head is inside the pipe mouth.
            state.SetHidden(state.GetY() >= m_BaseY);
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

AABB PiranhaPlantBehavior::GetHitbox(const EntityState& state) const {
    float w = static_cast<float>(state.GetWidth());
    float h = static_cast<float>(state.GetHeight());
    float hitW = std::min(w * 0.55f, static_cast<float>(GameConfig::TILE_SIZE) * 0.85f);
    float hitH = std::min(h * 0.70f, static_cast<float>(GameConfig::TILE_SIZE) * 1.30f);
    float hitX = state.GetX() + (w - hitW) * 0.5f;
    float hitY = state.GetY() + h * 0.05f;
    return AABB::FromPosSize(hitX, hitY, hitW, hitH);
}

std::unique_ptr<IEntityBehavior> PiranhaPlantBehavior::Clone() const {
    return std::make_unique<PiranhaPlantBehavior>(*this);
}

}  // namespace Mario
