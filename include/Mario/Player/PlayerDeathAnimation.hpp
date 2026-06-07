/**
 * @file PlayerDeathAnimation.hpp
 * @brief Player death-animation strategy interfaces.
 *        Encapsulates death motion timing so PlayerState remains a clean model.
 * @inheritance IPlayerDeathAnimation <- ClassicPlayerDeathAnimation <- TumblePlayerDeathAnimation
 */
#ifndef MARIO_PLAYER_DEATH_ANIMATION_HPP
#define MARIO_PLAYER_DEATH_ANIMATION_HPP

namespace Mario {

/**
 * Strategy interface for Mario death animation motion.
 */
class IPlayerDeathAnimation {
   public:
    virtual ~IPlayerDeathAnimation() = default;

    virtual void Start() = 0;
    virtual void Tick(float gravity, float tickInterval, float jumpVelocity,
                      float& playerY) = 0;
    virtual bool IsActive() const = 0;

    /** Get visual rotation angle in radians for death animation. */
    virtual float GetRotation() const { return 0.0f; }

    /** Get visual scale Y modifier for death animation. */
    virtual float GetScaleY() const { return 1.0f; }
};

/**
 * NES-like death animation: short freeze then upward launch and fall.
 */
class ClassicPlayerDeathAnimation : public IPlayerDeathAnimation {
   public:
    ClassicPlayerDeathAnimation() = default;
    virtual ~ClassicPlayerDeathAnimation() override = default;

    virtual void Start() override;
    virtual void Tick(float gravity, float tickInterval, float jumpVelocity,
                      float& playerY) override;
    virtual bool IsActive() const override { return m_Active; }

   protected:
    bool m_Active = false;
    bool m_Launched = false;
    int m_FrameCounter = 0;
    double m_VelY = 0.0;

    static constexpr int kFreezeFrames = 30; // Classic NES half-second freeze frame delay
    static constexpr double kLaunchMultiplier = 1.0; // Launch velocity scaling factor
    static constexpr double kGravityMultiplier = 2.0; // Lower gravity factor for a graceful floaty fall
};

/**
 * Premium tumble death animation: freeze, jump, fall, and spin tumble with a shrinking effect.
 */
class TumblePlayerDeathAnimation : public ClassicPlayerDeathAnimation {
   public:
    TumblePlayerDeathAnimation() = default;
    ~TumblePlayerDeathAnimation() override = default;

    void Start() override;
    void Tick(float gravity, float tickInterval, float jumpVelocity,
              float& playerY) override;
    float GetRotation() const override { return m_Rotation; }
    float GetScaleY() const override;

   private:
    float m_Rotation = 0.0f;
};

}  // namespace Mario

#endif  // MARIO_PLAYER_DEATH_ANIMATION_HPP