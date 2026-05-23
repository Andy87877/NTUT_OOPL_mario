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
    // Convert grid coordinates to world center coordinates of the block
    float worldCX = static_cast<float>(gridX * GameConfig::TILE_SIZE) + GameConfig::TILE_SIZE / 2.0f;
    float worldCY = static_cast<float>(gridY * GameConfig::TILE_SIZE) + GameConfig::TILE_SIZE / 2.0f;

    // Globally round the cameraOffset to avoid subpixel gaps between adjacent blocks
    float roundedOffset = std::round(cameraOffset);
    screenX = GameConfig::WorldToPTSDX(worldCX, roundedOffset);
    screenY = GameConfig::WorldToPTSDY(worldCY);
}

Block::Block(int blockID, int gridX, int gridY, const BlockDef& def,
             const std::string& levelName)
    : m_BlockID(blockID),
      m_GridX(gridX),
      m_GridY(gridY),
      m_Def(def),
      m_LevelName(levelName) {
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
    if ((blockID >= 801 && blockID <= 904) || blockID == 905) {
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
        if ((m_BlockID >= 801 && m_BlockID <= 904) || m_BlockID == 905) {
            m_SpritePath =
                SpritePathResolver::GetCastleSpritePathByID(m_BlockID);
            if (m_SpritePath.empty()) {
                SetVisible(false);
                return;
            }
        } else {
            // Standard block sprites: check level-specific dir first, then root
            m_SpritePath = SpritePathResolver::GetBlockSpritePath(m_Def.name, 0,
                                                                  m_LevelName);
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
                std::string animPath = SpritePathResolver::GetBlockSpritePath(
                    m_Def.name, i, m_LevelName);
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
            m_HitSpritePath = SpritePathResolver::GetBlockSpritePath(
                m_Def.hitSpriteName, 0, m_LevelName);
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

// Static cache helper to avoid duplicate disk loads for identical blocks
static std::shared_ptr<Util::Image> GetOrLoadBlockSprite(const std::string& path) {
    static std::unordered_map<std::string, std::shared_ptr<Util::Image>> s_BlockSpriteCache;
    if (path.empty()) return nullptr;
    auto it = s_BlockSpriteCache.find(path);
    if (it != s_BlockSpriteCache.end()) {
        return it->second;
    }
    std::ifstream test(path);
    if (test.good()) {
        try {
            auto img = std::make_shared<Util::Image>(path);
            s_BlockSpriteCache[path] = img;
            return img;
        } catch (...) {
            LOG_WARN("Failed to load block sprite: {}", path);
        }
    }
    return nullptr;
}

void Block::LoadSpriteOnDemand() {
    if (m_SpriteLoaded) return;
    m_SpriteLoaded = true;

    if (m_SpritePath.empty()) {
        SetVisible(false);
        return;
    }

    // Now actually load the sprite images using the cache
    try {
        m_Sprite = GetOrLoadBlockSprite(m_SpritePath);
        if (m_Sprite) {
            SetDrawable(m_Sprite);
        } else {
            SetVisible(false);
            return;
        }

        // Load animated frames if any
        if (!m_AnimPaths.empty()) {
            for (const auto& path : m_AnimPaths) {
                auto frame = GetOrLoadBlockSprite(path);
                if (frame) {
                    m_AnimFrames.push_back(frame);
                }
            }
        }

        // Load hit sprite if available
        if (!m_HitSpritePath.empty()) {
            m_HitSprite = GetOrLoadBlockSprite(m_HitSpritePath);
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

    // --- Gravity physics: bridge collapse ---
    // When SetGravity(true) is called, block enters free-fall using stored
    // world coordinates instead of the fixed grid position.
    if (m_HasGravity) {
        m_FallVelocity += 1.5f;  // Gravity acceleration per tick
        m_WorldY += m_FallVelocity;

        // Hide and deactivate once the block falls below the level boundary
        if (m_WorldY > GameConfig::LEVEL_HEIGHT_PX + 200.0f) {
            SetVisible(false);
            return;
        }

        // Use physics world coordinates for screen position, aligning edges to integers
        float roundedOffset = std::round(cameraOffset);
        float worldCX = std::round(m_WorldX) + GameConfig::TILE_SIZE / 2.0f;
        float worldCY = std::round(m_WorldY) + GameConfig::TILE_SIZE / 2.0f;

        float sx = GameConfig::WorldToPTSDX(worldCX, roundedOffset);
        float sy = GameConfig::WorldToPTSDY(worldCY);

        m_Transform.translation = {sx, sy};
        return;  // Skip grid-based positioning below
    }

    // --- Standard grid-based positioning ---
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

    // Only animate bounce for blocks that have the bounceBack flag set.
    // Castle/hard blocks (bounceBack=false) still play the bump sound but
    // do NOT visually bounce — matching NES behavior where hitting a hard
    // stone block gives a "bonk" sound without any block movement.
    if (m_Def.bounceBack) {
        Bounce();
    }
}

void Block::Bounce() {
    m_BounceHeight = static_cast<float>(GameConfig::TILE_SIZE) * 0.5f;
}

void Block::Break() {
    m_Solid = false;
    m_JustBroken = true;
    SetVisible(false);
    Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Break);
}

float Block::GetWorldX() const {
    return m_HasGravity ? m_WorldX
                        : static_cast<float>(m_GridX * GameConfig::TILE_SIZE);
}

float Block::GetWorldY() const {
    return m_HasGravity ? m_WorldY
                        : static_cast<float>(m_GridY * GameConfig::TILE_SIZE);
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
