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

}  // namespace

namespace Mario {

/**
 * @class DefaultBlockResolver
 * @brief Default strategy implementation for block sprite path resolution.
 * @inheritance IBlockResolver -> DefaultBlockResolver
 */
class DefaultBlockResolver : public IBlockResolver {
   public:
    std::string Resolve(const std::string& blockName, int frame,
                        const std::string& levelName) override {
        // Check level-specific directory first (e.g. 1-2/BrickBlock1.png)
        if (!levelName.empty()) {
            std::string levelDir =
                SpritePathResolver::SPRITE_BASE_PATH + levelName + "/";

            // Try exact name match first
            std::string exactPath = levelDir + blockName + ".png";
            std::ifstream exactTest(exactPath);
            if (exactTest.good()) return exactPath;

            // Try 1-based variant (e.g. BrickBlock1.png for BrickBlock in 1-2)
            std::string oneBased = levelDir + blockName + "1.png";
            std::ifstream oneTest(oneBased);
            if (oneTest.good()) return oneBased;
        }

        // Build the C# resource name
        std::string csName = blockName;
        if (frame >= 0 && blockName.find("Hit") == std::string::npos &&
            blockName.find("Break") == std::string::npos) {
            csName += std::to_string(frame);
        }

        // Attempt lookup in static sprite mapping table
        auto it = BLOCK_SPRITE_MAP.find(csName);
        if (it != BLOCK_SPRITE_MAP.end()) {
            std::string filename = it->second;
            // MarioStartBlue should be mapped to a black tile in
            // castle/underground levels
            if (blockName == "MarioStartBlue" &&
                (levelName == "8-4" || levelName == "1-2" ||
                 levelName.find('u') != std::string::npos)) {
                filename = "Black.png";
            }
            std::string path = SpritePathResolver::SPRITE_BASE_PATH + filename;
            std::ifstream test(path);
            if (test.good()) return path;
        }

        // Fallback: Try constructed filenames
        std::string targetFile = blockName + ".png";
        if (frame > 0) {
            targetFile = blockName + std::to_string(frame) + ".png";
        }

        std::string path = SpritePathResolver::SPRITE_BASE_PATH + targetFile;
        std::ifstream test(path);
        if (test.good()) return path;

        // Try without trailing digits
        std::string strippedName = blockName;
        while (!strippedName.empty() && std::isdigit(strippedName.back())) {
            strippedName.pop_back();
        }
        if (strippedName != blockName) {
            std::string fallbackPath =
                SpritePathResolver::SPRITE_BASE_PATH + strippedName + ".png";
            std::ifstream testFallback(fallbackPath);
            if (testFallback.good()) return fallbackPath;
        }

        // Ultimate fallback
        return SpritePathResolver::SPRITE_BASE_PATH + blockName +
               (frame > 0 ? std::to_string(frame) : "") + ".png";
    }
};

/**
 * @class DefaultPlayerResolver
 * @brief Default strategy implementation for player sprite path resolution.
 *        Handles star mode color variants with fallbacks to avoid terminal
 * errors.
 * @inheritance IPlayerResolver -> DefaultPlayerResolver
 */
class DefaultPlayerResolver : public IPlayerResolver {
   public:
    std::string Resolve(const std::string& prefix, int state, int frame,
                        int starState) override {
        // C# reference name: "Mario" + prefix + state + frame + (starState if
        // state==3||4)
        std::string csName =
            "Mario" + prefix + std::to_string(state) + std::to_string(frame);

        if (state == 3 || state == 4) {
            csName += std::to_string(starState);
        }

        std::string resolvedPath;

        // Check the sprite mapping table FIRST.
        auto it = PLAYER_SPRITE_MAP.find(csName);
        if (it != PLAYER_SPRITE_MAP.end()) {
            std::string path =
                SpritePathResolver::SPRITE_BASE_PATH + it->second;
            std::ifstream test(path);
            if (test.good()) {
                resolvedPath = path;
            }
        }

        if (resolvedPath.empty()) {
            // For Star states (3/4) the sprite names are unique enough that a
            // direct file match is safe and avoids needing an exhaustive map
            // entry for every star-color variant.
            if (state == 3 || state == 4) {
                std::string directPath =
                    SpritePathResolver::SPRITE_BASE_PATH + csName + ".png";
                std::ifstream directTest(directPath);
                if (directTest.good()) {
                    resolvedPath = directPath;
                } else {
                    // If the specific star-color variant is missing, attempt to
                    // fall back to color index 0
                    std::string altName = "Mario" + prefix +
                                          std::to_string(state) +
                                          std::to_string(frame) + "0";
                    std::string altPath =
                        SpritePathResolver::SPRITE_BASE_PATH + altName + ".png";
                    std::ifstream altTest(altPath);
                    if (altTest.good()) {
                        resolvedPath = altPath;
                    } else {
                        // Fall back to the base (non-star) state sprite
                        // State 3 (Small Star) -> State 0 (Small)
                        // State 4 (Big Star) -> State 1 (Big)
                        int baseState = (state == 3) ? 0 : 1;
                        std::string baseCsName = "Mario" + prefix +
                                                 std::to_string(baseState) +
                                                 std::to_string(frame);
                        auto baseIt = PLAYER_SPRITE_MAP.find(baseCsName);
                        if (baseIt != PLAYER_SPRITE_MAP.end()) {
                            std::string baseFilename = baseIt->second;
                            std::string baseFilePath =
                                SpritePathResolver::SPRITE_BASE_PATH +
                                baseFilename;
                            std::ifstream baseTest(baseFilePath);
                            if (baseTest.good()) {
                                resolvedPath = baseFilePath;
                            }
                        }
                        if (resolvedPath.empty()) {
                            // Ultimate fallback: standard idle sprites
                            std::string fallbackFile = (baseState == 0)
                                                           ? "MarioIdle.png"
                                                           : "MarioIdle1.png";
                            resolvedPath =
                                SpritePathResolver::SPRITE_BASE_PATH +
                                fallbackFile;
                        }
                    }
                }
            } else {
                // Default fallback (should rarely be reached after map lookup
                // above)
                resolvedPath =
                    SpritePathResolver::SPRITE_BASE_PATH + "MarioIdle.png";
            }
        }

        return resolvedPath;
    }
};

/**
 * @class DefaultEntityResolver
 * @brief Default strategy implementation for entity sprite path resolution.
 * @inheritance IEntityResolver -> DefaultEntityResolver
 */
class DefaultEntityResolver : public IEntityResolver {
   public:
    std::string Resolve(const std::string& entityName, int frame,
                        const std::string& levelName) override {
        // Check entity name overrides (frame-independent remapping)
        auto nit = ENTITY_NAME_OVERRIDE.find(entityName);
        if (nit != ENTITY_NAME_OVERRIDE.end()) {
            std::string path =
                SpritePathResolver::SPRITE_BASE_PATH + nit->second;
            std::ifstream test(path);
            if (test.good()) return path;
        }

        // Check entity sprite overrides first (e.g. PiranhaPlant placeholder)
        if (frame >= 0) {
            std::string overrideKey = entityName + std::to_string(frame + 1);
            auto oit = ENTITY_SPRITE_OVERRIDE.find(overrideKey);
            if (oit != ENTITY_SPRITE_OVERRIDE.end()) {
                std::string path =
                    SpritePathResolver::SPRITE_BASE_PATH + oit->second;
                std::ifstream test(path);
                if (test.good()) return path;
            }
        }

        // Build level-specific sprite path
        std::string levelDir =
            SpritePathResolver::SPRITE_BASE_PATH + levelName + "/";

        // Apply level-specific entity name remapping
        std::string resolvedName = entityName;
        std::string levelKey = levelName + "/" + entityName;
        auto lit = LEVEL_ENTITY_NAME_OVERRIDE.find(levelKey);
        if (lit != LEVEL_ENTITY_NAME_OVERRIDE.end()) {
            resolvedName = lit->second;
        }

        // Try frame-suffixed version first (for animated sprites)
        if (frame >= 0) {
            std::string path =
                levelDir + resolvedName + std::to_string(frame) + ".png";
            std::ifstream test(path);
            if (test.good()) return path;

            std::string path1based =
                levelDir + resolvedName + std::to_string(frame + 1) + ".png";
            std::ifstream test1based(path1based);
            if (test1based.good()) return path1based;
        }

        // Fall back to base entity sprite (no frame suffix)
        std::string basePath = levelDir + resolvedName + ".png";
        std::ifstream test(basePath);
        if (test.good()) return basePath;

        // If level-specific not found, try main Sprites directory (fallback)
        if (frame >= 0) {
            std::string path = SpritePathResolver::SPRITE_BASE_PATH +
                               entityName + std::to_string(frame) + ".png";
            std::ifstream test2(path);
            if (test2.good()) return path;

            std::string path1based = SpritePathResolver::SPRITE_BASE_PATH +
                                     entityName + std::to_string(frame + 1) +
                                     ".png";
            std::ifstream test1based(path1based);
            if (test1based.good()) return path1based;
        }

        std::string mainPath =
            SpritePathResolver::SPRITE_BASE_PATH + entityName + ".png";
        std::ifstream test3(mainPath);
        if (test3.good()) return mainPath;

        // If nothing found, return empty string
        return "";
    }
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
            std::ifstream test(path);
            if (test.good()) {
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
