/**
 * @file PlayerState.hpp
 * @brief Model layer for the Player in MVC architecture.
 *        Manages Mario's physics state, power-up state, and animation keys.
 *        Does NOT handle rendering ??that is the Player (View) class's job.
 * @inheritance None (pure Model)
 */
#ifndef MARIO_PLAYER_STATE_HPP
#define MARIO_PLAYER_STATE_HPP

#include <memory>
#include <string>

#include "Mario/Core/Collider.hpp"
#include "Mario/Core/GameConfig.hpp"
#include "Mario/Player/PlayerDeathAnimation.hpp"

namespace Mario {

class IPlayerForm;

/**
 * Mario's power-up states, matching C# reference:
 *   0=Small, 1=Big, 2=Fire, 3=SmallStar, 4=BigStar, 5=AddLife
 */
enum class PowerState {
    SMALL = 0,
    BIG = 1,
    FIRE = 2,
    SMALL_STAR = 3,
    BIG_STAR = 4,
    ADD_LIFE = 5,
};

/**
 * The Model for Mario. Encapsulates all game logic state:
 *   - Position & velocity
 *   - Power-up state machine
 *   - Grounded / jumping / crouching flags
 *   - Animation key construction
 *   - AABB hitbox calculation
 *
 * Separated from rendering (Player class) per MVC.
 */
class PlayerState {
   public:
    PlayerState();
    ~PlayerState();

    /**
     * Initialize player at a world position.
     * @param worldX Starting X position (pixels, grid-based)
     * @param worldY Starting Y position (pixels, grid-based)
     * @param startState Starting power state (0=small)
     */
    void Init(float worldX, float worldY, int startState = 0);

    /**
     * Tick the physics / state once per frame.
     * Updates velocity, position, animation frame, invincibility timer.
     */
    void Tick();

    // -- Movement Input (called by Controller) --
    void SetMovingRight(bool v) { m_MovingRight = v; }
    void SetMovingLeft(bool v) { m_MovingLeft = v; }
    void SetJumping(bool v);
    void SetCrouching(bool v);
    void SetRunning(bool v) { m_Running = v; }

    /**
     * Apply horizontal velocity based on current input flags.
     * Called each frame by the controller.
     */
    void ApplyMovement(float speed);

    /**
     * Apply gravity / jump physics.
     * @return Y velocity delta to apply
     */
    float ApplyGravity();

    // -- Position --
    float GetX() const { return m_PosX; }
    float GetY() const { return m_PosY; }
    void SetX(float x) { m_PosX = x; }
    void SetY(float y) { m_PosY = y; }
    void SetPosition(float x, float y) {
        m_PosX = x;
        m_PosY = y;
    }

    float GetVelX() const { return m_VelX; }
    float GetVelY() const { return static_cast<float>(m_VelY); }
    void SetVelX(float v) { m_VelX = v; }
    void SetVelY(double v) { m_VelY = v; }

    // -- State --
    int GetState() const;
    PowerState GetPowerState() const;
    void SetPowerState(PowerState ps);

    bool IsGrounded() const { return m_Grounded; }
    bool IsCrouching() const { return m_Crouching; }
    /** True when Mario is in a 2-tile-tall state (Big, Fire, or Big_Star). */
    bool IsBigOrFire() const;
    bool IsMoving() const { return m_MovingRight || m_MovingLeft; }
    bool IsJumping() const { return !m_Grounded; }
    bool IsFacingRight() const { return m_FacingRight; }
    void SetFacingRight(bool v) { m_FacingRight = v; }
    bool IsRunning() const { return m_Running; }
    bool IsDead() const { return m_Dead; }
    bool IsStar() const { return m_StarTimer > 0; }
    bool IsInvincible() const { return m_InvTimer > 0; }
    bool IsPoleSliding() const { return m_PoleSliding; }
    bool IsControllable() const { return m_Controllable; }
    bool IsDeathAnimActive() const {
        return m_DeathAnimation && m_DeathAnimation->IsActive();
    }

    void StartDeathAnimation();
    void UpdateDeathAnimation();

    void SetGrounded(bool v) { m_Grounded = v; }
    void SetDead(bool v) { m_Dead = v; }
    void SetControllable(bool v) { m_Controllable = v; }
    void SetPoleSliding(bool v) { m_PoleSliding = v; }

    /** True while Mario is being carried by a moving platform this frame. */
    bool IsOnMovingPlatform() const { return m_OnMovingPlatform; }
    /** Called by TryCarryPlayer to tag the player as platform-carried. */
    void SetOnMovingPlatform(bool v) { m_OnMovingPlatform = v; }

    /**
     * Take damage. Big -> Small, Small -> Dead.
     */
    void TakeDamage();

    /**
     * Collect a power-up.
     */
    void PowerUp(PowerState newState);

    /**
     * Start star invincibility.
     */
    void StartStar();

    /**
     * Cheat/debug helper: instantly switch to a power state by cyclic index.
     * Handles Y-position adjustment when size changes (SMALL<->BIG boundary),
     * resets invincibility, and sets the star timer when idx==3.
     * @param idx  0=SMALL, 1=BIG, 2=FIRE, 3=STAR (BIG_STAR)
     */
    void ForceApplyPowerState(int idx);

    // -- Hitbox --
    int GetWidth() const;
    int GetHeight() const;
    AABB GetHitbox() const;

    // -- Animation Key --
    /**
     * Get the animation prefix for sprite lookup.
     * Matches C# logic: "Idle", "Right", "Jump", "Crouch", "Pole", "Fire"
     */
    std::string GetAnimPrefix() const;

    /**
     * Get the current animation frame number.
     */
    int GetAnimFrame() const { return m_AnimFrame; }

    // -- Special --
    bool IsFireShooting() const { return m_SpecialActive; }
    int GetSpecialCounter() const { return m_SpecialCounter; }

    bool CanShootFire() const;

    const IPlayerForm* GetForm() const { return m_Form.get(); }

    void SetFireShooting(bool v);

    // -- Jump Height --
    double GetFallHeight() const { return m_FallHeight; }
    void SetFallHeight(double h) { m_FallHeight = h; }

    // -- Invincibility --
    int GetInvTimer() const { return m_InvTimer; }
    int GetStarTimer() const { return m_StarTimer; }
    void SetInvTimer(int t) { m_InvTimer = t; }
    PowerState GetMemoryState() const { return m_MemoryState; }
    void SetMemoryState(PowerState s) { m_MemoryState = s; }

   private:
    // Position & velocity
    float m_PosX = 0.0f;
    float m_PosY = 0.0f;
    float m_VelX = 0.0f;
    double m_VelY = 0.0;

    // Jump physics
    double m_FallHeight = 0.0;

    // State
    PowerState m_PowerState = PowerState::SMALL;
    bool m_Grounded = false;
    bool m_Crouching = false;
    bool m_MovingRight = false;
    bool m_MovingLeft = false;
    bool m_FacingRight = true;
    bool m_Running = false;
    bool m_Dead = false;
    bool m_Controllable = true;
    bool m_PoleSliding = false;
    bool m_OnMovingPlatform =
        false;  // Set by TryCarryPlayer; cleared each frame

    // Invincibility / star
    int m_InvTimer = -1;
    int m_StarTimer = 0;
    int m_StarState = 0;
    PowerState m_MemoryState = PowerState::SMALL;

    // Animation
    int m_AnimFrame = 0;
    int m_AnimTimer = 0;
    int m_TotalFrames = 0;
    bool m_BoomerangAnim = false;
    int m_TargetRate = 20;

    // Fire shooting
    bool m_SpecialActive = false;
    int m_SpecialCounter = 0;
    int m_SpecialLength = 15;

    // Death animation state
    std::unique_ptr<IPlayerDeathAnimation> m_DeathAnimation;

    // Active power-up form strategy
    std::unique_ptr<IPlayerForm> m_Form;
};

}  // namespace Mario

#endif  // MARIO_PLAYER_STATE_HPP
