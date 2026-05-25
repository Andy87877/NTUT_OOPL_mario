/**
 * @file BlockContactResolver.cpp
 * @brief Static per-direction AABB block contact resolution.
 *        Each method directly mirrors one C# CheckCollisionsXxx function from
 *        Form1.cs with the same strictness condition (TILE_SIZE * 0.75).
 * @inheritance None (static utility class)
 */
#include "Mario/Collision/BlockContactResolver.hpp"

#include "Mario/Core/GameConfig.hpp"
#include "Mario/Level/Level.hpp"
#include "Mario/Level/Block.hpp"

namespace Mario {

// Build the full-body AABB (C# GetRecPosition equivalent).
// Width is always TILE_SIZE regardless of hitbox ratio.
AABB BlockContactResolver::BodyRect(const PlayerState& state) {
    return AABB::FromPosSize(state.GetX(), state.GetY(),
                             static_cast<float>(GameConfig::TILE_SIZE),
                             static_cast<float>(state.GetHeight()));
}

// C#: if (Bottom > b.Top && Bottom < b.Top + size*0.75 && movingDown)
void BlockContactResolver::ResolveDown(PlayerState& state, const AABB& bb,
                                       bool& movingDown) {
    AABB body = BodyRect(state);
    // Velocity-adaptive threshold: grows with vertical speed to prevent clipping through the floor when falling fast
    const float threshold = std::max(static_cast<float>(GameConfig::TILE_SIZE) * GameConfig::INTERSECT_STRICTNESS,
                                     static_cast<float>(std::abs(state.GetVelY()) + 2.0f));
    if (body.bottom > bb.top && body.bottom < bb.top + threshold &&
        movingDown) {
        state.SetY(bb.top - static_cast<float>(state.GetHeight()));
        state.SetGrounded(true);
        state.SetVelY(0.0);
        state.SetFallHeight(0.0);
        movingDown = false;
    }
}

// C#: if (Top < b.Bottom && Top > b.Bottom - size*0.75 && movingUp)
void BlockContactResolver::ResolveUp(PlayerState& state, const AABB& bb,
                                     bool& movingUp) {
    AABB body = BodyRect(state);
    // Velocity-adaptive threshold: grows with vertical speed to prevent passing through block ceilings when jumping fast
    const float threshold = std::max(static_cast<float>(GameConfig::TILE_SIZE) * GameConfig::INTERSECT_STRICTNESS,
                                     static_cast<float>(std::abs(state.GetVelY()) + 2.0f));
    if (body.top < bb.bottom && body.top > bb.bottom - threshold && movingUp) {
        state.SetY(bb.bottom);
        state.SetFallHeight(0.0);
        state.SetVelY(0.0);
        movingUp = false;
    }
}

// C#: if (Right > b.Left && Right < b.Left + size*0.75 && movingRight)
void BlockContactResolver::ResolveRight(PlayerState& state, const AABB& bb,
                                        bool& movingRight) {
    AABB body = BodyRect(state);
    // Velocity-adaptive threshold: grows with horizontal speed to prevent clipping into walls when moving fast
    const float threshold = std::max(static_cast<float>(GameConfig::TILE_SIZE) * GameConfig::INTERSECT_STRICTNESS,
                                     static_cast<float>(std::abs(state.GetVelX()) + 2.0f));
    if (body.right > bb.left && body.right < bb.left + threshold &&
        movingRight) {
        state.SetX(bb.left - static_cast<float>(GameConfig::TILE_SIZE));
        state.SetVelX(0.0f);
        movingRight = false;
    }
}

// C#: if (Left < b.Right && Left > b.Right - size*0.75 && movingLeft)
void BlockContactResolver::ResolveLeft(PlayerState& state, const AABB& bb,
                                       bool& movingLeft) {
    AABB body = BodyRect(state);
    // Velocity-adaptive threshold: grows with horizontal speed to prevent clipping into walls when moving fast
    const float threshold = std::max(static_cast<float>(GameConfig::TILE_SIZE) * GameConfig::INTERSECT_STRICTNESS,
                                     static_cast<float>(std::abs(state.GetVelX()) + 2.0f));
    if (body.left < bb.right && body.left > bb.right - threshold &&
        movingLeft) {
        state.SetX(bb.right);
        state.SetVelX(0.0f);
        movingLeft = false;
    }
}

bool BlockContactResolver::IsPlayerOnStaticBlock(const PlayerState& state, Level& level) {
    AABB body = BodyRect(state);
    AABB fallDetect = {body.left, body.top + 1.0f, body.right, body.bottom + 1.0f};

    int tileX = static_cast<int>(body.left) / GameConfig::TILE_SIZE;
    int tileY = static_cast<int>(body.top) / GameConfig::TILE_SIZE;

    for (int gy = tileY; gy <= tileY + 3; gy++) {
        for (int gx = tileX - 1; gx <= tileX + 2; gx++) {
            Block* blk = level.GetBlockAt(gx, gy);
            if (blk && blk->IsSolid() && blk->GetAABB().Intersects(fallDetect)) {
                // Check if this block is a moving platform
                bool isMovingPlat = false;
                for (auto* otherPlat : level.GetMovingPlatforms()) {
                    if (blk == otherPlat) {
                        isMovingPlat = true;
                        break;
                    }
                }
                if (!isMovingPlat) {
                    return true;
                }
            }
        }
    }
    return false;
}

}  // namespace Mario
