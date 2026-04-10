/**
 * @file Player.cpp
 * @brief Implementation of Player (View layer).
 *        Renders Mario sprite based on PlayerState Model data.
 *        Handles sprite caching and PTSD coordinate conversion.
 *        PTSD: (0,0) = screen center, +X right, +Y up.
 *        Level: 16 rows, row 0 = top, world Y increases downward.
 * @inheritance Util::GameObject -> Player
 */
#include "Mario/Player.hpp"

#include <cmath>
#include <fstream>

#include "Mario/SpritePathResolver.hpp"
#include "Util/Logger.hpp"

namespace Mario {

Player::Player(float worldX, float worldY, int startState) {
    m_State.Init(worldX, worldY, startState);

    // Set initial Z-index for player rendering layer
    SetZIndex(GameConfig::Z_PLAYER);

    // Load initial sprite
    UpdateView(0.0f);
}

void Player::UpdateView(float cameraOffset) {
    // Build the sprite path from current state
    std::string spritePath = BuildSpritePath();

    // Only reload sprite if it changed
    if (spritePath != m_CurrentSpritePath) {
        m_CurrentSpritePath = spritePath;
        auto sprite = GetOrLoadSprite(spritePath);
        if (sprite) {
            SetDrawable(sprite);
        }
    }

    // Convert world coordinates to PTSD screen coordinates
    // World: (0,0) top-left, Y increases downward
    // PTSD:  (0,0) screen center, Y increases upward
    float levelHalfH = GameConfig::LEVEL_HEIGHT_PX / 2.0f;

    // Player world position is top-left of the player sprite
    // PTSD needs center of the sprite
    float playerWidth = static_cast<float>(m_State.GetWidth());
    float playerHeight = static_cast<float>(m_State.GetHeight());
    float playerCenterX = m_State.GetX() + playerWidth / 2.0f;
    float playerCenterY = m_State.GetY() + playerHeight / 2.0f;

    // Subtract half window width to convert from world-left-origin to
    // PTSD-center-origin
    float screenX =
        playerCenterX - cameraOffset - GameConfig::WINDOW_WIDTH / 2.0f;
    float screenY = levelHalfH - playerCenterY + GameConfig::RENDER_Y_OFFSET;

    m_Transform.translation = {screenX, screenY};

    // Flip sprite based on facing direction, maintaining scale
    float absScaleX = GameConfig::DRAW_SCALE;
    m_Transform.scale.y = GameConfig::DRAW_SCALE;

    if (m_State.IsFacingRight()) {
        m_Transform.scale.x = absScaleX;
    } else {
        m_Transform.scale.x = -absScaleX;
    }

    // Handle invincibility flicker effect
    if (m_State.IsInvincible()) {
        SetVisible((m_State.GetInvTimer() % 4) < 2);
    } else {
        SetVisible(true);
    }
}

std::string Player::BuildSpritePath() const {
    std::string prefix = m_State.GetAnimPrefix();
    int frame = m_State.GetAnimFrame();

    // Map power state to C# state number
    int spriteState = 0;
    switch (m_State.GetPowerState()) {
        case PowerState::SMALL:
            spriteState = 0;
            break;
        case PowerState::BIG:
            spriteState = 1;
            break;
        case PowerState::FIRE:
            spriteState = 2;
            break;
        case PowerState::SMALL_STAR:
            spriteState = 3;
            break;
        case PowerState::BIG_STAR:
            spriteState = 4;
            break;
        default:
            spriteState = 0;
            break;
    }

    // C# Walk animation uses frames 1, 2, 3 (not 0, 1, 2)
    int spriteFrame = frame;
    if (prefix == "Right") {
        spriteFrame = frame + 1;  // C# starts walk frames at 1
    }

    return SpritePathResolver::GetPlayerSpritePath(prefix, spriteState,
                                                   spriteFrame);
}

std::shared_ptr<Util::Image> Player::GetOrLoadSprite(const std::string& path) {
    auto it = m_SpriteCache.find(path);
    if (it != m_SpriteCache.end()) {
        return it->second;
    }

    // Verify file exists first to avoid PTSD checkerboard
    std::ifstream test(path);
    if (test.good()) {
        try {
            auto sprite = std::make_shared<Util::Image>(path);
            m_SpriteCache[path] = sprite;
            return sprite;
        } catch (...) {
            LOG_WARN("Failed to create Image from: {}", path);
        }
    }

    // Fallback to basic idle sprite
    LOG_WARN("Player sprite not found: {}, trying fallback", path);
    std::string fallback =
        SpritePathResolver::GetPlayerSpritePath("Idle", 0, 0);
    std::ifstream test2(fallback);
    if (test2.good()) {
        try {
            auto sprite = std::make_shared<Util::Image>(fallback);
            m_SpriteCache[path] = sprite;  // Cache fallback under original key
            return sprite;
        } catch (...) {
            LOG_ERROR("Failed to load fallback sprite: {}", fallback);
        }
    }

    LOG_ERROR("No valid player sprite found!");
    return nullptr;
}

}  // namespace Mario
