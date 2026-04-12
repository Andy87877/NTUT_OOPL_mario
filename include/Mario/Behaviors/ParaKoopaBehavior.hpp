// /**
//  * @file ParaKoopaBehavior.hpp
//  * @brief Paratroopa (winged Koopa) enemy behavior using Strategy pattern.
//  *        Implements floating oscillation pattern with landing mechanic.
//  * @inheritance IEntityBehavior <- ParaKoopaBehavior
//  */
// #ifndef MARIO_PARA_KOOPA_BEHAVIOR_HPP
// #define MARIO_PARA_KOOPA_BEHAVIOR_HPP

// #include "Mario/Behaviors/IEntityBehavior.hpp"

// namespace Mario {

// /**
//  * ParaKoopaBehavior — Flying Koopa with oscillating movement.
//  * Floats up/down in sine wave while moving horizontally.
//  * Loses wings and becomes grounded when jumped on.
//  *
//  * Implementation via Strategy Pattern:
//  *  - Uses EntityState flags to track flying vs grounded state
//  *  - Phase-based oscillation without gravity when airborne
//  */
// class ParaKoopaBehavior : public IEntityBehavior {
//    public:
//     ParaKoopaBehavior() = default;
//     virtual ~ParaKoopaBehavior() = default;

//     /**
//      * Update Para-Koopa with oscillation and ground collision logic.
//      */
//     void Update(EntityState& state, const Level& level, const Player& player,
//                 int gameTimer) override;

//     /**
//      * Handle player collision (landing).
//      */
//     bool OnPlayerCollision(EntityState& state, Player& player,
//                            bool isFromAbove) override;

//     /**
//      * Clone this behavior.
//      */
//     std::unique_ptr<IEntityBehavior> Clone() const override;

//     const char* GetName() const override { return "ParaKoopaBehavior"; }

//    private:
//     // Flight parameters
//     static constexpr float WALK_SPEED = 0.5f;
//     static constexpr float FLOAT_AMPLITUDE = 1.5f;  // Oscillation distance
//     static constexpr float FLOAT_FREQUENCY = 2.0f;  // Oscillation rate

//     float m_FloatPhase = 0.0f;  // Current position in sine wave
//     float m_OriginalY = 0.0f;   // Starting Y for oscillation offset
//     bool m_IsFlying = true;     // Track if still has wings
// };

// }  // namespace Mario

// #endif  // MARIO_PARA_KOOPA_BEHAVIOR_HPP
