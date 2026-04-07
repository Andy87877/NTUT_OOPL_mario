/**
 * @file Entity.cpp
 * @brief Implementation of Entity View layer.
 *        Handles sprite rendering, direction-based flipping, and
 *        world-to-PTSD coordinate conversion.
 *        Sprite naming: C# uses name + frame (e.g. "Goomba1", "Goomba2").
 * @inheritance Util::GameObject -> Entity
 */
#include "Mario/Entity.hpp"
#include "Mario/SpritePathResolver.hpp"

#include "Util/Logger.hpp"

#include <cmath>
#include <fstream>

namespace Mario {

Entity::Entity(const EntityDef& def, float worldX, float worldY,
               int direction, bool fromBlock) : m_Def(def) {

    m_State.Init(
        def.name, worldX, worldY, direction,
        def.isEnemy, def.isPowerUp, def.isCoin,
        def.isStatic, def.doesCollide, def.squishable,
        def.koopaSquash, def.doesJump, def.isBounce,
        def.scoreWorth, def.isAnimated, def.animFrames,
        def.animBuffer, def.oneLoop, fromBlock,
        def.powerUpState
    );

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
        } else {
            SetVisible(false);
        }
    }

    // Convert world to PTSD screen coordinates
    float levelHalfH = GameConfig::LEVEL_HEIGHT_PX / 2.0f;

    float centerX = m_State.GetX() + m_State.GetWidth() / 2.0f;
    float centerY = m_State.GetY() + m_State.GetHeight() / 2.0f;

    float screenX = centerX - cameraOffset - GameConfig::WINDOW_WIDTH / 2.0f;
    float screenY = levelHalfH - centerY;

    m_Transform.translation = {screenX, screenY};

    // Flip based on direction (left-facing enemies should be flipped)
    float absScaleX = std::abs(m_Transform.scale.x);
    if (absScaleX < 0.01f) absScaleX = 1.0f;

    // Direction 0=Left (flip), 1=Right (normal)
    if (m_State.GetDirection() == 0) {
        m_Transform.scale.x = -absScaleX;
    } else {
        m_Transform.scale.x = absScaleX;
    }
}

std::string Entity::BuildSpritePath() const {
    const std::string& name = m_State.GetName();

    // Squished sprite
    if (m_State.IsSquished()) {
        return SpritePathResolver::GetEntitySpritePath(name + "Squish", -1);
    }

    // Animated entities: name + frame (1-indexed in C#)
    if (m_Def.isAnimated) {
        int frame = m_State.GetAnimFrame() + 1; // C# uses 1-indexed frames
        return SpritePathResolver::GetEntitySpritePath(name, frame);
    }

    // Static/single-frame entities
    return SpritePathResolver::GetEntitySpritePath(name, -1);
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

} // namespace Mario
