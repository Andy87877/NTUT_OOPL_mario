/**
 * @file Player.hpp
 * @brief View layer for Mario in MVC architecture.
 *        Inherits Util::GameObject for PTSD rendering pipeline.
 *        Delegates all game logic to PlayerState (Model).
 * @inheritance Util::GameObject -> Player
 */
#ifndef MARIO_PLAYER_HPP
#define MARIO_PLAYER_HPP

#include <memory>
#include <string>
#include <unordered_map>

#include "Mario/GameConfig.hpp"
#include "Mario/PlayerState.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "pch.hpp"

namespace Mario {

/**
 * The View for Mario. Inherits Util::GameObject for rendering.
 *
 * Responsibilities:
 *   - Render the correct sprite based on PlayerState's animation key
 *   - Cache loaded sprites to avoid re-loading
 *   - Convert between world coords and PTSD screen coords
 *   - Delegate all logic to PlayerState (Model)
 */
class Player : public Util::GameObject {
   public:
    /**
     * Construct the player at a world position.
     * @param worldX Starting X position (pixels)
     * @param worldY Starting Y position (pixels)
     * @param startState Starting power state
     */
    Player(float worldX, float worldY, int startState = 0);

    virtual ~Player() = default;

    /**
     * Update the view: pick the correct sprite and set screen position.
     * Called every frame after PlayerState::Tick().
     * @param cameraOffset Current camera X offset
     */
    void UpdateView(float cameraOffset);

    /**
     * Get the underlying Model.
     */
    PlayerState& GetState() { return m_State; }
    const PlayerState& GetState() const { return m_State; }

    /**
     * Get world-space hitbox (for collision).
     */
    AABB GetHitbox() const { return m_State.GetHitbox(); }

    /**
     * Get world X/Y (passthrough to Model).
     */
    float GetWorldX() const { return m_State.GetX(); }
    float GetWorldY() const { return m_State.GetY(); }

    /**
     * Set visibility for special sequences (e.g., entering castle).
     * C# reference: Form1.cs line 1221 setRecBox(0, 0) makes Mario invisible.
     */
    void SetVisible(bool visible) { m_Visible = visible; }
    bool IsVisible() const { return m_Visible; }

   private:
    /**
     * Load or retrieve a cached sprite by path.
     */
    std::shared_ptr<Util::Image> GetOrLoadSprite(const std::string& path);

    /**
     * Build the sprite path from current player state.
     */
    std::string BuildSpritePath() const;

    PlayerState m_State;  // The Model

    // Sprite cache: path -> Image
    std::unordered_map<std::string, std::shared_ptr<Util::Image>> m_SpriteCache;

    // Current sprite path for change detection
    std::string m_CurrentSpritePath;

    // Visibility flag (used during ending sequences)
    bool m_Visible = true;
};

}  // namespace Mario

#endif  // MARIO_PLAYER_HPP
