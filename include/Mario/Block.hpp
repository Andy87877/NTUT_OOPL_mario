/**
 * @file Block.hpp
 * @brief Block game object for terrain tiles (ground, bricks, pipes, etc.).
 *        Inherits from Util::GameObject for rendering in PTSD framework.
 * @inheritance Util::GameObject -> Block
 */
#ifndef MARIO_BLOCK_HPP
#define MARIO_BLOCK_HPP

#include <memory>
#include <string>
#include <vector>

#include "Mario/Collider.hpp"
#include "Mario/EntityDef.hpp"
#include "Mario/GameConfig.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "pch.hpp"

namespace Mario {

/**
 * Represents a single tile/block in the level grid.
 * Inherits Util::GameObject for PTSD rendering pipeline.
 *
 * Responsibilities:
 *   - Render the correct sprite based on block type
 *   - Handle hit events (bounce, break, spawn items)
 *   - Provide AABB for collision detection
 *   - Support animation (e.g., question blocks)
 */
class Block : public Util::GameObject {
   public:
    /**
     * Construct a block from its grid position and definition data.
     * @param blockID The block type ID from the level CSV
     * @param gridX Grid column index
     * @param gridY Grid row index
     * @param def Block definition from IDList.csv
     */
    Block(int blockID, int gridX, int gridY, const BlockDef& def);

    virtual ~Block() = default;

    /**
     * Update block state (animation, bounce effect).
     * @param cameraOffset Current camera scroll offset
     */
    void Update(float cameraOffset);

    /**
     * Called when player head-bumps this block.
     * @param playerState Player's power state (0=small, 1=big, 2=fire)
     */
    void OnHit(int playerState);

    /**
     * Trigger bounce animation.
     */
    void Bounce();

    /**
     * Break this block (make invisible and non-solid).
     */
    void Break();

    // -- Getters --
    int GetBlockID() const { return m_BlockID; }
    int GetGridX() const { return m_GridX; }
    int GetGridY() const { return m_GridY; }
    const std::string& GetName() const { return m_Def.name; }

    bool IsSolid() const { return m_Solid; }
    bool IsBreakable() const { return m_Def.breakable; }
    bool IsBackground() const { return m_Def.background; }
    bool IsGoal() const { return m_Def.isGoal; }
    bool IsContainer() const { return m_Def.isContainer; }
    bool IsHit() const { return m_IsHit; }
    bool IsSpawner() const { return m_Def.spawner; }

    bool JustBroken() {
        if (m_JustBroken) {
            m_JustBroken = false;
            return true;
        }
        return false;
    }

    virtual float GetWorldX() const;
    virtual float GetWorldY() const;
    virtual AABB GetAABB() const;

    /**
     * Get the contents/entity to spawn when this block is hit.
     * If the player is big/fire and contents = "Mushroom", returns
     * "FireFlower".
     */
    std::string GetSpawnContents(int playerState) const;

    const BlockDef& GetDef() const { return m_Def; }

    void SetCollidable(bool collidable) { m_Solid = collidable; }

    /**
     * Enable physics gravity on this block (used for bridge collapse in 8-4).
     * Once enabled, the block falls downward each Update() tick.
     * @param gravity true to start falling
     */
    void SetGravity(bool gravity) {
        if (gravity && !m_HasGravity) {
            m_HasGravity = true;
            m_FallVelocity = 0.0f;
            // Snapshot world position before entering physics mode
            m_WorldX = static_cast<float>(m_GridX * GameConfig::TILE_SIZE);
            m_WorldY = static_cast<float>(m_GridY * GameConfig::TILE_SIZE);
        }
    }

   protected:
    int m_BlockID;
    int m_GridX;
    int m_GridY;
    BlockDef m_Def;

    bool m_Solid;
    bool m_IsHit = false;
    bool m_JustBroken = false;
    int m_HP = 1;

    // Bounce animation
    float m_BounceHeight = 0.0f;

    // Gravity physics (for bridge collapse)
    bool m_HasGravity = false;
    float m_FallVelocity = 0.0f;
    float m_WorldX = 0.0f;  // Physics world X (fixed, set on gravity enable)
    float m_WorldY = 0.0f;  // Physics world Y (updated each tick while falling)

    // Animation
    int m_CurrentFrame = 0;
    int m_AnimTimer = 0;

    // Sprites
    std::shared_ptr<Util::Image> m_Sprite;
    std::shared_ptr<Util::Image> m_HitSprite;
    std::vector<std::shared_ptr<Util::Image>> m_AnimFrames;

    // Lazy loading support
    bool m_SpriteLoaded = false;
    std::string m_SpritePath;
    std::vector<std::string> m_AnimPaths;
    std::string m_HitSpritePath;

    void SetupSprite();
    void LoadSpriteOnDemand();  // Called on first Render()
};

}  // namespace Mario

#endif  // MARIO_BLOCK_HPP
