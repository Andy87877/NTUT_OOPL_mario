/**
 * @file EnemyDeathStyleFactory.hpp
 * @brief Factory for selecting enemy death-animation strategies by entity type.
 *        Keeps EntityFactory focused on entity creation and behavior wiring.
 * @inheritance None (factory pattern)
 */
#ifndef MARIO_ENEMY_DEATH_STYLE_FACTORY_HPP
#define MARIO_ENEMY_DEATH_STYLE_FACTORY_HPP

#include <memory>

#include "Mario/Level/EnemyDeathAnimation.hpp"
#include "Mario/Level/EntityDef.hpp"

namespace Mario {

class EnemyDeathStyleFactory {
   public:
    static std::unique_ptr<IEnemyDeathAnimation> CreateFor(EntityType type);
};

}  // namespace Mario

#endif  // MARIO_ENEMY_DEATH_STYLE_FACTORY_HPP