/**
 * @file Player.cpp
 * @brief Implementation of Player (View layer).
 *        Renders Mario sprite based on PlayerState Model data.
 *        Handles sprite caching and PTSD coordinate conversion.
 *        PTSD: (0,0) = screen center, +X right, +Y up.
 *        Level: 16 rows, row 0 = top, world Y increases downward.
 * @inheritance Util::GameObject -> Player
 */
#include "Mario/Player/Player.hpp"
#include "Mario/Player/PlayerForm.hpp"

#include <cmath>
#include <fstream>

#include "Mario/Core/SpritePathResolver.hpp"
#include "Util/Logger.hpp"

namespace Mario {

Player::Player(float worldX, float worldY, int startState) {
    m_State.Init(worldX, worldY, startState);

    // Set initial Z-index for player rendering layer
    SetZIndex(GameConfig::Z_PLAYER);

    // Initialize current sprite path
    m_CurrentSpritePath = "";

    // Load initial sprite
    std::string initialPath =
        SpritePathResolver::GetPlayerSpritePath("Idle", startState, 0);

    if (!initialPath.empty()) {
        auto sprite = GetOrLoadSprite(initialPath);
        if (sprite) {
            SetDrawable(sprite);
            m_CurrentSpritePath = initialPath;
        }
    }
}

Player::~Player() = default;


void Player::UpdateView(float cameraOffset) {
    // Check visibility flag (for ending sequences like entering castle)
    if (!m_Visible) {
        SetVisible(false);
        return;  // Skip rendering if invisible
    }

    // Build the sprite path from current state following C# logic:
    // sprite_name = "Mario" + prefix + state + frame
    std::string prefix;
    int frame;
    int spriteState;
    int starState = 0;

    if (m_State.IsDeathAnimActive()) {
        prefix = "Jump";
        frame = 0;
        spriteState = 0;
    } else if (m_State.GetTransitionTimer() > 0) {
        // Alternate sprites every 4 frames during transition
        prefix = "Idle";
        frame = 0;
        bool useBig = (m_State.GetTransitionTimer() / 4) % 2 == 0;
        
        if (m_State.GetPowerState() == PowerState::FIRE && m_State.GetPrevPowerState() == PowerState::BIG) {
            // Transition from Big to Fire: alternate between Big (state 1) and Fire (state 2)
            spriteState = useBig ? 2 : 1;
        } else {
            // Transition between Small and Big/Fire: alternate between Small (state 0) and Big (state 1)
            spriteState = useBig ? 1 : 0;
        }
        starState = 0;
    } else {
        prefix = m_State.GetAnimPrefix();
        frame = m_State.GetAnimFrame();
        
        const auto* form = m_State.GetForm();
        bool isShooting = m_State.IsFireShooting();
        spriteState = form->GetSpriteState(isShooting);
        starState = form->GetStarState(m_State.GetStarTimer(), isShooting);
    }

    // Get the sprite path
    std::string spritePath = SpritePathResolver::GetPlayerSpritePath(
        prefix, spriteState, frame, starState);

    // Only reload sprite if it changed
    if (!spritePath.empty() && spritePath != m_CurrentSpritePath &&
        spritePath.find("MarioIdle0.png") == std::string::npos) {
        m_CurrentSpritePath = spritePath;
        auto sprite = GetOrLoadSprite(spritePath);
        if (sprite) {
            SetDrawable(sprite);
        }
        // If loading fails, keep the previous sprite
    }

    // Convert world coordinates to PTSD screen coordinates
    float playerWidth = static_cast<float>(m_State.GetWidth());
    float playerHeight = static_cast<float>(m_State.GetHeight());
    float roundedOffset = std::round(cameraOffset);

    // X: standard top-left → PTSD centre helper
    float screenX = GameConfig::TopLeftToPTSDX(std::round(m_State.GetX()),
                                               playerWidth, roundedOffset);

    // Y: crouch and transition fix — anchor sprite bottom to hitbox bottom so the sprite
    // never sinks into the floor when Big/Fire Mario crouches or alternates states.
    // The Big/Fire sprite canvas is always 2-tile tall; the Small sprite is 1-tile tall.
    // spriteState == 0 (SMALL) and spriteState == 3 (SMALL_STAR) are both 1-tile tall.
    float spriteHeight = (spriteState == 0 || spriteState == 3)
                             ? static_cast<float>(GameConfig::TILE_SIZE)
                             : static_cast<float>(GameConfig::TILE_SIZE * 2);
    float hitboxBottom = std::round(m_State.GetY()) + playerHeight;
    float worldCY = hitboxBottom - spriteHeight / 2.0f;
    float screenY = GameConfig::WorldToPTSDY(worldCY);

    m_Transform.translation = {screenX, screenY};

    // Flip sprite based on facing direction, maintaining scale
    float absScaleX = GameConfig::DRAW_SCALE;
    m_Transform.scale.y = GameConfig::DRAW_SCALE;

    if (m_State.IsDeathAnimActive()) {
        m_Transform.scale.x = absScaleX;
    } else if (m_State.IsFacingRight()) {
        m_Transform.scale.x = absScaleX;
    } else {
        m_Transform.scale.x = -absScaleX;
    }

    // Handle invincibility flicker effect.
    // Call the PTSD base class directly so that m_Visible (the intentional-hide
    // flag used by castle/pipe sequences) is never modified by the blink cycle.
    // If SetVisible() were called here instead, m_Visible would flip to false
    // on a blink-off frame and the early-return guard at the top of UpdateView
    // would permanently hide Mario on every subsequent frame.
    if (m_State.IsInvincible()) {
        Util::GameObject::SetVisible((m_State.GetInvTimer() % 4) < 2);
    } else {
        Util::GameObject::SetVisible(true);
    }
}

std::shared_ptr<Util::Image> Player::GetOrLoadSprite(const std::string& path) {
    if (path.empty()) {
        LOG_ERROR("GetOrLoadSprite: Received empty path");
        return nullptr;
    }

    // Check cache first
    auto it = m_SpriteCache.find(path);
    if (it != m_SpriteCache.end()) {
        return it->second;
    }

    // Verify file exists before loading
    std::ifstream test(path);
    if (test.good()) {
        try {
            auto sprite = std::make_shared<Util::Image>(path);
            m_SpriteCache[path] = sprite;
            LOG_DEBUG("Loaded sprite: {}", path);
            return sprite;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to create Image from {}: {}", path, e.what());
            return nullptr;
        }
    }

    // File doesn't exist - this is a critical error
    LOG_ERROR("Sprite file not found: {}", path);
    return nullptr;
}

}  // namespace Mario
