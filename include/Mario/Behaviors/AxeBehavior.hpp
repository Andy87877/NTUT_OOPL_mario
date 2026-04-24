/**
 * @file AxeBehavior.hpp
 * @brief Behavior for the World 8-4 Boss Bridge Axe
 * @inheritance IEntityBehavior -> AxeBehavior
 */
#ifndef MARIO_AXE_BEHAVIOR_HPP
#define MARIO_AXE_BEHAVIOR_HPP

#include "Mario/Behaviors/IEntityBehavior.hpp"

namespace Mario {
class AxeBehavior : public IEntityBehavior {
   private:
    int m_AnimationFrame;

   public:
    AxeBehavior() : m_AnimationFrame(0) {}
    virtual ~AxeBehavior() = default;

    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;

    // Checks player hit - triggers Bowser bridge collapse
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;

    std::unique_ptr<IEntityBehavior> Clone() const override {
        return std::make_unique<AxeBehavior>(*this);
    }
    const char* GetName() const override { return "AxeBehavior"; }
};
}  // namespace Mario
#endif