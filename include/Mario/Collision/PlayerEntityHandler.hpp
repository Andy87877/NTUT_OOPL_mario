/**
 * @file PlayerEntityHandler.hpp
 * @brief Handles Player-Entity collision for one frame.
 *        Covers enemy stomp/damage, shell kick, power-up collection, and
 *        coin pickup. Maintains the NES stomp-combo multiplier state.
 * @inheritance PlayerEntityHandler : ICollisionHandler
 */
#ifndef MARIO_COLLISION_PLAYER_ENTITY_HANDLER_HPP
#define MARIO_COLLISION_PLAYER_ENTITY_HANDLER_HPP

#include <memory>
#include <vector>

#include "Mario/Collision/ICollisionHandler.hpp"
#include "Mario/Entity.hpp"
#include "Mario/Player.hpp"

namespace Mario {

class Camera;
class GameStateManager;
class UIManager;

/**
 * Resolves player-entity collisions each game frame.
 *
 * Responsibilities:
 *   - Star power (player invincible) : instant-kill any enemy
 *   - Stomp  : player falls onto enemy from above
 *               — NES combo: 1st ×1, 2nd ×2, 3rd ×4, 4th ×8, 5th+ = 1000
 *   - Shell kick : stomp or kick a Koopa shell
 *   - Side damage: sideways enemy contact hurts player
 *   - Power-up collection: mushroom, fire flower, star
 *   - Coin collection
 *
 * m_StompCombo persists across frames; it resets when Mario is grounded
 * (can no longer be in a stomp chain).
 */
class PlayerEntityHandler : public ICollisionHandler {
   public:
    PlayerEntityHandler() = default;

    /**
     * Check and resolve all Player-Entity collisions for one frame.
     */
    void Resolve(Player& player, std::vector<std::shared_ptr<Entity>>& entities,
                 Camera& camera, GameStateManager& gameState,
                 UIManager& uiManager);

   private:
    int m_StompCombo = 0;  // NES consecutive-stomp score multiplier

    // Handle overlap with an active enemy entity.
    void HandleEnemyCollision(Player& player, Entity& entity, Camera& camera,
                              GameStateManager& gameState,
                              UIManager& uiManager);

    // Handle overlap with a power-up or coin entity.
    void HandleItemCollision(Player& player, Entity& entity, Camera& camera,
                             GameStateManager& gameState, UIManager& uiManager);
};

}  // namespace Mario

#endif  // MARIO_COLLISION_PLAYER_ENTITY_HANDLER_HPP
