/**
 * @file Camera.cpp
 * @brief Implementation of Camera viewport scrolling.
 * @inheritance None
 */
#include "Mario/Core/Camera.hpp"

#include <algorithm>
#include "Mario/Level/LevelConfig.hpp"

namespace Mario {

void Camera::Update(float playerWorldX, float levelWidthPixels, const std::string& levelName, bool isLevelCompleteActive) {
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

    // Boss Room Camera Lock:
    // Lock the camera coordinate once it reaches the configured boss lock offset.
    // Allow the camera to scroll right during the ending cutscene (Axe Sequence) so Mario can reach the Princess.
    if (LevelConfig::HasCameraBossLock(levelName) && !isLevelCompleteActive) {
        float lockOffset = LevelConfig::GetCameraBossLockOffset(levelName);
        if (m_Offset >= lockOffset) {
            m_Offset = lockOffset;
        }
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
