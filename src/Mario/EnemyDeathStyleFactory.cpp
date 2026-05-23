/**
 * @file EnemyDeathStyleFactory.cpp
 * @brief Implementation of EnemyDeathStyleFactory.
 *        Maps each enemy archetype to its death-animation strategy.
 * @inheritance None (factory pattern)
 */
#include "Mario/EnemyDeathStyleFactory.hpp"

namespace Mario {

std::unique_ptr<IEnemyDeathAnimation> EnemyDeathStyleFactory::CreateFor(
    EntityType type) {
    switch (type) {
        case EntityType::GOOMBA:
            return std::make_unique<GoombaSquishDeathAnimation>();
        case EntityType::KOOPA_TROOPA:
        case EntityType::PARAKOOPA:
        case EntityType::KOOPA_SHELL:
            return std::make_unique<KoopaRetreatDeathAnimation>();
        case EntityType::AXE_KOOPA:
        case EntityType::PIRANHA_PLANT:
        case EntityType::PODOBOO:
        case EntityType::BOWSER:
            return std::make_unique<FireballFlipDeathAnimation>();
        default:
            return std::make_unique<ClassicEnemyDeathAnimation>();
    }
}

}  // namespace Mario