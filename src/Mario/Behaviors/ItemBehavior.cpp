/**
 * @file ItemBehavior.cpp
 * @brief Implementation of power-up and collectible item behavior.
 * @inheritance IEntityBehavior <- ItemBehavior
 */
#include "Mario/Behaviors/ItemBehavior.hpp"

#include "Mario/Collider.hpp"
#include "Mario/EntityState.hpp"
#include "Mario/Level.hpp"
#include "Mario/PhysicsEngine.hpp"
#include "Mario/Player.hpp"
#include "Mario/PlayerState.hpp"
#include "Util/Logger.hpp"

namespace Mario {

ItemBehavior::ItemBehavior(ItemType type) : m_Type(type) {}

void ItemBehavior::Update(EntityState& state, const Level& level,
                          const Player& player, int gameTimer) {
    // Coins don't move - just animate in place
    if (m_Type == ItemType::COIN) {
        if (state.IsAnimated()) {
            state.AdvanceAnimationFrame();
        }
        return;
    }

    // Power-ups that bounce when spawned
    if (state.IsStatic()) {
        // Static items (in blocks) don't move until spawned
        if (state.IsAnimated()) {
            state.AdvanceAnimationFrame();
        }
        return;
    }

    // Spawned items (mushroom, fire flower) bounce horizontally
    // Apply gravity
    double fallHeight = state.GetFallHeight();
    double velY = state.GetVelY();
    float yDelta =
        PhysicsEngine::ApplyGravity(fallHeight, velY, state.IsGrounded());
    state.SetFallHeight(fallHeight);
    state.SetVelY(velY);
    state.SetWorldY(state.GetWorldY() + yDelta);

    // Horizontal movement
    float xDelta = state.GetVelX() * (state.GetDirection() == 1 ? 1 : -1);
    state.SetWorldX(state.GetWorldX() + xDelta);

    // Check ground collision for bouncing
    AABB footprint = state.GetCollider();
    footprint.top = footprint.bottom;
    footprint.bottom += GameConfig::GRAVITY_ACCELERATION;

    bool isGroundedNow = false;
    const std::vector<std::shared_ptr<Block>>& blocks = level.GetAllBlocks();
    for (const auto& block : blocks) {
        if (block && footprint.Intersects(block->GetAABB())) {
            isGroundedNow = true;
            break;
        }
    }

    if (isGroundedNow && !state.IsGrounded()) {
        // Just landed - bounce
        state.SetFallHeight(GameConfig::MUSHROOM_BOUNCE_HEIGHT);
    }

    state.SetGrounded(isGroundedNow);

    // Check for walls - change direction
    AABB wallCheck = state.GetCollider();
    if (state.GetDirection() == 1) {
        wallCheck.left = wallCheck.right;
    } else {
        wallCheck.right = wallCheck.left;
    }

    bool hitWall = false;
    for (const auto& block : blocks) {
        if (block && wallCheck.Intersects(block->GetAABB())) {
            hitWall = true;
            break;
        }
    }

    if (hitWall) {
        // Reverse direction
        int dir = state.GetDirection();
        state.SetDirection(dir == 1 ? 0 : 1);
    }

    // Animation
    if (state.IsAnimated()) {
        m_MoveFrameCounter++;
        if (m_MoveFrameCounter % 8 == 0) {
            state.AdvanceAnimationFrame();
        }
    }
}

bool ItemBehavior::OnPlayerCollision(EntityState& state, Player& player,
                                     bool isFromAbove) {
    if (m_Type == ItemType::COIN) {
        // Coins are always collected
        state.Delete();
        return true;
    }

    if (m_Type == ItemType::MUSHROOM) {
        // If player is small, grow to big
        // This is handled in App collision manager
        state.Delete();
        return true;
    }

    if (m_Type == ItemType::FIRE_FLOWER) {
        // Grant fire power
        state.Delete();
        return true;
    }

    if (m_Type == ItemType::STAR) {
        // Grant invincibility
        state.Delete();
        return true;
    }

    if (m_Type == ItemType::ONE_UP) {
        // Grant extra life
        state.Delete();
        return true;
    }

    return false;
}

std::unique_ptr<IEntityBehavior> ItemBehavior::Clone() const {
    return std::make_unique<ItemBehavior>(*this);
}

}  // namespace Mario
