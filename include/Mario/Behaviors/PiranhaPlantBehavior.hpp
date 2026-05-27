/**
 * @file PiranhaPlantBehavior.hpp
 * @brief Behavior for the Piranha Plant enemy (食人花).
 *        Periodically emerges from a pipe, waits, then retreats.
 *        Stays hidden when Mario is within one tile width of the pipe opening.
 *        Damages Mario on contact.
 * @inheritance IEntityBehavior <- PiranhaPlantBehavior
 */
#ifndef MARIO_PIRANHA_PLANT_BEHAVIOR_HPP
#define MARIO_PIRANHA_PLANT_BEHAVIOR_HPP

#include "Mario/Behaviors/IEntityBehavior.hpp"

namespace Mario {

/**
 * Implements the Piranha Plant (食人花) enemy AI.
 *
 * State machine:
 *   HIDING    -> EMERGING  (after HIDE_FRAMES, only if Mario is not nearby)
 *   EMERGING  -> VISIBLE   (once fully extended)
 *   VISIBLE   -> RETREATING (after VISIBLE_FRAMES)
 *   RETREATING -> HIDING   (once back inside pipe)
 *
 * The plant is positioned at the top of its pipe tile (spawn Y = pipe top).
 * It moves upward by EMERGE_HEIGHT pixels when fully extended.
 *
 * OOP Design:
 *   - Inherits IEntityBehavior (Strategy Pattern)
 *   - Stateless from App's perspective; all AI state lives here
 */
class PiranhaPlantBehavior : public IEntityBehavior {
   public:
    /** Phase of the Piranha Plant's movement cycle. */
    enum class Phase {
        HIDING,      // Fully inside pipe, invisible to player
        EMERGING,    // Moving upward out of pipe
        VISIBLE,     // At maximum height, briefly paused
        RETREATING,  // Moving back down into pipe
    };

    PiranhaPlantBehavior();
    virtual ~PiranhaPlantBehavior() = default;

    /**
     * Per-frame AI update.
     * - Records base Y on first call.
     * - If Mario is within MARIO_SAFE_RADIUS horizontally and plant is HIDING,
     *   resets the hide timer so the plant never emerges while Mario is nearby.
     * - Advances the phase state machine.
     */
    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;

    /**
     * On contact with Mario: always deals damage (no stomp immunity).
     * Returns true (entity stays alive; it retreats instead of dying).
     */
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;

    /** Piranha Plants are immune to fireballs in this implementation. */
    bool OnFireballHit(EntityState& /*state*/) override { return false; }

    /** Piranha Plants cannot be stomped. */
    bool IsImmuneToStomp() const override { return true; }

    /** Deep copy for factory use. */
    std::unique_ptr<IEntityBehavior> Clone() const override;

    AABB GetHitbox(const EntityState& state) const override;

    const char* GetName() const override { return "PiranhaPlantBehavior"; }

    Phase GetPhase() const { return m_Phase; }

   private:
    Phase m_Phase = Phase::HIDING;
    int m_PhaseTimer = 0;     // Frames spent in current phase
    float m_BaseY = 0.0f;     // Y of the pipe opening (set on first Update)
    bool m_BaseYSet = false;  // Guard so m_BaseY is only recorded once

    static constexpr int HIDE_FRAMES = 90;             // 1.5 s hidden
    static constexpr int VISIBLE_FRAMES = 45;          // 0.75 s at full height
    static constexpr float EMERGE_SPEED = 2.0f;        // px/frame up or down
    static constexpr float HIDE_HEIGHT = 90.0f;        // Fully below pipe mouth
    static constexpr float EXTEND_HEIGHT = 64.0f;      // Emerging 1.4 tiles, leaving 26px inside pipe mouth
    static constexpr float MARIO_SAFE_RADIUS = 45.0f;  // 1 tile width
};

}  // namespace Mario

#endif  // MARIO_PIRANHA_PLANT_BEHAVIOR_HPP
