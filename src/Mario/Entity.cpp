/**
 * @file Entity.cpp
 * @brief Implementation of Entity View layer.
 *        Handles sprite rendering, direction-based flipping, and
 *        world-to-PTSD coordinate conversion.
 *        Sprite naming: C# uses name + frame (e.g. "Goomba1", "Goomba2").
 * @inheritance Util::GameObject -> Entity
 */
#include "Mario/Entity.hpp"

#include <cmath>
#include <fstream>

#include "Mario/SpritePathResolver.hpp"
#include "Util/Logger.hpp"

namespace Mario {

Entity::Entity(const EntityDef& def, float worldX, float worldY, int direction,
               bool fromBlock, const std::string& levelName)
    : m_Def(def), m_LevelName(levelName) {
    m_State.Init(def.name, worldX, worldY, direction, def.isEnemy,
                 def.isPowerUp, def.isCoin, def.isStatic, def.doesCollide,
                 def.squishable, def.koopaSquash, def.doesJump, def.isBounce,
                 def.scoreWorth, def.isAnimated, def.animFrames, def.animBuffer,
                 def.oneLoop, fromBlock, def.powerUpState);

    SetZIndex(GameConfig::Z_ENTITY);
    UpdateView(0.0f);
}

void Entity::UpdateView(float cameraOffset) {
    if (!m_State.IsActive()) {
        SetVisible(false);
        return;
    }

    // Build sprite path
    std::string spritePath = BuildSpritePath();

    if (spritePath != m_CurrentSpritePath) {
        m_CurrentSpritePath = spritePath;
        auto sprite = GetOrLoadSprite(spritePath);
        if (sprite) {
            SetDrawable(sprite);
            SetVisible(true);

            // Adjust entity size based on sprite dimensions (like C# Entity.cs)
            // If sprite is taller than wide, it's a 2-tile height entity
            glm::vec2 spriteSize = sprite->GetSize();

            // For 8-4: calculate collision box size based on sprite and scaling
            if (m_LevelName == "8-4") {
                float spriteWidth = spriteSize.x;
                if (spriteWidth > 0) {
                    // Determine target width for collision
                    float targetWidth = 32.0f;  // Default for regular enemies
                    if (m_State.GetName() == "Bowser") {
                        targetWidth = 64.0f;  // 2x for Bowser
                    }

                    // Set collision box width
                    m_State.SetSizeX(static_cast<int>(targetWidth));

                    // Adjust height if sprite is taller than wide
                    if (spriteSize.y > spriteSize.x) {
                        // 2-tile height entity
                        float heightScale = spriteSize.y / spriteSize.x;
                        m_State.SetSizeY(
                            static_cast<int>(targetWidth * heightScale));
                        m_State.SetY(m_State.GetY() - GameConfig::TILE_SIZE);
                        LOG_DEBUG("Adjusted {} collision: {}x{}, moved Y up",
                                  m_State.GetName(), m_State.GetWidth(),
                                  m_State.GetHeight());
                    } else {
                        // Regular square/wide enemy
                        m_State.SetSizeY(static_cast<int>(targetWidth));
                    }
                }
            } else if (spriteSize.y > spriteSize.x) {
                // 1-1: If sprite is taller than wide, adjust height
                m_State.SetSizeY(GameConfig::TILE_SIZE * 2);
                m_State.SetY(m_State.GetY() - GameConfig::TILE_SIZE);
            }
        } else {
            SetVisible(false);
        }
    }

    // Convert world to PTSD screen coordinates
    float levelHalfH = GameConfig::LEVEL_HEIGHT_PX / 2.0f;

    float centerX = m_State.GetX() + m_State.GetWidth() / 2.0f;
    float centerY = m_State.GetY() + m_State.GetHeight() / 2.0f;

    float screenX = centerX - cameraOffset - GameConfig::WINDOW_WIDTH / 2.0f;
    float screenY = levelHalfH - centerY + GameConfig::RENDER_Y_OFFSET;

    m_Transform.translation = {screenX, screenY};

    // Calculate scale based on level and sprite size
    float absScaleX = GameConfig::DRAW_SCALE;
    float absScaleY = GameConfig::DRAW_SCALE;

    // For 8-4 level: scale based on actual sprite WIDTH to match 1-1 width
    // Reference: 1-1 standard enemy width (Goomba = 48px, KoopaTroopa = 32px)
    // - Regular 8-4 enemies match 1-1 enemy width
    // - Bowser gets 2x width multiplier
    if (m_LevelName == "8-4" && m_Drawable) {
        glm::vec2 spriteSize = m_Drawable->GetSize();
        float spriteWidth = spriteSize.x;

        if (spriteWidth > 0) {
            // Determine target width based on enemy type
            float targetWidth = 32.0f;  // Default: enemies should be 32px wide

            // Bowser is special: 2x width
            if (m_State.GetName() == "Bowser") {
                targetWidth = 64.0f;  // 2x for Bowser
            }

            // Calculate scale to match target width
            float widthScale = targetWidth / spriteWidth;

            // Apply same scale to both X and Y to maintain aspect ratio
            absScaleX = GameConfig::DRAW_SCALE * widthScale;
            absScaleY = GameConfig::DRAW_SCALE * widthScale;
        }
    }

    // For 1-1 and other levels: use fixed DRAW_SCALE (original behavior)

    // Direction 0=Left (flip), 1=Right (normal)
    if (m_State.GetDirection() == 0) {
        m_Transform.scale.x = -absScaleX;
    } else {
        m_Transform.scale.x = absScaleX;
    }
    m_Transform.scale.y = absScaleY;
}

std::string Entity::BuildSpritePath() const {
    const std::string& name = m_State.GetName();

    // Squished sprite
    if (m_State.IsSquished()) {
        return SpritePathResolver::GetEntitySpritePath(name + "Squish", -1,
                                                       m_LevelName);
    }

    // Special handling: KoopaTroopaShell uses KoopaShell sprite
    // (KoopaTroopaShell is a dynamic entity created when KoopaTroopa is
    // stomped)
    std::string displayName = name;
    if (name == "KoopaTroopaShell") {
        displayName = "KoopaShell";
    }

    // Animated entities: name + frame (1-indexed in C#)
    if (m_Def.isAnimated) {
        int frame = m_State.GetAnimFrame() + 1;  // C# uses 1-indexed frames
        return SpritePathResolver::GetEntitySpritePath(displayName, frame,
                                                       m_LevelName);
    }

    // Static/single-frame entities
    return SpritePathResolver::GetEntitySpritePath(displayName, -1,
                                                   m_LevelName);
}

std::shared_ptr<Util::Image> Entity::GetOrLoadSprite(const std::string& path) {
    auto it = m_SpriteCache.find(path);
    if (it != m_SpriteCache.end()) {
        return it->second;
    }

    std::ifstream test(path);
    if (test.good()) {
        try {
            auto sprite = std::make_shared<Util::Image>(path);
            m_SpriteCache[path] = sprite;
            return sprite;
        } catch (...) {
            LOG_WARN("Failed to create entity sprite: {}", path);
        }
    }

    LOG_WARN("Entity sprite not found: {}", path);
    return nullptr;
}

}  // namespace Mario
