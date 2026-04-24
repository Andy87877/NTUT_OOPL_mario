/**
 * @file ParticleDebris.hpp
 * @brief Block breaking particle effects with gravity and rotation
 * @inheritance IEntityBehavior -> ParticleDebris
 */
#ifndef MARIO_PARTICLE_DEBRIS_HPP
#define MARIO_PARTICLE_DEBRIS_HPP

#include "Mario/Behaviors/IEntityBehavior.hpp"

namespace Mario {

class ParticleDebris : public IEntityBehavior {
   private:
    float m_InitialVelX, m_InitialVelY;
    int m_LifetimeFrames;
    float m_RotationAngle;

   public:
    ParticleDebris();
    virtual ~ParticleDebris() = default;

    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;

    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;

    std::unique_ptr<IEntityBehavior> Clone() const override;

    const char* GetName() const override { return "ParticleDebris"; }

    // Set initial custom velocity right after creation
    void SetInitialVelocity(float vx, float vy) {
        m_InitialVelX = vx;
        m_InitialVelY = vy;
    }
};

}  // namespace Mario

#endif