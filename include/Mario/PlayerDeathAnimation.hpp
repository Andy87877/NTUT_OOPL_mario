/**
 * @file PlayerDeathAnimation.hpp
 * @brief Player death-animation strategy interfaces.
 *        Encapsulates death motion timing so PlayerState remains a clean model.
 * @inheritance IPlayerDeathAnimation <- ClassicPlayerDeathAnimation
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
};

/**
 * NES-like death animation: short freeze then upward launch and fall.
 */
class ClassicPlayerDeathAnimation : public IPlayerDeathAnimation {
   public:
    void Start() override;
    void Tick(float gravity, float tickInterval, float jumpVelocity,
              float& playerY) override;
    bool IsActive() const override { return m_Active; }

   private:
    bool m_Active = false;
    bool m_Launched = false;
    int m_FrameCounter = 0;
    double m_VelY = 0.0;

    static constexpr int kFreezeFrames = 6;
    static constexpr double kLaunchMultiplier = 1.35;
    static constexpr double kGravityMultiplier = 2.8;
};

}  // namespace Mario

#endif  // MARIO_PLAYER_DEATH_ANIMATION_HPP