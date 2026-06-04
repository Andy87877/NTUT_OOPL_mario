/**
 * @file PlayerAnimator.hpp
 * @brief Animates the player and resolves sprite paths based on state.
 *        Follows the Single Responsibility Principle (SRP): decouples
 *        animation control from the main Player view class.
 * @inheritance None
 */
#ifndef MARIO_PLAYER_ANIMATOR_HPP
#define MARIO_PLAYER_ANIMATOR_HPP

#include <string>

namespace Mario {

class PlayerState;

class PlayerAnimator {
   public:
    PlayerAnimator() = default;

    /**
     * Resolve the current sprite path for Mario based on state.
     * @param state The PlayerState (Model)
     * @return Full file path to the sprite image
     */
    std::string GetSpritePath(const PlayerState& state) const;

    /**
     * Get the active sprite state index.
     * @param state The PlayerState (Model)
     * @return Sprite state index (0=small, 1=big, 2=fire, 3=small_star, etc.)
     */
    int GetSpriteState(const PlayerState& state) const;

    /**
     * Get the height of the sprite based on its state.
     * @param spriteState The current sprite state (0=small, 1=big, etc.)
     * @return Height of the sprite in pixels
     */
    float GetSpriteHeight(int spriteState) const;
};

}  // namespace Mario

#endif  // MARIO_PLAYER_ANIMATOR_HPP
