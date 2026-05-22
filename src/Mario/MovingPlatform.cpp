/**
 * @file MovingPlatform.cpp
 * @brief Implementation of MovingPlatform oscillating solid block.
 * @inheritance Util::GameObject -> Block -> MovingPlatform
 */
#include "Mario/MovingPlatform.hpp"

#include "Mario/GameConfig.hpp"

namespace Mario {

// ---------------------------------------------------------------------------
// File-local helper: convert live world coords to PTSD screen coords.
// Mirrors the GridToScreen formula in Block.cpp but uses float world coords.
// ---------------------------------------------------------------------------
static void WorldToScreen(float worldX, float worldY, float cameraOffset,
                          float& sx, float& sy) {
    float roundedOffset = std::round(cameraOffset);
    float leftScreenX = std::round(worldX) - roundedOffset - GameConfig::WINDOW_WIDTH / 2.0f;
    sx = leftScreenX + GameConfig::TILE_SIZE / 2.0f;

    float bottomScreenY = GameConfig::LEVEL_HEIGHT_PX / 2.0f -
                          (std::round(worldY) + GameConfig::TILE_SIZE) +
                          GameConfig::RENDER_Y_OFFSET;
    sy = bottomScreenY + GameConfig::TILE_SIZE / 2.0f;
}

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
    float sx, sy;
    WorldToScreen(m_LiveWorldX, m_LiveWorldY, cameraOffset, sx, sy);
    m_Transform.translation = {sx, sy};
}

}  // namespace Mario
