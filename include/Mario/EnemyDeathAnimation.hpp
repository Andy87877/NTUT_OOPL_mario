/**
 * @file EnemyDeathAnimation.hpp
 * @brief Enemy death-animation strategy interfaces and concrete styles.
 *        Each enemy can own a different death style (Goomba squish, Koopa
 *        shell-retreat, fireball flip) while preserving OOP polymorphism.
 * @inheritance IEnemyDeathAnimation <- GoombaSquishDeathAnimation
 *              IEnemyDeathAnimation <- KoopaRetreatDeathAnimation
 *              IEnemyDeathAnimation <- FireballFlipDeathAnimation
 *              IEnemyDeathAnimation <- ClassicEnemyDeathAnimation
 */
#ifndef MARIO_ENEMY_DEATH_ANIMATION_HPP
#define MARIO_ENEMY_DEATH_ANIMATION_HPP

namespace Mario {

enum class EnemyDeathCause {
    STOMP,
    FIREBALL,
    SHELL_HIT,
    STAR_HIT,
    GENERIC,
};

/**
 * Mutable runtime values exposed to death-animation strategies.
 */
struct EnemyDeathRuntime {
    int currentFrameCounter;
    bool& isEnemy;
    bool& isSquashed;
    bool& applyGravity;
    float& velX;
    double& velY;
    float& posY;
    bool& isActive;
};

/**
 * Strategy interface for enemy death animation behavior.
 */
class IEnemyDeathAnimation {
   public:
    virtual ~IEnemyDeathAnimation() = default;

    virtual const char* GetName() const = 0;
    virtual void Start(EnemyDeathCause cause, EnemyDeathRuntime& runtime) = 0;
    virtual void Tick(EnemyDeathRuntime& runtime, float gravity,
                      float tickInterval) = 0;
    virtual bool IsActive() const = 0;
};

/**
 * Generic fallback enemy death timing.
 * Uses quick squash despawn for stomp-like causes.
 */
class ClassicEnemyDeathAnimation : public IEnemyDeathAnimation {
   public:
    const char* GetName() const override { return "ClassicEnemyDeath"; }
    void Start(EnemyDeathCause cause, EnemyDeathRuntime& runtime) override;
    void Tick(EnemyDeathRuntime& runtime, float gravity,
              float tickInterval) override;
    bool IsActive() const override { return m_Active; }

   private:
    bool m_Active = false;
    int m_DeleteAtFrame = 0;
    static constexpr int kQuickSquashFrames = 30;
};

/**
 * Goomba-style death:
 * - STOMP/SHELL_HIT/STAR_HIT: squash then disappear almost immediately.
 * - FIREBALL: flip and despawn.
 */
class GoombaSquishDeathAnimation : public IEnemyDeathAnimation {
   public:
    const char* GetName() const override { return "GoombaSquishDeath"; }
    void Start(EnemyDeathCause cause, EnemyDeathRuntime& runtime) override;
    void Tick(EnemyDeathRuntime& runtime, float gravity,
              float tickInterval) override;
    bool IsActive() const override { return m_Active; }

   private:
    bool m_Active = false;
    int m_DeleteAtFrame = 0;
    static constexpr int kQuickSquashFrames = 30;
    static constexpr int kFlipDespawnFrames = 45;
};

/**
 * Koopa-style death policy:
 * - STOMP => retreat into shell (no auto-delete)
 * - FIREBALL/SHELL_HIT/STAR_HIT => flipped arc then short despawn
 */
class KoopaRetreatDeathAnimation : public IEnemyDeathAnimation {
   public:
    const char* GetName() const override { return "KoopaRetreatDeath"; }
    void Start(EnemyDeathCause cause, EnemyDeathRuntime& runtime) override;
    void Tick(EnemyDeathRuntime& runtime, float gravity,
              float tickInterval) override;
    bool IsActive() const override { return m_Active; }

   private:
    enum class Mode {
        NONE,
        SHELL,
        FLIPPED,
    };

    bool m_Active = false;
    Mode m_Mode = Mode::NONE;
    int m_DeleteAtFrame = 0;
    static constexpr int kFlipDespawnFrames = 45;
};

/**
 * Fireball/Shell-hit generic death: flip upward and short despawn.
 */
class FireballFlipDeathAnimation : public IEnemyDeathAnimation {
   public:
    const char* GetName() const override { return "FireballFlipDeath"; }
    void Start(EnemyDeathCause cause, EnemyDeathRuntime& runtime) override;
    void Tick(EnemyDeathRuntime& runtime, float gravity,
              float tickInterval) override;
    bool IsActive() const override { return m_Active; }

   private:
    bool m_Active = false;
    int m_DeleteAtFrame = 0;
    static constexpr int kFlipDespawnFrames = 45;
    static constexpr double kInitialFlipVelY = -8.0;
};

}  // namespace Mario

#endif  // MARIO_ENEMY_DEATH_ANIMATION_HPP