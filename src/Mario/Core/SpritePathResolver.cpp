/**
 * @file SpritePathResolver.cpp
 * @brief Implementation of sprite path resolution using Strategy Pattern.
 *        Decouples different resolution logic into specialized Strategy
 * classes.
 * @inheritance IBlockResolver -> DefaultBlockResolver,
 *              IPlayerResolver -> DefaultPlayerResolver,
 *              IEntityResolver -> DefaultEntityResolver,
 *              ICastleResolver -> DefaultCastleResolver
 */

#include "Mario/Core/SpritePathResolver.hpp"

#include <SDL.h>
#include <SDL_image.h>

#include <cctype>
#include <fstream>
#include <unordered_map>
#include <vector>

#include "Mario/Level/LevelConfig.hpp"
#include "Util/Logger.hpp"

namespace {

// Static sprite mapping tables extracted from C# reference code (.resx files)
static const std::unordered_map<std::string, std::string>
    ENTITY_SPRITE_OVERRIDE = {};

static const std::unordered_map<std::string, std::string> ENTITY_NAME_OVERRIDE =
    {
        {"BrickBlock2Break", "BrickBlockBreak1.png"},
};

static const std::unordered_map<std::string, std::string>
    LEVEL_ENTITY_NAME_OVERRIDE = {
        {"8-4/KoopaTroopa", "Koopa"},
        {"8-4/KoopaTroopaSquish", "KoopaSquish"},
};

static const std::unordered_map<std::string, std::string> BLOCK_SPRITE_MAP = {
    // Brick blocks
    {"BrickBlock20", "BrickBlock1.png"},
    {"BrickBlock2Break", "BrickBlockBreak1.png"},
    {"BrickBlock0", "BrickBlock.png"},

    // Question blocks
    {"QuestionBlock10", "QuestionBlock3.png"},
    {"QuestionBlock11", "QuestionBlock11.png"},
    {"QuestionBlock12", "QuestionBlock21.png"},
    {"QuestionBlock0", "QuestionBlock.png"},
    {"QuestionBlock1", "QuestionBlock1.png"},
    {"QuestionBlock2", "QuestionBlock2.png"},
    {"QuestionBlockHit", "QuestionBlockHit.png"},
    {"QuestionBlockHit1", "QuestionBlockHit1.png"},

    // Solid blocks
    {"SolidBlock10", "SolidBlock1.png"},
    {"SolidBlock0", "SolidBlock.png"},

    // Ground blocks
    {"Ground20", "Ground2.png"},
    {"Ground0", "Ground.png"},

    // Castle blocks
    {"Castle10", "Castle.png"},
    {"Castle20", "Castle2.png"},
    {"Castle30", "Castle3.png"},
    {"Castle40", "Castle4.png"},
    {"Castle50", "Castle5.png"},
    {"Castle60", "Castle6.png"},

    // Black space block
    {"Black0", "Black.png"},

    // Player spawn blocks
    {"MarioStartBlack0", "Black.png"},
    {"MarioStartGreen0", "MarioSpawnGreen.png"},
    {"MarioStartBlue0", "Sky.png"},
};

static const std::unordered_map<std::string, std::string> PLAYER_SPRITE_MAP = {
    // Idle animations
    {"MarioIdle00", "MarioIdle.png"},
    {"MarioIdle0", "MarioIdle.png"},
    {"MarioIdle10", "MarioIdle1.png"},
    {"MarioIdle1", "MarioIdle1.png"},
    {"MarioIdle20", "MarioIdle2.png"},
    {"MarioIdle2", "MarioIdle2.png"},

    // Right movement - Small
    {"MarioRight00", "MarioRight1.png"},
    {"MarioRight01", "MarioRight2.png"},
    {"MarioRight02", "MarioRight3.png"},

    // Right movement - Big
    {"MarioRight10", "MarioRight11.png"},
    {"MarioRight11", "MarioRight21.png"},
    {"MarioRight12", "MarioRight31.png"},

    // Right movement - Fire
    {"MarioRight20", "MarioRight12.png"},
    {"MarioRight21", "MarioRight22.png"},
    {"MarioRight22", "MarioRight32.png"},

    // Jump animations
    {"MarioJump00", "MarioJump.png"},
    {"MarioJump10", "MarioJump1.png"},
    {"MarioJump20", "MarioJump2.png"},

    // Crouch animations
    {"MarioCrouch10", "MarioCrouch1.png"},
    {"MarioCrouch20", "MarioCrouch2.png"},

    // Pole climbing animations
    {"MarioPole00", "MarioPole10.png"},
    {"MarioPole01", "MarioPole20.png"},
    {"MarioPole10", "MarioPole11.png"},
    {"MarioPole11", "MarioPole21.png"},
    {"MarioPole20", "MarioPole12.png"},
    {"MarioPole21", "MarioPole22.png"},

    // Fire throw animation
    {"MarioFire20", "MarioFire2.png"},
};

// Global cache for resolved paths to avoid per-frame disk I/O queries
static std::unordered_map<std::string, std::string> s_ResolvedPathCache;

// Helper function to check if a file exists on disk to prevent duplicated raw
// stream creations
static bool FileExists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

}  // namespace

namespace Mario {

// ============================================================================
// Block Resolution Rules
// ============================================================================

class IBlockResolutionRule {
   public:
    virtual ~IBlockResolutionRule() = default;
    virtual std::string TryResolve(const std::string& blockName, int frame,
                                   const std::string& levelName) = 0;
};

class LevelSpecificBlockRule : public IBlockResolutionRule {
   public:
    std::string TryResolve(const std::string& blockName, int /*frame*/,
                           const std::string& levelName) override {
        if (!levelName.empty()) {
            std::string levelDir =
                SpritePathResolver::SPRITE_BASE_PATH + levelName + "/";

            // Try exact name match first (e.g. 1-2/BrickBlock.png)
            std::string exactPath = levelDir + blockName + ".png";
            if (FileExists(exactPath)) return exactPath;

            // Try 1-based variant (e.g. 1-2/BrickBlock1.png)
            std::string oneBased = levelDir + blockName + "1.png";
            if (FileExists(oneBased)) return oneBased;
        }
        return "";
    }
};

class MapLookupBlockRule : public IBlockResolutionRule {
   public:
    std::string TryResolve(const std::string& blockName, int frame,
                           const std::string& levelName) override {
        std::string csName = blockName;
        if (frame >= 0 && blockName.find("Hit") == std::string::npos &&
            blockName.find("Break") == std::string::npos) {
            csName += std::to_string(frame);
        }

        auto it = BLOCK_SPRITE_MAP.find(csName);
        if (it != BLOCK_SPRITE_MAP.end()) {
            std::string filename = it->second;
            if (blockName == "MarioStartBlue" &&
                LevelConfig::GetProfile(levelName).isUnderground) {
                filename = "Black.png";
            }
            std::string path = SpritePathResolver::SPRITE_BASE_PATH + filename;
            if (FileExists(path)) return path;
        }
        return "";
    }
};

class DirectConstructBlockRule : public IBlockResolutionRule {
   public:
    std::string TryResolve(const std::string& blockName, int frame,
                           const std::string& /*levelName*/) override {
        std::string targetFile = blockName + ".png";
        if (frame > 0) {
            targetFile = blockName + std::to_string(frame) + ".png";
        }

        std::string path = SpritePathResolver::SPRITE_BASE_PATH + targetFile;
        if (FileExists(path)) return path;
        return "";
    }
};

class StrippedNameBlockRule : public IBlockResolutionRule {
   public:
    std::string TryResolve(const std::string& blockName, int /*frame*/,
                           const std::string& /*levelName*/) override {
        std::string strippedName = blockName;
        while (!strippedName.empty() && std::isdigit(strippedName.back())) {
            strippedName.pop_back();
        }
        if (strippedName != blockName) {
            std::string fallbackPath =
                SpritePathResolver::SPRITE_BASE_PATH + strippedName + ".png";
            if (FileExists(fallbackPath)) return fallbackPath;
        }
        return "";
    }
};

class UltimateFallbackBlockRule : public IBlockResolutionRule {
   public:
    std::string TryResolve(const std::string& blockName, int frame,
                           const std::string& /*levelName*/) override {
        return SpritePathResolver::SPRITE_BASE_PATH + blockName +
               (frame > 0 ? std::to_string(frame) : "") + ".png";
    }
};

/**
 * @class DefaultBlockResolver
 * @brief Default strategy implementation for block sprite path resolution.
 *        Orchestrates a pipeline of block path resolution rules.
 * @inheritance IBlockResolver -> DefaultBlockResolver
 */
class DefaultBlockResolver : public IBlockResolver {
   public:
    DefaultBlockResolver() {
        m_Rules.push_back(std::make_unique<LevelSpecificBlockRule>());
        m_Rules.push_back(std::make_unique<MapLookupBlockRule>());
        m_Rules.push_back(std::make_unique<DirectConstructBlockRule>());
        m_Rules.push_back(std::make_unique<StrippedNameBlockRule>());
        m_Rules.push_back(std::make_unique<UltimateFallbackBlockRule>());
    }

    std::string Resolve(const std::string& blockName, int frame,
                        const std::string& levelName) override {
        for (const auto& rule : m_Rules) {
            std::string path = rule->TryResolve(blockName, frame, levelName);
            if (!path.empty()) return path;
        }
        return "";
    }

   private:
    std::vector<std::unique_ptr<IBlockResolutionRule>> m_Rules;
};

// ============================================================================
// Player Resolution Rules
// ============================================================================

class IPlayerResolutionRule {
   public:
    virtual ~IPlayerResolutionRule() = default;
    virtual std::string TryResolve(const std::string& prefix, int state,
                                   int frame, int starState) = 0;
};

class MapLookupPlayerRule : public IPlayerResolutionRule {
   public:
    std::string TryResolve(const std::string& prefix, int state, int frame,
                           int /*starState*/) override {
        std::string csName =
            "Mario" + prefix + std::to_string(state) + std::to_string(frame);
        auto it = PLAYER_SPRITE_MAP.find(csName);
        if (it != PLAYER_SPRITE_MAP.end()) {
            std::string path =
                SpritePathResolver::SPRITE_BASE_PATH + it->second;
            if (FileExists(path)) return path;
        }
        return "";
    }
};

class StarStatePlayerRule : public IPlayerResolutionRule {
   public:
    std::string TryResolve(const std::string& prefix, int state, int frame,
                           int starState) override {
        if (state != 3 && state != 4) return "";

        std::string csName =
            "Mario" + prefix + std::to_string(state) + std::to_string(frame);
        std::string starCsName = csName + std::to_string(starState);

        std::string directPath =
            SpritePathResolver::SPRITE_BASE_PATH + starCsName + ".png";
        if (FileExists(directPath)) return directPath;

        // Fallback to color index 0
        std::string altPath =
            SpritePathResolver::SPRITE_BASE_PATH + csName + "0.png";
        if (FileExists(altPath)) return altPath;

        // Fallback to base non-star state (Small state 0, Big state 1)
        int baseState = (state == 3) ? 0 : 1;
        std::string baseCsName = "Mario" + prefix + std::to_string(baseState) +
                                 std::to_string(frame);
        auto baseIt = PLAYER_SPRITE_MAP.find(baseCsName);
        if (baseIt != PLAYER_SPRITE_MAP.end()) {
            std::string baseFilePath =
                SpritePathResolver::SPRITE_BASE_PATH + baseIt->second;
            if (FileExists(baseFilePath)) return baseFilePath;
        }

        // Star ultimate fallback: standard idle
        std::string fallbackFile =
            (baseState == 0) ? "MarioIdle.png" : "MarioIdle1.png";
        return SpritePathResolver::SPRITE_BASE_PATH + fallbackFile;
    }
};

class DefaultFallbackPlayerRule : public IPlayerResolutionRule {
   public:
    std::string TryResolve(const std::string& /*prefix*/, int /*state*/,
                           int /*frame*/, int /*starState*/) override {
        return SpritePathResolver::SPRITE_BASE_PATH + "MarioIdle.png";
    }
};

/**
 * @class DefaultPlayerResolver
 * @brief Default strategy implementation for player sprite path resolution.
 *        Orchestrates a pipeline of player path resolution rules.
 * @inheritance IPlayerResolver -> DefaultPlayerResolver
 */
class DefaultPlayerResolver : public IPlayerResolver {
   public:
    DefaultPlayerResolver() {
        m_Rules.push_back(std::make_unique<MapLookupPlayerRule>());
        m_Rules.push_back(std::make_unique<StarStatePlayerRule>());
        m_Rules.push_back(std::make_unique<DefaultFallbackPlayerRule>());
    }

    std::string Resolve(const std::string& prefix, int state, int frame,
                        int starState) override {
        for (const auto& rule : m_Rules) {
            std::string path =
                rule->TryResolve(prefix, state, frame, starState);
            if (!path.empty()) return path;
        }
        return SpritePathResolver::SPRITE_BASE_PATH + "MarioIdle.png";
    }

   private:
    std::vector<std::unique_ptr<IPlayerResolutionRule>> m_Rules;
};

// ============================================================================
// Entity Resolution Rules
// ============================================================================

class IEntityResolutionRule {
   public:
    virtual ~IEntityResolutionRule() = default;
    virtual std::string TryResolve(const std::string& entityName, int frame,
                                   const std::string& levelName) = 0;
};

class StaticOverrideEntityRule : public IEntityResolutionRule {
   public:
    std::string TryResolve(const std::string& entityName, int frame,
                           const std::string& /*levelName*/) override {
        // Name overrides
        auto nit = ENTITY_NAME_OVERRIDE.find(entityName);
        if (nit != ENTITY_NAME_OVERRIDE.end()) {
            std::string path =
                SpritePathResolver::SPRITE_BASE_PATH + nit->second;
            if (FileExists(path)) return path;
        }

        // Sprite overrides
        if (frame >= 0) {
            std::string overrideKey = entityName + std::to_string(frame + 1);
            auto oit = ENTITY_SPRITE_OVERRIDE.find(overrideKey);
            if (oit != ENTITY_SPRITE_OVERRIDE.end()) {
                std::string path =
                    SpritePathResolver::SPRITE_BASE_PATH + oit->second;
                if (FileExists(path)) return path;
            }
        }
        return "";
    }
};

class LevelSpecificEntityRule : public IEntityResolutionRule {
   public:
    std::string TryResolve(const std::string& entityName, int frame,
                           const std::string& levelName) override {
        std::string levelDir =
            SpritePathResolver::SPRITE_BASE_PATH + levelName + "/";

        std::string resolvedName = entityName;
        std::string levelKey = levelName + "/" + entityName;
        auto lit = LEVEL_ENTITY_NAME_OVERRIDE.find(levelKey);
        if (lit != LEVEL_ENTITY_NAME_OVERRIDE.end()) {
            resolvedName = lit->second;
        }

        if (frame >= 0) {
            std::string path =
                levelDir + resolvedName + std::to_string(frame) + ".png";
            if (FileExists(path)) return path;

            std::string path1based =
                levelDir + resolvedName + std::to_string(frame + 1) + ".png";
            if (FileExists(path1based)) return path1based;
        }

        std::string basePath = levelDir + resolvedName + ".png";
        if (FileExists(basePath)) return basePath;

        return "";
    }
};

class FallbackEntityRule : public IEntityResolutionRule {
   public:
    std::string TryResolve(const std::string& entityName, int frame,
                           const std::string& /*levelName*/) override {
        if (frame >= 0) {
            std::string path = SpritePathResolver::SPRITE_BASE_PATH +
                               entityName + std::to_string(frame) + ".png";
            if (FileExists(path)) return path;

            std::string path1based = SpritePathResolver::SPRITE_BASE_PATH +
                                     entityName + std::to_string(frame + 1) +
                                     ".png";
            if (FileExists(path1based)) return path1based;
        }

        std::string mainPath =
            SpritePathResolver::SPRITE_BASE_PATH + entityName + ".png";
        if (FileExists(mainPath)) return mainPath;

        return "";
    }
};

/**
 * @class DefaultEntityResolver
 * @brief Default strategy implementation for entity sprite path resolution.
 *        Orchestrates a pipeline of entity path resolution rules.
 * @inheritance IEntityResolver -> DefaultEntityResolver
 */
class DefaultEntityResolver : public IEntityResolver {
   public:
    DefaultEntityResolver() {
        m_Rules.push_back(std::make_unique<StaticOverrideEntityRule>());
        m_Rules.push_back(std::make_unique<LevelSpecificEntityRule>());
        m_Rules.push_back(std::make_unique<FallbackEntityRule>());
    }

    std::string Resolve(const std::string& entityName, int frame,
                        const std::string& levelName) override {
        for (const auto& rule : m_Rules) {
            std::string path = rule->TryResolve(entityName, frame, levelName);
            if (!path.empty()) return path;
        }
        return "";
    }

   private:
    std::vector<std::unique_ptr<IEntityResolutionRule>> m_Rules;
};

/**
 * @class DefaultCastleResolver
 * @brief Default strategy implementation for 8-4 castle sprite path resolution.
 * @inheritance ICastleResolver -> DefaultCastleResolver
 */
class DefaultCastleResolver : public ICastleResolver {
   public:
    std::string Resolve(int blockID) override {
        int actualID = blockID;
        if (actualID == 905) {
            actualID = 826;  // render as tile_0026
        }

        if (actualID >= 801 && actualID <= 904) {
            int tileIdx = actualID - 800;
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "%s8-4/tile_%04d.png",
                     SpritePathResolver::SPRITE_BASE_PATH.c_str(), tileIdx);

            std::string path = buffer;
            if (FileExists(path)) {
                return path;
            }
        }

        return "";
    }
};

const std::string SpritePathResolver::SPRITE_BASE_PATH =
    std::string(RESOURCE_DIR) + "/Sprites/";

std::unique_ptr<IBlockResolver> SpritePathResolver::s_BlockResolver = nullptr;
std::unique_ptr<IPlayerResolver> SpritePathResolver::s_PlayerResolver = nullptr;
std::unique_ptr<IEntityResolver> SpritePathResolver::s_EntityResolver = nullptr;
std::unique_ptr<ICastleResolver> SpritePathResolver::s_CastleResolver = nullptr;

void SpritePathResolver::SetBlockResolver(
    std::unique_ptr<IBlockResolver> resolver) {
    s_BlockResolver = std::move(resolver);
}

void SpritePathResolver::SetPlayerResolver(
    std::unique_ptr<IPlayerResolver> resolver) {
    s_PlayerResolver = std::move(resolver);
}

void SpritePathResolver::SetEntityResolver(
    std::unique_ptr<IEntityResolver> resolver) {
    s_EntityResolver = std::move(resolver);
}

void SpritePathResolver::SetCastleResolver(
    std::unique_ptr<ICastleResolver> resolver) {
    s_CastleResolver = std::move(resolver);
}

void SpritePathResolver::EnsureResolversInitialized() {
    if (!s_BlockResolver) {
        s_BlockResolver = std::make_unique<DefaultBlockResolver>();
    }
    if (!s_PlayerResolver) {
        s_PlayerResolver = std::make_unique<DefaultPlayerResolver>();
    }
    if (!s_EntityResolver) {
        s_EntityResolver = std::make_unique<DefaultEntityResolver>();
    }
    if (!s_CastleResolver) {
        s_CastleResolver = std::make_unique<DefaultCastleResolver>();
    }
}

std::string SpritePathResolver::GetSpritePath(const std::string& name,
                                              const std::string& suffix) {
    return SPRITE_BASE_PATH + name + suffix + ".png";
}

std::string SpritePathResolver::GetBlockSpritePath(
    const std::string& blockName, int frame, const std::string& levelName) {
    std::string key =
        "block:" + levelName + ":" + blockName + ":" + std::to_string(frame);
    auto cacheIt = s_ResolvedPathCache.find(key);
    if (cacheIt != s_ResolvedPathCache.end()) {
        return cacheIt->second;
    }

    EnsureResolversInitialized();
    std::string resolvedPath =
        s_BlockResolver->Resolve(blockName, frame, levelName);
    s_ResolvedPathCache[key] = resolvedPath;
    return resolvedPath;
}

std::string SpritePathResolver::GetPlayerSpritePath(const std::string& prefix,
                                                    int state, int frame,
                                                    int starState) {
    std::string key = "player:" + prefix + ":" + std::to_string(state) + ":" +
                      std::to_string(frame) + ":" + std::to_string(starState);
    auto cacheIt = s_ResolvedPathCache.find(key);
    if (cacheIt != s_ResolvedPathCache.end()) {
        return cacheIt->second;
    }

    EnsureResolversInitialized();
    std::string resolvedPath =
        s_PlayerResolver->Resolve(prefix, state, frame, starState);
    s_ResolvedPathCache[key] = resolvedPath;
    return resolvedPath;
}

std::string SpritePathResolver::GetEntitySpritePath(
    const std::string& entityName, int frame, const std::string& levelName) {
    std::string key =
        "entity:" + levelName + ":" + entityName + ":" + std::to_string(frame);
    auto cacheIt = s_ResolvedPathCache.find(key);
    if (cacheIt != s_ResolvedPathCache.end()) {
        return cacheIt->second;
    }

    EnsureResolversInitialized();
    std::string resolvedPath =
        s_EntityResolver->Resolve(entityName, frame, levelName);
    s_ResolvedPathCache[key] = resolvedPath;
    return resolvedPath;
}

std::string SpritePathResolver::GetCastleSpritePathByID(int blockID) {
    std::string key = "castle:" + std::to_string(blockID);
    auto cacheIt = s_ResolvedPathCache.find(key);
    if (cacheIt != s_ResolvedPathCache.end()) {
        return cacheIt->second;
    }

    EnsureResolversInitialized();
    std::string resolvedPath = s_CastleResolver->Resolve(blockID);
    s_ResolvedPathCache[key] = resolvedPath;
    return resolvedPath;
}

}  // namespace Mario
