/**
 * @file MovingPlatform.hpp
 * @brief Moving platform block that slides linearly between two bounds.
 *        1-2: VERTICAL (up/down).  8-4: HORIZONTAL (left/right).
 * @inheritance Util::GameObject -> Block -> MovingPlatform
 */
#ifndef MARIO_MOVING_PLATFORM_HPP
#define MARIO_MOVING_PLATFORM_HPP

#include "Mario/Block.hpp"

namespace Mario {

/**
 * A solid, animated platform that oscillates along one axis.
 *
 * Movement model:
 *   Each tick StepMovement() advances the live world position by m_Velocity.
 *   When the position reaches m_MinBound or m_MaxBound the velocity reverses.
 *
 * Player carry:
 *   After StepMovement() the caller can read GetLastDeltaX/Y() and apply the
 *   delta to the player if the player was standing on this platform.
 *
 * Rendering:
 *   Update(cameraOffset) calls Block::Update() for sprite loading/animation,
 *   then overrides m_Transform.translation with the live world position.
 */
class MovingPlatform : public Block {
   public:
    enum class Direction { VERTICAL, HORIZONTAL };

    /**
     * @param blockID    Block type ID (960 = vertical, 961 = horizontal)
     * @param gridX      Initial grid column
     * @param gridY      Initial grid row
     * @param def        Block definition loaded from IDList.csv
     * @param dir        VERTICAL or HORIZONTAL movement axis
     * @param levelName  Level name for level-specific sprite lookup
     * @param speed      Movement speed in world-pixels per tick (default 1.5)
     * @param rangeHalf  Half the total travel range in world-pixels (default 4
     * tiles)
     */
    MovingPlatform(int blockID, int gridX, int gridY, const BlockDef& def,
                   Direction dir, const std::string& levelName = "",
                   float speed = 1.5f, float rangeHalf = 4.0f * 45.0f);

    virtual ~MovingPlatform() = default;

    /**
     * Advance the platform one tick.
     * Moves by m_Velocity along the movement axis; reverses at bounds.
     * Records the frame delta in m_LastDeltaX / m_LastDeltaY.
     */
    void StepMovement();

    // Delta applied during last StepMovement() — used by carry logic
    float GetLastDeltaX() const { return m_LastDeltaX; }
    float GetLastDeltaY() const { return m_LastDeltaY; }

    Direction GetDirection() const { return m_Direction; }

    // Overrides returning the live (moving) world position
    float GetWorldX() const override;
    float GetWorldY() const override;
    AABB GetAABB() const override;

    /**
     * Update visual position to match live world coords.
     * Calls Block::Update() for sprite/animation, then overrides screen pos.
     * @param cameraOffset Current camera horizontal scroll offset
     */
    void Update(float cameraOffset) override;

   private:
    Direction m_Direction;
    float m_Speed;

    float m_LiveWorldX;  // Live world X (updated each StepMovement)
    float m_LiveWorldY;  // Live world Y (updated each StepMovement)
    float m_MinBound;    // Lower bound on the movement axis
    float m_MaxBound;    // Upper bound on the movement axis
    float m_Velocity;    // Current signed velocity (+/- speed)

    float m_LastDeltaX = 0.0f;  // X delta applied last tick
    float m_LastDeltaY = 0.0f;  // Y delta applied last tick
};

}  // namespace Mario

#endif  // MARIO_MOVING_PLATFORM_HPP
