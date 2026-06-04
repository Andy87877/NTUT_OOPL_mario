/**
 * @file PlayerAnimator.cpp
 * @brief Implementation of PlayerAnimator (SRP Decoupling).
 *        Resolves the correct sprite path and height parameters based on Mario's state.
 * @inheritance None
 */
#include "Mario/Player/PlayerAnimator.hpp"

#include "Mario/Player/PlayerState.hpp"
#include "Mario/Player/PlayerForm.hpp"
#include "Mario/Core/SpritePathResolver.hpp"
#include "Mario/Core/GameConfig.hpp"

namespace Mario {

std::string PlayerAnimator::GetSpritePath(const PlayerState& state) const {
    std::string prefix;
    int frame;
    int spriteState;
    int starState = 0;

    if (state.IsDeathAnimActive()) {
        prefix = "Jump";
        frame = 0;
        spriteState = 0;
    } else if (state.GetTransitionTimer() > 0) {
        // Alternate sprites every 4 frames during transition
        prefix = "Idle";
        frame = 0;
        bool useBig = (state.GetTransitionTimer() / 4) % 2 == 0;

        if (state.GetPowerState() == PowerState::FIRE && state.GetPrevPowerState() == PowerState::BIG) {
            // Transition from Big to Fire: alternate between Big (state 1) and Fire (state 2)
            spriteState = useBig ? 2 : 1;
        } else {
            // Transition between Small and Big/Fire: alternate between Small (state 0) and Big (state 1)
            spriteState = useBig ? 1 : 0;
        }
        starState = 0;
    } else {
        prefix = state.GetAnimPrefix();
        frame = state.GetAnimFrame();

        const auto* form = state.GetForm();
        bool isShooting = state.IsFireShooting();
        spriteState = form->GetSpriteState(isShooting);
        starState = form->GetStarState(state.GetStarTimer(), isShooting);
    }

    return SpritePathResolver::GetPlayerSpritePath(prefix, spriteState, frame, starState);
}

int PlayerAnimator::GetSpriteState(const PlayerState& state) const {
    if (state.IsDeathAnimActive()) {
        return 0;
    } else if (state.GetTransitionTimer() > 0) {
        bool useBig = (state.GetTransitionTimer() / 4) % 2 == 0;
        if (state.GetPowerState() == PowerState::FIRE && state.GetPrevPowerState() == PowerState::BIG) {
            return useBig ? 2 : 1;
        } else {
            return useBig ? 1 : 0;
        }
    } else {
        const auto* form = state.GetForm();
        bool isShooting = state.IsFireShooting();
        return form->GetSpriteState(isShooting);
    }
}

float PlayerAnimator::GetSpriteHeight(int spriteState) const {
    // The Big/Fire sprite canvas is always 2-tile tall; the Small sprite is 1-tile tall.
    // spriteState == 0 (SMALL) and spriteState == 3 (SMALL_STAR) are both 1-tile tall.
    return (spriteState == 0 || spriteState == 3)
               ? static_cast<float>(GameConfig::TILE_SIZE)
               : static_cast<float>(GameConfig::TILE_SIZE * 2);
}

}  // namespace Mario
