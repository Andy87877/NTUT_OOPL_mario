// /**
//  * @file ParaKoopaBehavior.cpp
//  * @brief Implementation of Paratroopa floating behavior with landing
//  mechanic.
//  * @inheritance IEntityBehavior <- ParaKoopaBehavior
//  */
// #include "Mario/Behaviors/ParaKoopaBehavior.hpp"

// #include <cmath>

// #include "Mario/EntityState.hpp"
// #include "Mario/Level.hpp"
// #include "Mario/PhysicsEngine.hpp"
// #include "Mario/Player.hpp"
// #include "Util/Logger.hpp"

// namespace Mario {

// void ParaKoopaBehavior::Update(EntityState& state, const Level& level,
//                                const Player& player, int gameTimer) {
//     if (state.IsSquished() || state.IsDead()) {
//         return;
//     }

//     // Para-Koopa only floats up/down, does not move left/right
//     // Only horizontal movement is from player knocking it

//     if (m_IsFlying) {
//         // -- Floating behavior (no gravity, oscillate vertically) --
//         m_FloatPhase +=
//             FLOAT_FREQUENCY * 0.016f;  // deltaTime ≈ 0.016s per frame
//         if (m_FloatPhase > 2.0f * 3.14159f) {
//             m_FloatPhase -= 2.0f * 3.14159f;
//         }

//         // Oscillate vertically
//         float offset = FLOAT_AMPLITUDE * std::sin(m_FloatPhase);
//         float newY = m_OriginalY + offset;
//         state.SetWorldY(newY);
//         state.SetVelY(0);  // No vertical velocity when floating

//     } else {
//         // -- Grounded behavior (normal gravity, like regular Koopa) --
//         double fallHeight = state.GetFallHeight();
//         double velY = state.GetVelY();
//         float yDelta =
//             PhysicsEngine::ApplyGravity(fallHeight, velY,
//             state.IsGrounded());
//         state.SetFallHeight(fallHeight);
//         state.SetVelY(velY);
//         state.SetWorldY(state.GetWorldY() + yDelta);
//     }

//     // Para-Koopa does not move horizontally - only floats vertically
//     // No wall collision or direction changes needed
//     // It stays in its original X position while floating up/down

//     // Animation
//     if (state.IsAnimated() && gameTimer % 10 == 0) {
//         state.AdvanceAnimationFrame();
//     }
// }

// bool ParaKoopaBehavior::OnPlayerCollision(EntityState& state, Player& player,
//                                           bool isFromAbove) {
//     if (state.IsSquished() || state.IsDead()) {
//         return false;
//     }

//     if (isFromAbove && m_IsFlying) {
//         // Mario jumped on flying Koopa - lose wings and fall
//         m_IsFlying = false;
//         m_FloatPhase = 0.0f;
//         // Now subject to normal gravity
//         return true;
//     } else if (isFromAbove && !m_IsFlying) {
//         // Grounded Para-Koopa can be squished like normal Koopa
//         if (state.IsSquishable()) {
//             state.Squish();
//             return true;
//         }
//     }

//     return false;
// }

// std::unique_ptr<IEntityBehavior> ParaKoopaBehavior::Clone() const {
//     return std::make_unique<ParaKoopaBehavior>(*this);
// }

// }  // namespace Mario
