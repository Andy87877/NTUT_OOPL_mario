/**
 * @file Camera.hpp
 * @brief Camera system that follows the player and manages viewport scrolling.
 *        Converts world coordinates to screen coordinates.
 * @inheritance None (standalone manager)
 */
#ifndef MARIO_CAMERA_HPP
#define MARIO_CAMERA_HPP

#include "Mario/GameConfig.hpp"

namespace Mario {

/**
 * Manages the viewport offset for side-scrolling.
 * The camera only scrolls right (classic Mario behavior).
 */
class Camera {
public:
    Camera() = default;

    /**
     * Update camera position based on player's world X.
     * Camera scrolls when player passes center of viewport.
     * @param playerWorldX Player's world X position
     * @param levelWidthPixels Total level width in pixels
     */
    void Update(float playerWorldX, float levelWidthPixels);

    /**
     * Reset camera to initial position.
     */
    void Reset();

    /**
     * Get current camera offset (how far scrolled right).
     */
    float GetOffset() const { return m_Offset; }

    /**
     * Convert world X coordinate to screen X.
     */
    float WorldToScreenX(float worldX) const { return worldX - m_Offset; }

    /**
     * Convert world Y coordinate to screen Y (no vertical scrolling).
     */
    float WorldToScreenY(float worldY) const { return worldY; }

    /**
     * Check if a world-space rectangle is visible on screen.
     */
    bool IsVisible(float worldX, float width) const;

    /**
     * Get the visible world-space left/right bounds for culling.
     */
    float GetVisibleLeft() const { return m_Offset - GameConfig::TILE_SIZE * 3; }
    float GetVisibleRight() const {
        return m_Offset + GameConfig::WINDOW_WIDTH + GameConfig::TILE_SIZE * 3;
    }

private:
    float m_Offset = 0.0f;       // Current scroll offset
    float m_PreviousOffset = 0.0f;
};

} // namespace Mario

#endif // MARIO_CAMERA_HPP
