/**
 * @file Collider.hpp
 * @brief AABB (Axis-Aligned Bounding Box) for collision detection.
 *        Provides rectangle intersection and containment checks.
 * @inheritance None (data structure)
 */
#ifndef MARIO_COLLIDER_HPP
#define MARIO_COLLIDER_HPP

namespace Mario {

/**
 * Axis-Aligned Bounding Box for all collision checks.
 * Uses float for sub-pixel precision.
 */
struct AABB {
    float left   = 0.0f;
    float top    = 0.0f;
    float right  = 0.0f;
    float bottom = 0.0f;

    /**
     * Create an AABB from position and size.
     */
    static AABB FromPosSize(float x, float y, float w, float h) {
        return { x, y, x + w, y + h };
    }

    /**
     * Check if two AABBs overlap.
     */
    bool Intersects(const AABB& other) const {
        return left < other.right  && right  > other.left &&
               top  < other.bottom && bottom > other.top;
    }

    /**
     * Check if a point is inside this AABB.
     */
    bool Contains(float x, float y) const {
        return x >= left && x <= right && y >= top && y <= bottom;
    }

    /**
     * Get the width of this AABB.
     */
    float Width() const { return right - left; }

    /**
     * Get the height of this AABB.
     */
    float Height() const { return bottom - top; }

    /**
     * Get the center X of this AABB.
     */
    float CenterX() const { return (left + right) / 2.0f; }

    /**
     * Get the center Y of this AABB.
     */
    float CenterY() const { return (top + bottom) / 2.0f; }
};

} // namespace Mario

#endif // MARIO_COLLIDER_HPP
