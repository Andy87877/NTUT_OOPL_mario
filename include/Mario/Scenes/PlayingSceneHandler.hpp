/**
 * @file PlayingSceneHandler.hpp
 * @brief Main gameplay scene handler (App::State::PLAYING).
 *        Coordinates input, physics, entity AI, collision, camera, and timing
 *        for the active gameplay loop.
 * @inheritance ISceneHandler <- PlayingSceneHandler
 */
#ifndef MARIO_PLAYING_SCENE_HANDLER_HPP
#define MARIO_PLAYING_SCENE_HANDLER_HPP

#include <vector>
#include "Mario/Scenes/ISceneHandler.hpp"

namespace Mario {

class Block;

/**
 * Handles the primary gameplay loop while App is in State::PLAYING.
 *
 * Per-frame responsibilities (in order):
 *  1.  ESC input -> pause (transition to ESC_MENU)
 *  2.  Player input (via InputHandler)
 *  3.  Player gravity & position update
 *  4.  Player-block collision resolution (CollisionManager)
 *  5.  Entity block-spawns (coin blocks, etc.)
 *  6.  Player state tick + fireball spawn
 *  7.  Entity AI (behavior strategies) + pending spawn requests
 *  8.  Entity-block collision + entity tick + view update
 *  9.  Player-entity collision & entity-entity collision
 *  10. Axe / flagpole / pipe trigger checks
 *  11. Camera follow + level block updates
 *  12. Brick-debris particle spawn
 *  13. Player view update
 *  14. Game timer countdown + hurry-up BGM switch
 *  15. Pit-fall check
 *  16. Death state transition
 *  17. Dead-entity cleanup
 */
class PlayingSceneHandler : public ISceneHandler {
   public:
    PlayingSceneHandler() = default;

    void Update(App& app) override;
    void OnRender(App& app) override;
    const char* GetName() const override { return "PlayingScene"; }

   private:
    /** Spawn a fireball when the player fires (power state = Fire). */
    void SpawnPlayerFireball(App& app) const;

    /** Spawn four brick-debris particles for any block that just broke. */
    void SpawnBrickDebris(App& app) const;

    // -------------------------------------------------------------------------
    // Trigger detectors — game-logic decisions owned by the PLAYING state.
    // Kept private: no other state needs these; moving them here removes the
    // last "game-logic" methods from App and completes the OOP architecture.
    // -------------------------------------------------------------------------

    /** Check if Mario has touched the flagpole goal block. */
    void CheckFlagpoleCollision(App& app) const;

    /** Check if Mario is entering a warp pipe (down or right direction). */
    void CheckPipeCollision(App& app) const;

    /** Check if Mario has touched the axe in 8-4 to start the ending. */
    void CheckAxeCollision(App& app) const;

    /** Delegate player-entity collision to CollisionManager. */
    void CheckPlayerEntityCollision(App& app) const;

    /** Delegate entity-entity collision to CollisionManager. */
    void CheckEntityEntityCollision(App& app) const;

    /** Remove inactive entities from the entity list and renderer. */
    void CleanupDeadEntities(App& app) const;
};

}  // namespace Mario

#endif  // MARIO_PLAYING_SCENE_HANDLER_HPP
