/**
 * @file MovingPlatform.cpp
 * @brief Implementation of MovingPlatform oscillating solid block.
 * @inheritance Util::GameObject -> Block -> MovingPlatform
 */
#include "Mario/Level/MovingPlatform.hpp"
#include "Mario/Player/PlayerState.hpp"
#include "Mario/Core/GameConfig.hpp"

namespace Mario {

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
MovingPlatform::MovingPlatform(int blockID, int gridX, int gridY,
                               const BlockDef& def, Direction dir,
                               const std::string& levelName, float speed,
                               float rangeHalf)
    : Block(blockID, gridX, gridY, def, levelName),
      m_Direction(dir),
      m_Speed(speed),
      m_LiveWorldX(static_cast<float>(gridX) * GameConfig::TILE_SIZE),
      m_LiveWorldY(static_cast<float>(gridY) * GameConfig::TILE_SIZE),
      m_Velocity(speed) {
    if (m_Direction == Direction::VERTICAL) {
        m_MinBound = m_LiveWorldY - rangeHalf;
        m_MaxBound = m_LiveWorldY + rangeHalf;
    } else {
        m_MinBound = m_LiveWorldX - rangeHalf;
        m_MaxBound = m_LiveWorldX + rangeHalf;
    }
}

// ---------------------------------------------------------------------------
// StepMovement
// ---------------------------------------------------------------------------
void MovingPlatform::StepMovement() {
    m_LastDeltaX = 0.0f;
    m_LastDeltaY = 0.0f;

    if (m_Direction == Direction::VERTICAL) {
        m_LiveWorldY += m_Velocity;
        m_LastDeltaY = m_Velocity;

        if (m_LiveWorldY >= m_MaxBound) {
            m_LiveWorldY = m_MaxBound;
            m_Velocity = -m_Speed;
        } else if (m_LiveWorldY <= m_MinBound) {
            m_LiveWorldY = m_MinBound;
            m_Velocity = m_Speed;
        }
    } else {
        m_LiveWorldX += m_Velocity;
        m_LastDeltaX = m_Velocity;

        if (m_LiveWorldX >= m_MaxBound) {
            m_LiveWorldX = m_MaxBound;
            m_Velocity = -m_Speed;
        } else if (m_LiveWorldX <= m_MinBound) {
            m_LiveWorldX = m_MinBound;
            m_Velocity = m_Speed;
        }
    }
}

// ---------------------------------------------------------------------------
// Position overrides
// ---------------------------------------------------------------------------
float MovingPlatform::GetWorldX() const { return m_LiveWorldX; }
float MovingPlatform::GetWorldY() const { return m_LiveWorldY; }

AABB MovingPlatform::GetAABB() const {
    return AABB::FromPosSize(m_LiveWorldX, m_LiveWorldY,
                             static_cast<float>(GameConfig::TILE_SIZE),
                             static_cast<float>(GameConfig::TILE_SIZE));
}

// ---------------------------------------------------------------------------
// Update — sprite/animation via base class, then override screen position
// ---------------------------------------------------------------------------
void MovingPlatform::Update(float cameraOffset) {
    // Handles lazy sprite loading and question-block animation
    Block::Update(cameraOffset);

    // Override the screen translation with the live floating position
    float roundedOffset = std::round(cameraOffset);
    float sx = GameConfig::TopLeftToPTSDX(std::round(m_LiveWorldX),
                                          GameConfig::TILE_SIZE, roundedOffset);
    float sy = GameConfig::TopLeftToPTSDY(std::round(m_LiveWorldY),
                                          GameConfig::TILE_SIZE);
    m_Transform.translation = {sx, sy};
}

bool MovingPlatform::ShouldResolveVerticallyFirst(const AABB& playerBody) const {
    AABB bb = GetAABB();
    return playerBody.bottom < bb.top + bb.Height() * 0.5f;
}

void MovingPlatform::TryCarryPlayer(PlayerState& ps, const AABB& prevAABB,
                                    bool onStaticBlock) {
    if (ps.IsGrounded() && !onStaticBlock) {
        AABB playerBox = ps.GetHitbox();
        float gap = std::abs(playerBox.bottom - prevAABB.top);
        bool xOverlap = (playerBox.left < prevAABB.right &&
                         playerBox.right > prevAABB.left);
        if (gap < 2.0f && xOverlap) {
            ps.SetX(ps.GetX() + GetLastDeltaX());
            ps.SetY(ps.GetY() + GetLastDeltaY());
            ps.SetOnMovingPlatform(true);  // Suppress Jump sprite this frame
        }
    }
}

}  // namespace Mario
