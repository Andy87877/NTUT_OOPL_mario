/**
 * @file EntityAnimator.hpp
 * @brief Animates entities and resolves their sprite paths based on state.
 *        Follows the Single Responsibility Principle (SRP): decouples
 *        animation control from the main Entity view class.
 * @inheritance None
 */
#ifndef MARIO_ENTITY_ANIMATOR_HPP
#define MARIO_ENTITY_ANIMATOR_HPP

#include <string>

namespace Mario {

class EntityState;
struct EntityDef;

class EntityAnimator {
   public:
    EntityAnimator() = default;

    /**
     * Resolve the current sprite path for the entity.
     * @param state     The EntityState (Model)
     * @param def       The EntityDef configuration
     * @param levelName The current level name
     * @return Full file path to the sprite image
     */
    std::string GetSpritePath(const EntityState& state, const EntityDef& def,
                              const std::string& levelName) const;
};

}  // namespace Mario

#endif  // MARIO_ENTITY_ANIMATOR_HPP
