/**
 * @file BlockContactResolver.hpp
 * @brief Static per-direction AABB block contact resolution helpers.
 *        Each static method mirrors one C# CheckCollisionsXxx function and
 *        snaps the player's position along one axis.
 *        Also provides BodyRect() which builds the full-width AABB matching
 *        C# GetRecPosition().
 * @inheritance None (static utility class — delete constructor)
 */
#ifndef MARIO_COLLISION_BLOCK_CONTACT_RESOLVER_HPP
#define MARIO_COLLISION_BLOCK_CONTACT_RESOLVER_HPP

#include "Mario/Collider.hpp"
#include "Mario/PlayerState.hpp"

namespace Mario {

/**
 * Provides static helpers that resolve player-AABB contact along each axis.
 * All methods use a TILE_SIZE-wide body rect (matching C# GetRecPosition()) and
 * apply the INTERSECT_STRICTNESS (0.75) threshold so shallow grazing contacts
 * are ignored — only a real penetration of ≥75 % of the tile triggers a snap.
 *
 * movingDown / movingUp are passed by non-const reference because they must
 * persist across multiple block iterations (set to false once resolved).
 * movingRight / movingLeft are per-block copies owned by the caller.
 */
class BlockContactResolver {
   public:
    BlockContactResolver() = delete;  // static-only — no instances

    /**
     * Build the full-body AABB.
     * Width = TILE_SIZE (C# GetRecPosition). Height = state.GetHeight().
     * Origin = (state.GetX(), state.GetY()).
     */
    static AABB BodyRect(const PlayerState& state);

    /**
     * Snap player feet to block top when falling.
     * C#: if (Bottom > b.Top && Bottom < b.Top + size*0.75 && movingDown)
     *       → SetY(b.Top − height); SetGrounded(true); SetVelY(0);
     */
    static void ResolveDown(PlayerState& state, const AABB& bb,
                            bool& movingDown);

    /**
     * Snap player head to block bottom when rising.
     * C#: if (Top < b.Bottom && Top > b.Bottom − size*0.75 && movingUp)
     *       → SetY(b.Bottom); SetVelY(0);
     * Note: block content triggering is handled separately in
     * StepCeilingTrigger.
     */
    static void ResolveUp(PlayerState& state, const AABB& bb, bool& movingUp);

    /**
     * Snap player right edge to block left when moving right.
     * C#: if (Right > b.Left && Right < b.Left + size*0.75 && movingRight)
     *       → SetX(b.Left − TILE_SIZE); SetVelX(0);
     */
    static void ResolveRight(PlayerState& state, const AABB& bb,
                             bool& movingRight);

    /**
     * Snap player left edge to block right when moving left.
     * C#: if (Left < b.Right && Left > b.Right − size*0.75 && movingLeft)
     *       → SetX(b.Right); SetVelX(0);
     */
    static void ResolveLeft(PlayerState& state, const AABB& bb,
                            bool& movingLeft);
};

}  // namespace Mario

#endif  // MARIO_COLLISION_BLOCK_CONTACT_RESOLVER_HPP
