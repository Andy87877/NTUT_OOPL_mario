/**
 * @file Player.cpp
 * @brief Implementation of Player (View layer).
 *        Renders Mario sprite based on PlayerState Model data.
 *        Handles sprite caching and PTSD coordinate conversion.
 * @inheritance Util::GameObject -> Player
 */
#include "Mario/Player.hpp"
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
    if (!m_State.IsDead() || m_State.GetY() < GameConfig::WINDOW_HEIGHT + 100) {
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
    }

    // Convert world coordinates to PTSD screen coordinates
    // PTSD: (0,0) = center of screen, +Y = up
    float halfH = static_cast<float>(GameConfig::VIEWPORT_TILES_Y *
                  GameConfig::TILE_SIZE) / 2.0f;

    float screenX = m_State.GetX() + GameConfig::TILE_SIZE / 2.0f - cameraOffset;
    float screenY = halfH - (m_State.GetY() + m_State.GetHeight() / 2.0f);

    m_Transform.translation = {screenX, screenY};

    // Flip sprite based on facing direction
    // PTSD uses negative scale.x to flip horizontally
    float absScaleX = std::abs(m_Transform.scale.x);
    if (absScaleX == 0.0f) absScaleX = 1.0f;

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
    int state = m_State.GetState();
    int frame = m_State.GetAnimFrame();

    // Map power state to sprite state number
    // C# naming: "Mario" + prefix + state + frame
    // e.g. "MarioIdle.png" (small idle), "MarioIdle10.png" (big idle)
    int spriteState = 0;
    switch (m_State.GetPowerState()) {
        case PowerState::SMALL:
        case PowerState::SMALL_STAR:
            spriteState = 0;
            break;
        case PowerState::BIG:
        case PowerState::BIG_STAR:
            spriteState = 1;
            break;
        case PowerState::FIRE:
            spriteState = 2;
            break;
        default:
            spriteState = 0;
            break;
    }

    // Star states use cycling color: 300-303, 400-403
    if (m_State.GetPowerState() == PowerState::SMALL_STAR) {
        spriteState = 3;
    } else if (m_State.GetPowerState() == PowerState::BIG_STAR) {
        spriteState = 4;
    }

    return SpritePathResolver::GetPlayerSpritePath(prefix, spriteState, frame);
}

std::shared_ptr<Util::Image> Player::GetOrLoadSprite(const std::string& path) {
    auto it = m_SpriteCache.find(path);
    if (it != m_SpriteCache.end()) {
        return it->second;
    }

    try {
        auto sprite = std::make_shared<Util::Image>(path);
        m_SpriteCache[path] = sprite;
        return sprite;
    } catch (...) {
        // Fallback: try without frame number
        try {
            // Extract base name and try without the trailing digit
            std::string fallback = SpritePathResolver::GetSpritePath("MarioIdle");
            auto sprite = std::make_shared<Util::Image>(fallback);
            m_SpriteCache[path] = sprite;
            return sprite;
        } catch (...) {
            LOG_ERROR("Failed to load player sprite: {}", path);
            return nullptr;
        }
    }
}

} // namespace Mario
