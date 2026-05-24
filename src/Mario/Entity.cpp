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

    if (def.name == "PiranhaPlant" || def.type == EntityType::COIN) {
        SetZIndex(GameConfig::Z_BLOCK - 1.0f);
    } else {
        SetZIndex(GameConfig::Z_ENTITY);
    }
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
                        } else if (m_State.GetName() == "Bowser_fire") {
                            targetWidth = 48.0f;
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
                } else if (m_State.GetName() == "PiranhaPlant") {
                    // PiranhaPlant: always 2 tiles wide × 2 tiles tall
                    // (same collision box as 8-4 path, but without Y shift
                    // since PiranhaPlantBehavior manages the Y position)
                    m_State.SetSizeX(GameConfig::TILE_SIZE * 2);
                    m_State.SetSizeY(GameConfig::TILE_SIZE * 2);
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

    // Entity is not hidden → ensure it's visible (SetVisible(true) is only
    // called inside the sprite-change block above, so an entity that was hidden
    // and then un-hidden without changing its animation frame would stay
    // invisible. Always sync visibility here after the sprite-path logic.
    if (m_Drawable) {
        SetVisible(true);
    }

    // Convert world to PTSD screen coordinates using unified helpers
    float entityWidth = static_cast<float>(m_State.GetWidth());
    float entityHeight = static_cast<float>(m_State.GetHeight());

    // Globally round the cameraOffset and world coordinates for perfect integer
    // pixel alignment
    float roundedOffset = std::round(cameraOffset);
    float worldCX = std::round(m_State.GetX()) + entityWidth / 2.0f;
    float worldCY = std::round(m_State.GetY()) + entityHeight / 2.0f;

    float screenX = GameConfig::WorldToPTSDX(worldCX, roundedOffset);
    float screenY = GameConfig::WorldToPTSDY(worldCY);

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
            } else if (m_State.GetName() == "Bowser_fire") {
                targetWidth = 48.0f;  // 1.5x larger than default 32px
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
    } else if (m_State.GetName() == "PiranhaPlant" && m_Drawable) {
        // PiranhaPlant in non-8-4 levels: scale to 2 tiles width (90px)
        // Uses the same targetWidth=64 formula as the 8-4 path:
        //   screen_width = DRAW_SCALE * (64 / spriteWidth) * spriteWidth = 90px
        glm::vec2 spriteSize = m_Drawable->GetSize();
        if (spriteSize.x > 0) {
            float widthScale = 64.0f / spriteSize.x;
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

    // Procedural animation for coins to simulate rotation (since they fall back
    // to a single static image file on disk e.g. UnderCoin.png or Coin.png)
    if (m_State.IsCoin() && !m_State.IsHidden() && m_State.IsActive()) {
        int frame = m_State.GetAnimFrame();
        // Cycle of 4 animation frames:
        // frame 0: full width (1.0)
        // frame 1: medium width (0.6)
        // frame 2: thin line width (0.15)
        // frame 3: medium width (0.6)
        float scaleXModifier = 1.0f;
        if (frame == 1 || frame == 3) {
            scaleXModifier = 0.6f;
        } else if (frame == 2) {
            scaleXModifier = 0.15f;
        }
        m_Transform.scale.x *= scaleXModifier;
    }
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
    static std::unordered_map<std::string, std::shared_ptr<Util::Image>>
        s_EntitySpriteCache;
    auto it = s_EntitySpriteCache.find(path);
    if (it != s_EntitySpriteCache.end()) {
        return it->second;
    }

    std::ifstream test(path);
    if (test.good()) {
        try {
            auto sprite = std::make_shared<Util::Image>(path);
            s_EntitySpriteCache[path] = sprite;
            return sprite;
        } catch (...) {
            LOG_WARN("Failed to create entity sprite: {}", path);
        }
    }

    LOG_WARN("Entity sprite not found: {}", path);
    return nullptr;
}

}  // namespace Mario
