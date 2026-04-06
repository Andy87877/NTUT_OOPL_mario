/**
 * @file Camera.cpp
 * @brief Implementation of Camera viewport scrolling.
 * @inheritance None
 */
#include "Mario/Camera.hpp"

#include <algorithm>

namespace Mario {

void Camera::Update(float playerWorldX, float levelWidthPixels) {
    m_PreviousOffset = m_Offset;

    // Camera scrolls when player passes the center of the viewport
    float scrollThreshold = m_Offset + GameConfig::WINDOW_WIDTH / 2.0f;

    if (playerWorldX > scrollThreshold) {
        m_Offset = playerWorldX - GameConfig::WINDOW_WIDTH / 2.0f;
    }

    // Clamp: camera cannot go left of origin
    m_Offset = std::max(0.0f, m_Offset);

    // Clamp: camera cannot go past the end of the level
    float maxOffset = levelWidthPixels - GameConfig::WINDOW_WIDTH;
    if (maxOffset > 0.0f) {
        m_Offset = std::min(m_Offset, maxOffset);
    }
}

void Camera::Reset() {
    m_Offset = 0.0f;
    m_PreviousOffset = 0.0f;
}

bool Camera::IsVisible(float worldX, float width) const {
    float screenX = worldX - m_Offset;
    return (screenX + width > -GameConfig::TILE_SIZE * 3) &&
           (screenX < GameConfig::WINDOW_WIDTH + GameConfig::TILE_SIZE * 3);
}

} // namespace Mario
