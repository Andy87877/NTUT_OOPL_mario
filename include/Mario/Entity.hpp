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
#include "Mario/EntityDef.hpp"
#include "Mario/EntityState.hpp"
#include "Mario/GameConfig.hpp"
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

    // -- Model access --
    EntityState& GetState() { return m_State; }
    const EntityState& GetState() const { return m_State; }

    // -- World position (for camera/collision) --
    float GetWorldX() const { return m_State.GetX(); }
    float GetWorldY() const { return m_State.GetY(); }

   private:
    std::string BuildSpritePath() const;
    std::shared_ptr<Util::Image> GetOrLoadSprite(const std::string& path);

    EntityState m_State;
    EntityDef m_Def;
    std::string m_LevelName;  // Level name for sprite path resolution (e.g.,
                              // "1-1", "8-4")

    std::string m_CurrentSpritePath;
    std::unordered_map<std::string, std::shared_ptr<Util::Image>> m_SpriteCache;

    std::unique_ptr<IEntityBehavior> m_Behavior;  // Strategy Pattern behavior
};

}  // namespace Mario

#endif  // MARIO_ENTITY_HPP
