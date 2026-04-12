/**
 * @file Block.cpp
 * @brief Implementation of Block terrain tile.
 *        Converts grid coordinates to PTSD screen coordinates.
 *        PTSD: (0,0) = screen center, +X right, +Y up.
 *        Level: row 0 = top, row 15 = bottom, 16 rows x 32px = 512px.
 * @inheritance Util::GameObject -> Block
 */
#include "Mario/Block.hpp"

#include <cmath>
#include <cstdio>
#include <fstream>

#include "Mario/AudioManager.hpp"
#include "Mario/SpritePathResolver.hpp"
#include "Util/Logger.hpp"

namespace Mario {

// Convert grid (col, row) to PTSD screen coordinates
// Grid origin: top-left, Y increases downward
// PTSD origin: screen center, Y increases upward
static void GridToScreen(int gridX, int gridY, float cameraOffset,
                         float& screenX, float& screenY) {
    // X: tile center in world, minus camera, minus half-window to convert
    //    from world-left-origin to PTSD-center-origin
    screenX = static_cast<float>(gridX * GameConfig::TILE_SIZE) +
              GameConfig::TILE_SIZE / 2.0f - cameraOffset -
              GameConfig::WINDOW_WIDTH / 2.0f;
    // Y: flip vertical. Level is 16 rows (512px).
    // PTSD Y=0 is screen center. Level center Y should be at screen center.
    float levelHalfH = GameConfig::LEVEL_HEIGHT_PX / 2.0f;
    screenY = levelHalfH -
              (static_cast<float>(gridY * GameConfig::TILE_SIZE) +
               GameConfig::TILE_SIZE / 2.0f) +
              GameConfig::RENDER_Y_OFFSET;
}

Block::Block(int blockID, int gridX, int gridY, const BlockDef& def)
    : m_BlockID(blockID), m_GridX(gridX), m_GridY(gridY), m_Def(def) {
    m_Solid = def.solid;

    // Special case: multi-hit coin block (ID 5 in reference)
    if (blockID == 5) {
        m_HP = 5;
    }

    // Set initial screen position (camera offset = 0)
    float sx, sy;
    GridToScreen(gridX, gridY, 0.0f, sx, sy);
    m_Transform.translation = {sx, sy};
    // Initialize scale factor based on GameConfig
    // Apply additional scaling for 8-4 castle sprites (original sprites are
    // smaller)
    float scaleX = GameConfig::DRAW_SCALE;
    float scaleY = GameConfig::DRAW_SCALE;
    if (blockID >= 801 && blockID <= 904) {
        scaleX *= 2.0f;  // Enlarge 8-4 castle blocks by 2.0x
        scaleY *= 2.0f;
    }
    m_Transform.scale = {scaleX, scaleY};

    // Set Z-index based on whether it's background or foreground
    SetZIndex(def.background ? GameConfig::Z_BACKGROUND : GameConfig::Z_BLOCK);

    // Setup sprite
    SetupSprite();
}

void Block::SetupSprite() {
    if (m_Def.name.empty()) {
        SetVisible(false);
        return;
    }

    // Lazy-load: Just collect paths, don't load images yet
    try {
        // Special handling for 8-4 castle sprites (IDs 801-904)
        if (m_BlockID >= 801 && m_BlockID <= 904) {
            m_SpritePath =
                SpritePathResolver::GetCastleSpritePathByID(m_BlockID);
            if (m_SpritePath.empty()) {
                SetVisible(false);
                return;
            }
        } else {
            // Standard block sprites: only resolve path, don't load
            m_SpritePath =
                SpritePathResolver::GetBlockSpritePath(m_Def.name, 0);
        }

        // Verify file exists (without loading it yet)
        std::ifstream test(m_SpritePath);
        if (!test.good()) {
            SetVisible(false);
            return;
        }

        // For animated blocks, collect all frame paths (don't load yet)
        if (m_Def.animated) {
            for (int i = 0; i <= m_Def.animationFrames; ++i) {
                std::string animPath =
                    SpritePathResolver::GetBlockSpritePath(m_Def.name, i);
                std::ifstream testAnim(animPath);
                if (testAnim.good()) {
                    m_AnimPaths.push_back(animPath);
                } else {
                    m_AnimPaths.push_back(m_SpritePath);  // fallback to base
                }
            }
        }

        // Collect hit sprite path if available
        if (!m_Def.hitSpriteName.empty()) {
            m_HitSpritePath =
                SpritePathResolver::GetBlockSpritePath(m_Def.hitSpriteName, 0);
            std::ifstream test(m_HitSpritePath);
            if (!test.good()) {
                m_HitSpritePath.clear();
            }
        }

        // Mark as ready but not yet loaded
        m_SpriteLoaded = false;
        SetVisible(true);

    } catch (...) {
        SetVisible(false);
    }
}

void Block::LoadSpriteOnDemand() {
    if (m_SpriteLoaded) return;
    m_SpriteLoaded = true;

    if (m_SpritePath.empty()) {
        SetVisible(false);
        return;
    }

    // Now actually load the sprite images
    try {
        m_Sprite = std::make_shared<Util::Image>(m_SpritePath);
        SetDrawable(m_Sprite);

        // Load animated frames if any
        if (!m_AnimPaths.empty()) {
            for (const auto& path : m_AnimPaths) {
                m_AnimFrames.push_back(std::make_shared<Util::Image>(path));
            }
        }

        // Load hit sprite if available
        if (!m_HitSpritePath.empty()) {
            m_HitSprite = std::make_shared<Util::Image>(m_HitSpritePath);
        }

    } catch (...) {
        SetVisible(false);
    }

    // Invisible question blocks start hidden
    if (m_Def.name == "InvisQuestionBlock") {
        SetVisible(false);
    }
}

void Block::Update(float cameraOffset) {
    // Load sprite on first update (lazy loading)
    // This spreads sprite loading across frames instead of all at level load
    if (!m_SpriteLoaded) {
        LoadSpriteOnDemand();
    }

    // Convert grid position to screen position with camera offset
    float sx, sy;
    GridToScreen(m_GridX, m_GridY, cameraOffset, sx, sy);

    // Apply bounce effect (upward in screen = positive Y)
    sy += m_BounceHeight;

    m_Transform.translation = {sx, sy};

    // Bounce animation decay
    if (m_BounceHeight > 0.0f) {
        m_BounceHeight -= 0.45f;
        if (m_BounceHeight < 0.0f) {
            m_BounceHeight = 0.0f;
        }
    }

    // Block animation (e.g., question block shimmer)
    if (m_Def.animated && !m_IsHit && !m_AnimFrames.empty()) {
        m_AnimTimer++;
        if (m_AnimTimer >= 10) {  // Change frame every 10 ticks
            m_AnimTimer = 0;
            m_CurrentFrame = (m_CurrentFrame + 1) % m_AnimFrames.size();
            m_Sprite = m_AnimFrames[m_CurrentFrame];
            SetDrawable(m_Sprite);
        }
    }
}

void Block::OnHit(int playerState) {
    m_HP--;
    if (m_HP <= 0) {
        m_IsHit = true;

        if (m_HitSprite) {
            // Switch to hit sprite (e.g., question block -> empty block)
            SetDrawable(m_HitSprite);
        } else if (m_Def.breakable && playerState > 0) {
            // Big/Fire Mario breaks bricks
            Break();
            return;
        }

        // Make invisible question blocks visible after being hit
        if (m_Def.name == "InvisQuestionBlock") {
            SetVisible(true);
        }

        // Play bump sound when HP reduces to 0
        Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Bump);
    }

    Bounce();
}

void Block::Bounce() {
    m_BounceHeight = static_cast<float>(GameConfig::TILE_SIZE) * 0.5f;
}

void Block::Break() {
    m_Solid = false;
    SetVisible(false);
    Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Break);
}

float Block::GetWorldX() const {
    return static_cast<float>(m_GridX * GameConfig::TILE_SIZE);
}

float Block::GetWorldY() const {
    return static_cast<float>(m_GridY * GameConfig::TILE_SIZE);
}

AABB Block::GetAABB() const {
    // AABB uses grid-based coordinates (for collision, not screen)
    float x = GetWorldX();
    float y = GetWorldY();
    return AABB::FromPosSize(x, y, static_cast<float>(GameConfig::TILE_SIZE),
                             static_cast<float>(GameConfig::TILE_SIZE));
}

std::string Block::GetSpawnContents(int playerState) const {
    if (!m_Def.isContainer) return "";

    // If player already big/fire and content is mushroom, give fire flower
    if (m_Def.contents == "Mushroom" && playerState > 0 && playerState != 3) {
        return "FireFlower";
    }
    return m_Def.contents;
}

}  // namespace Mario
