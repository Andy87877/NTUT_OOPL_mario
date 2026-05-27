/**
 * @file Entity.cpp
 * @brief Implementation of Entity View layer.
 *        Handles sprite rendering, direction-based flipping, and
 *        world-to-PTSD coordinate conversion.
 *        Sprite naming: C# uses name + frame (e.g. "Goomba1", "Goomba2").
 * @inheritance Util::GameObject -> Entity
 */
#include "Mario/Level/Entity.hpp"

#include <cmath>
#include <fstream>

#include "Mario/Core/SpritePathResolver.hpp"
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

    if (def.type == EntityType::PIRANHA_PLANT || def.type == EntityType::COIN) {
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

                if (m_Def.type == EntityType::PIRANHA_PLANT) {
                    // PiranhaPlant: always 2-tile hitbox.
                    // PiranhaPlantBehavior manages Y position — no shift here.
                    m_State.SetSizeX(GameConfig::TILE_SIZE * 2);
                    m_State.SetSizeY(GameConfig::TILE_SIZE * 2);
                } else if (m_Def.renderTargetWidth > 0.0f &&
                           spriteSize.x > 0.0f) {
                    // Width-override entity: EntityFactory set
                    // renderTargetWidth based on level context (e.g. 8-4 enemy
                    // scaling).
                    float tw = m_Def.renderTargetWidth;
                    m_State.SetSizeX(static_cast<int>(tw));
                    if (spriteSize.y > spriteSize.x) {
                        float heightScale = spriteSize.y / spriteSize.x;
                        m_State.SetSizeY(static_cast<int>(tw * heightScale));
                        m_State.SetY(m_State.GetY() - GameConfig::TILE_SIZE);
                    } else {
                        m_State.SetSizeY(static_cast<int>(tw));
                    }
                } else if (spriteSize.y > spriteSize.x) {
                    // Tall-sprite default: 2-tile height, shift up one tile.
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

    // Calculate scale: if EntityFactory set a renderTargetWidth override, use
    // it to derive a per-entity widthScale.  Otherwise fall back to the global
    // DRAW_SCALE so existing non-8-4 entities are unchanged.
    float absScaleX = GameConfig::DRAW_SCALE;
    float absScaleY = GameConfig::DRAW_SCALE;

    if (m_Def.renderTargetWidth > 0.0f && m_Drawable) {
        glm::vec2 spriteSize = m_Drawable->GetSize();
        if (spriteSize.x > 0.0f) {
            float widthScale = m_Def.renderTargetWidth / spriteSize.x;
            absScaleX = GameConfig::DRAW_SCALE * widthScale;
            absScaleY = GameConfig::DRAW_SCALE * widthScale;
        }
    }

    // TopLeftToPTSD* handles centering + pixel alignment internally
    float screenX = GameConfig::TopLeftToPTSDX(
        std::round(m_State.GetX()), entityWidth, std::round(cameraOffset));
    float screenY =
        GameConfig::TopLeftToPTSDY(std::round(m_State.GetY()), entityHeight);

    // Anchored bottom-alignment correction for width-override entities:
    // Ensures the bottom of the drawn sprite (spriteSize.y * absScaleY)
    // aligns exactly with the bottom of the logical hitbox (entityHeight),
    // preventing the sprite from sinking into solid blocks when scaled.
    if (m_Def.renderTargetWidth > 0.0f && m_Drawable) {
        glm::vec2 spriteSize = m_Drawable->GetSize();
        if (spriteSize.y > 0.0f) {
            float drawnHeight = spriteSize.y * absScaleY;
            screenY += (drawnHeight - entityHeight) * 0.5f;
        }
    }

    if (m_Behavior) {
        screenY += m_Behavior->GetVisualYOffset(m_LevelName);
    }

    m_Transform.translation = {screenX, screenY};

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

AABB Entity::GetHitbox() const {
    if (m_Behavior) {
        return m_Behavior->GetHitbox(m_State);
    }
    return m_State.GetHitbox();
}

}  // namespace Mario
