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

    // Z-layer: entities flagged rendersBehindBlocks (PiranhaPlant, Coin) render
    // at Z_BLOCK-1 so they appear behind solid tiles.  EntityFactory sets the
    // flag; Entity.cpp never needs to know about specific EntityType values
    // (OCP / DIP — data-driven, not type-switch).
    if (def.rendersBehindBlocks) {
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

            // Size initialization: run ONLY ONCE per entity lifetime.
            // Running this on every animation frame change causes tall-sprite
            // entities (Princess, etc.) to drift upward every frame.
            if (!m_SizeInitialized) {
                m_SizeInitialized = true;
                InitializeSizeOnce(sprite->GetSize());
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

    // Flip vertically and rotate (spin) if dying in flipped death animation
    if (m_State.IsDeathActive() && !m_State.IsSquished()) {
        m_Transform.scale.y = -absScaleY;
        // Spin by 0.15 radians (approx 8.6 degrees) per frame for a graceful tumble
        m_Transform.rotation += 0.15f;
    } else {
        m_Transform.scale.y = absScaleY;
        m_Transform.rotation = 0.0f; // Reset rotation for normal active states
    }

    // Per-frame X-scale modifier (e.g. coin rotation) — delegated to behavior
    // so Entity.cpp stays free of entity-specific visual logic (OCP).
    if (m_Behavior) {
        m_Transform.scale.x *= m_Behavior->GetVisualScaleXModifier(m_State);
    }
}

void Entity::InitializeSizeOnce(const glm::vec2& spriteSize) {
    if (m_Def.fixedHitboxTiles > 0) {
        // EntityFactory set an explicit tile-size hitbox (e.g. PiranhaPlant=2).
        // Entity.cpp never inspects EntityType here — data-driven (OCP/DIP).
        int sz = m_Def.fixedHitboxTiles * GameConfig::TILE_SIZE;
        m_State.SetSizeX(sz);
        m_State.SetSizeY(sz);
    } else if (m_Def.renderTargetWidth > 0.0f && spriteSize.x > 0.0f) {
        // Width-override entity: EntityFactory set renderTargetWidth (e.g. 8-4
        // enemy scaling).  Height is derived proportionally from the sprite.
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
        // Tall-sprite default: 2-tile height, shift up one tile so the bottom
        // of the sprite aligns with the entity's logical ground position.
        m_State.SetSizeY(GameConfig::TILE_SIZE * 2);
        m_State.SetY(m_State.GetY() - GameConfig::TILE_SIZE);
    }
}

std::string Entity::BuildSpritePath() const {
    const std::string& name = m_State.GetName();

    // Squished or dead Koopa family members use the Shell sprite
    if (m_State.IsSquished() || (m_State.IsDead() && m_State.IsKoopaSquash())) {
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
