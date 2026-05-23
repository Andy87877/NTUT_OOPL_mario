/**
 * @file CollisionManager.cpp
 * @brief Thin facade: forwards each public call to the matching handler.
 *        All collision logic lives in the Collision/ subsystem classes.
 *        Adding a new collision type only requires a new handler ??no changes
 *        to this file.
 * @inheritance None (facade)
 */
#include "Mario/CollisionManager.hpp"

#include "Mario/Camera.hpp"
#include "Mario/Entity.hpp"
#include "Mario/GameConfig.hpp"
#include "Mario/GameStateManager.hpp"
#include "Mario/UIManager.hpp"

namespace Mario {

// ============================================================================
// CheckPlayerBlockCollision ??delegates to PlayerBlockHandler
// ============================================================================
void CollisionManager::CheckPlayerBlockCollision(
    Player& player, Level& level, Camera& camera, GameStateManager& gameState,
    UIManager& uiManager, std::vector<Level::SpawnPoint>* outSpawns) {
    m_PlayerBlockHandler.Resolve(player, level, camera, gameState, uiManager,
                                 outSpawns);
}

// ============================================================================
// CheckPitFall ??simple boundary test (no handler needed)
// ============================================================================
bool CollisionManager::CheckPitFall(const Player& player) const {
    float levelBottom = static_cast<float>(GameConfig::LEVEL_HEIGHT_PX);
    return player.GetState().GetY() > levelBottom + GameConfig::TILE_SIZE;
}

// ============================================================================
// CheckPlayerEntityCollision ??delegates to PlayerEntityHandler
// ============================================================================
void CollisionManager::CheckPlayerEntityCollision(
    Player& player, std::vector<std::shared_ptr<Entity>>& entities,
    Camera& camera, GameStateManager& gameState, UIManager& uiManager) {
    m_PlayerEntityHandler.Resolve(player, entities, camera, gameState,
                                  uiManager);
}

// ============================================================================
// CheckEntityEntityCollision ??delegates to EntityEntityHandler
// ============================================================================
void CollisionManager::CheckEntityEntityCollision(
    std::vector<std::shared_ptr<Entity>>& entities, GameStateManager& gameState,
    float cameraOffset) {
    m_EntityEntityHandler.Resolve(entities, gameState, cameraOffset);
}

// ============================================================================
// CheckEntityBlockCollision ??delegates to EntityBlockHandler
// ============================================================================
void CollisionManager::CheckEntityBlockCollision(
    Entity& entity, Level& level,
    std::vector<std::shared_ptr<Entity>>* outNewEntities) {
    m_EntityBlockHandler.Resolve(entity, level, outNewEntities);
}

}  // namespace Mario
