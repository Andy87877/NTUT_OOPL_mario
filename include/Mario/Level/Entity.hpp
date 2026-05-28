/**
 * @file Entity.hpp
 * @brief Entity View layer — renders enemies, power-ups, coins.
 *        Inherits from Util::GameObject for PTSD rendering.
 *        Contains EntityState as the Model (MVC pattern).
 * @inheritance Util::GameObject -> Entity
 */
#ifndef MARIO_ENTITY_HPP
#define MARIO_ENTITY_HPP

#include <memory>
#include <string>
#include <unordered_map>

#include "Mario/Behaviors/IEntityBehavior.hpp"
#include "Mario/Core/Collider.hpp"
#include "Mario/Core/GameConfig.hpp"
#include "Mario/Level/EntityDef.hpp"
#include "Mario/Level/EntityState.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "pch.hpp"  // IWYU pragma: export

namespace Mario {

// Forward declaration
class IEntityBehavior;
class Level;
class Player;

/**
 * Entity (View) — inherits Util::GameObject.
 * Renders Goomba, KoopaTroopa, Mushroom, FireFlower, Star, Coin, etc.
 * Uses EntityState as Model and reads sprite data from EntityDef.
 * Behavior (Strategy Pattern) handles AI and collision logic.
 */
class Entity : public Util::GameObject {
   public:
    /**
     * Construct an entity at the given world position.
     * @param def Entity definition from EntityList.csv lookup
     * @param worldX World X position (grid-based, left edge)
     * @param worldY World Y position (grid-based, top edge)
     * @param direction 0=Left, 1=Right, 2=None
     * @param fromBlock Whether spawned from hitting a block
     * @param levelName Level name for sprite path resolution (e.g., "1-1",
     * "8-4")
     */
    Entity(const EntityDef& def, float worldX, float worldY, int direction = 1,
           bool fromBlock = false, const std::string& levelName = "1-1");

    /**
     * Update the View: select sprite, convert world->screen coords.
     * @param cameraOffset Current camera scroll offset
     */
    void UpdateView(float cameraOffset);

    /**
     * Set the behavior strategy for this entity.
     * Behavior handles AI logic and collision response.
     * @param behavior IEntityBehavior implementation
     */
    void SetBehavior(std::unique_ptr<IEntityBehavior> behavior) {
        m_Behavior = std::move(behavior);
    }

    /**
     * Get current behavior (for debugging/inspection).
     */
    IEntityBehavior* GetBehavior() const { return m_Behavior.get(); }

    /**
     * Get level name for sprite resolution.
     */
    const std::string& GetLevelName() const { return m_LevelName; }

    /**
     * Get entity definition (entity type, properties).
     */
    const EntityDef& GetDef() const { return m_Def; }

    // -- Model access --
    EntityState& GetState() { return m_State; }
    const EntityState& GetState() const { return m_State; }

    // -- World position (for camera/collision) --
    float GetWorldX() const { return m_State.GetX(); }
    float GetWorldY() const { return m_State.GetY(); }
    AABB GetHitbox() const;

   private:
    std::string BuildSpritePath() const;
    std::shared_ptr<Util::Image> GetOrLoadSprite(const std::string& path);

    /**
     * One-time hitbox / size initialization triggered on first sprite load.
     * Reads EntityDef data fields (fixedHitboxTiles, renderTargetWidth) and
     * sets EntityState dimensions accordingly — called from UpdateView only
     * when m_SizeInitialized is false.  Extracted from UpdateView to keep
     * that method focused on per-frame transform updates (SRP).
     */
    void InitializeSizeOnce(const glm::vec2& spriteSize);

    EntityState m_State;
    EntityDef m_Def;
    std::string m_LevelName;  // Level name for sprite path resolution (e.g.,
                              // "1-1", "8-4")

    std::string m_CurrentSpritePath;

    // Flag: size and Y position are initialized only once on first sprite load.
    // Prevents animated entities (Princess, Goomba) from drifting upward every
    // animation frame change when spriteSize.y > spriteSize.x.
    bool m_SizeInitialized = false;

    std::unique_ptr<IEntityBehavior> m_Behavior;  // Strategy Pattern behavior
};

}  // namespace Mario

#endif  // MARIO_ENTITY_HPP
