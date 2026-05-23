/**
 * @file PlayerBlockHandler.hpp
 * @brief Handles the three-step Player-Block collision pipeline ported from
 *        C# Form1.cs onTick(): FallDetect → CeilingTrigger → BodyResolution.
 *        Replaces the monolithic lambda in the old CollisionManager with clear,
 *        named step methods and a ProcessSingleBlock helper.
 * @inheritance PlayerBlockHandler : ICollisionHandler
 */
#ifndef MARIO_COLLISION_PLAYER_BLOCK_HANDLER_HPP
#define MARIO_COLLISION_PLAYER_BLOCK_HANDLER_HPP

#include <vector>

#include "Mario/Collision/ICollisionHandler.hpp"
#include "Mario/Level.hpp"
#include "Mario/Player.hpp"

namespace Mario {

class Camera;
class GameStateManager;
class UIManager;

/**
 * Runs the full player-block collision pipeline once per game frame.
 *
 * Step 1 — FallDetect (C# marioIntersect):
 *   A 1-pixel-tall strip shifted down from the feet. If no solid block
 *   intersects it, SetGrounded(false).
 *
 * Step 2 — CeilingTrigger (C# GetHitBox narrow hitbox):
 *   Fires only while Mario is rising. Detects head entry into block bottom,
 *   snaps position, and triggers block contents (questions, bricks, coins).
 *
 * Step 3 — BodyResolution (C# GetRecPosition full-width AABB loop):
 *   For every nearby solid block that overlaps Mario's body rectangle:
 *     Airborne : DOWN → RIGHT → LEFT → DOWN (2nd pass) → UP → LEFT (2nd pass)
 *     Grounded : RIGHT or LEFT (or stationary push-out if VelX == 0)
 */
class PlayerBlockHandler : public ICollisionHandler {
   public:
    PlayerBlockHandler() = default;

    /**
     * Execute the 3-step pipeline for one frame.
     */
    void Resolve(Player& player, Level& level, Camera& camera,
                 GameStateManager& gameState, UIManager& uiManager,
                 std::vector<Level::SpawnPoint>* outSpawns = nullptr);

   private:
    // -- Pipeline steps -------------------------------------------------------

    /** Step 1: thin strip below feet; no solid block → SetGrounded(false). */
    void StepFallDetect(PlayerState& state, Level& level);

    /**
     * Step 2: narrow hitbox head-bump while rising.
     * Snaps Y position AND triggers block contents.
     * movingUp is set to false when a ceiling is hit.
     */
    void StepCeilingTrigger(PlayerState& state, Level& level, Camera& camera,
                            GameStateManager& gameState, UIManager& uiManager,
                            std::vector<Level::SpawnPoint>* outSpawns,
                            bool& movingUp);

    /**
     * Step 3: iterate nearby solid blocks and resolve each one.
     * movingDown and movingUp are shared state across blocks (once resolved,
     * they stay false for the rest of the frame — matching C# behaviour).
     * movingRight and movingLeft are reset per block inside ProcessSingleBlock.
     */
    void StepBodyResolution(PlayerState& state, Level& level, Camera& camera,
                            bool movingRight, bool movingLeft, bool& movingDown,
                            bool& movingUp);

    // -- Per-block helper -----------------------------------------------------

    /**
     * Apply the C# resolution order to one block AABB.
     *
     * movingDown / movingUp : by reference — modifications persist across the
     *                          block-iteration loop (shared frame state).
     * movingRight / movingLeft: by value — each block gets a fresh copy,
     *                          matching C# "localMovingRight/localMovingLeft".
     */
    void ProcessSingleBlock(PlayerState& state, const AABB& bb,
                            bool& movingDown, bool& movingUp, bool movingRight,
                            bool movingLeft);

    // -- Block interaction ----------------------------------------------------

    /**
     * Handle a head-bump on a block tile.
     * Dispatches to coin, item-spawn, or brick logic; plays audio; adds score;
     * appends spawn-points to outSpawns.
     */
    void TriggerBlockHit(Block& block, PlayerState& state, Camera& camera,
                         GameStateManager& gameState, UIManager& uiManager,
                         std::vector<Level::SpawnPoint>* outSpawns);
};

}  // namespace Mario

#endif  // MARIO_COLLISION_PLAYER_BLOCK_HANDLER_HPP
