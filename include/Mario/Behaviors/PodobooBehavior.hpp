/**
 * @file PodobooBehavior.hpp
 * @brief Behavior for the Podoboo (Lava Bubble) enemy in 8-4.
 *        Oscillates vertically out of lava pits. Ignores terrain.
 *        Cannot be destroyed — immortal to all damage.
 * @inheritance IEntityBehavior <- PodobooBehavior
 */
#ifndef MARIO_PODOBOO_BEHAVIOR_HPP
#define MARIO_PODOBOO_BEHAVIOR_HPP

#include "Mario/Behaviors/IEntityBehavior.hpp"

namespace Mario {

/**
 * Implements Podoboo (Lava Bubble) behavior:
 *   - Waits hidden inside lava for WAIT_FRAMES
 *   - Launches upward with a parabolic arc (JUMP_HIGH_VELOCITY)
 *   - Falls back into lava, repeating the cycle
 *   - Immune to all damage (fireball, stomp)
 *   - Damages Mario on contact regardless of approach direction
 *
 * Terrain is naturally ignored because entity physics bypass block
 * collision in this codebase (only the player collides with blocks).
 *
 * Staggering: each instance offsets its initial wait timer by its
 * X-position modulo WAIT_FRAMES so multiple Podoboos don't jump together.
 */
class PodobooBehavior : public IEntityBehavior {
   public:
    enum class Phase {
        WAITING,  // Hidden at lava base, counting timer
        JUMPING,  // Visible, physics-driven parabolic arc
    };

    PodobooBehavior();
    virtual ~PodobooBehavior() = default;

    /**
     * Drive the jump-oscillate state machine.
     * - WAITING: hold at m_BaseY, count timer, then launch
     * - JUMPING: let physics run; reset when entity returns to base
     */
    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;

    /**
     * Always damages Mario — Podoboo is not stompable.
     * Returns false so Podoboo is never deleted on contact.
     */
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;

    /**
     * Podoboo is immortal — absorbs fireballs without being destroyed.
     * Returns true so the caller skips deletion of this entity.
     */
    bool OnFireballHit(EntityState& state) override;

    /**
     * Podoboo cannot be stomped — contact always damages Mario.
     * Star power still defeats it via the handler's star-kill path.
     */
    bool IsImmuneToStomp() const override { return true; }
 
    /** Podoboos ignore solid block terrain snapping. */
    bool IgnoresBlocks() const override { return true; }

    std::unique_ptr<IEntityBehavior> Clone() const override;

    const char* GetName() const override { return "PodobooBehavior"; }

   private:
    Phase m_Phase = Phase::WAITING;
    int m_WaitTimer = 0;
    float m_BaseY = 0.0f;
    bool m_BaseYSet = false;
    int m_AnimTimer = 0;

    // Frames to wait between jumps (2 seconds at 60 fps)
    static constexpr int WAIT_FRAMES = 120;
    // Jump height: uses JUMP_HIGH_VELOCITY from PhysicsEngine
    static constexpr double JUMP_HEIGHT = 19.62;
};

}  // namespace Mario

#endif  // MARIO_PODOBOO_BEHAVIOR_HPP
