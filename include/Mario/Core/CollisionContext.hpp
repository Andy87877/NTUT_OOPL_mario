/**
 * @file CollisionContext.hpp
 * @brief Data transfer object carrying references needed during collision handling.
 *        Passed to collision strategies so they can access game systems.
 * @inheritance None (data struct)
 */
#ifndef MARIO_COLLISION_CONTEXT_HPP
#define MARIO_COLLISION_CONTEXT_HPP

#include <memory>

namespace Mario {

// Forward declarations to avoid circular includes
class Player;
class Level;
class EntityFactory;
class GameStateManager;

/**
 * Bundles all the references needed during collision resolution.
 * Prevents collision strategies from needing direct knowledge of
 * the entire game state.
 */
struct CollisionContext {
    std::shared_ptr<Player> player;
    std::shared_ptr<Level> level;
    EntityFactory* entityFactory = nullptr;
    GameStateManager* gameState  = nullptr;
    int gameTimer = 0;
    int invTimer  = -1;
};

} // namespace Mario

#endif // MARIO_COLLISION_CONTEXT_HPP
