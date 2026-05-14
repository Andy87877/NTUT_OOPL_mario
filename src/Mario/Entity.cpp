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

    // PiranhaPlant (and any other behavior that calls SetHidden) hides the
    // entity while it is inside the pipe without deactivating it.
    if (m_State.IsHidden()) {
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

            // Size and Y-offset initialization: run ONLY ONCE per entity
            // lifetime (first sprite load). Running this on every animation
            // frame change causes entities with tall sprites (Princess, etc.)
            // to drift upward by TILE_SIZE every frame — a critical bug.
            if (!m_SizeInitialized) {
                m_SizeInitialized = true;

                glm::vec2 spriteSize = sprite->GetSize();

                // For 8-4: set collision box from sprite dimensions
                if (m_LevelName == "8-4") {
                    float spriteWidth = spriteSize.x;
                    if (spriteWidth > 0) {
                        float targetWidth = 32.0f;
                        if (m_State.GetName() == "Bowser") {
                            targetWidth = 64.0f;
                        } else if (m_State.GetName() == "PiranhaPlant") {
                            // 2-tile-wide pipe: collision box = 2 * TILE_SIZE
                            targetWidth =
                                static_cast<float>(GameConfig::TILE_SIZE * 2);
                        }
                        m_State.SetSizeX(static_cast<int>(targetWidth));

                        if (spriteSize.y > spriteSize.x) {
                            float heightScale = spriteSize.y / spriteSize.x;
                            m_State.SetSizeY(
                                static_cast<int>(targetWidth * heightScale));
                            // Shift entity up by one tile so its BOTTOM aligns
                            // with its spawn row (same as C# Entity.cs init)
                            m_State.SetY(m_State.GetY() -
                                         GameConfig::TILE_SIZE);
                        } else {
                            m_State.SetSizeY(static_cast<int>(targetWidth));
                        }
                    }
                } else if (spriteSize.y > spriteSize.x) {
                    // Other levels: tall sprite → 2-tile height
                    m_State.SetSizeY(GameConfig::TILE_SIZE * 2);
                    m_State.SetY(m_State.GetY() - GameConfig::TILE_SIZE);
                }
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

            if (m_State.GetName() == "Bowser") {
                targetWidth = 64.0f;  // 2x tile width for Bowser
            } else if (m_State.GetName() == "PiranhaPlant") {
                // targetWidth drives the scale formula:
                //   screen_width = DRAW_SCALE * targetWidth
                // We want screen_width = 2 * TILE_SIZE = 90px
                //   targetWidth = 90 / DRAW_SCALE = 90 / (45/32) = 64
                targetWidth = 64.0f;  // renders as 90px = 2 tiles = full pipe
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
        if (m_State.IsKoopaSquash()) {
            // Koopa/ParaKoopa shells use KoopaShell sprite
            return SpritePathResolver::GetEntitySpritePath("KoopaShell", -1,
                                                           m_LevelName);
        }
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
