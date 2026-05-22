/**
 * @file SpritePathResolver.cpp
 * @brief Implementation of sprite path resolution.
 *        Handles the C# reference's sprite naming conventions.
 *
 *        Block sprites: name from IDList.csv + frame suffix
 *          - C# tries: name + "0" (e.g. "Ground0") but files are "Ground.png"
 *          - We try: name.png first, then name + frame + ".png"
 *
 *        Player sprites: "Mario" + prefix + state + frame
 *          - Small (state=0): "MarioIdle.png", "MarioRight1.png"
 *          - Big (state=1):   "MarioIdle1.png", "MarioRight11.png"
 *          - The state digit is omitted when 0, frame digit omitted when 0
 *
 * @inheritance None
 */
#include "Mario/SpritePathResolver.hpp"

#include <SDL.h>
#include <SDL_image.h>

#include <cctype>
#include <fstream>
#include <unordered_map>

#include "Util/Logger.hpp"

namespace {

// Static sprite mapping tables extracted from C# reference code (.resx files)
// These mappings represent the exact correspondence between C# sprite names
// and the actual PNG filenames. Do not modify without verification against
// the reference C# code.

/**
 * Entity sprite overrides for enemies that cannot be resolved by the normal
 * level-directory lookup.  Currently empty — all 8-4 entity sprites exist in
 * Resources/Sprites/8-4/ and are found automatically by GetEntitySpritePath.
 */
static const std::unordered_map<std::string, std::string>
    ENTITY_SPRITE_OVERRIDE = {};

/**
 * Entity name override: maps entity name (no frame) to PNG filename.
 * Used for entities whose name doesn't directly match the PNG filename.
 */
static const std::unordered_map<std::string, std::string> ENTITY_NAME_OVERRIDE =
    {
        {"BrickBlock2Break", "BrickBlockBreak1.png"},
};

/**
 * Level-specific entity sprite name remapping.
 * Key format: "levelName/entityName".
 * Value: the display name to use when looking up sprites in the level subdir.
 * Example: in 8-4 the KoopaTroopa sprites are stored as Koopa1.png/Koopa2.png
 * instead of KoopaTroopa1.png/KoopaTroopa2.png.
 */
static const std::unordered_map<std::string, std::string>
    LEVEL_ENTITY_NAME_OVERRIDE = {
        {"8-4/KoopaTroopa", "Koopa"},
        {"8-4/KoopaTroopaSquish", "KoopaSquish"},
};

/**
 * Block sprite mapping: C# resource name -> PNG filename
 * Example: C# "BrickBlock20" -> actual file "BrickBlock1.png"
 */
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

/**
 * Player sprite mapping: C# animation name -> PNG filename
 * Format: "Mario" + prefix + state + frame
 * Example: "MarioRight00" -> "MarioRight1.png"
 */
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

}  // namespace

namespace Mario {

const std::string SpritePathResolver::SPRITE_BASE_PATH =
    std::string(RESOURCE_DIR) + "/Sprites/";

std::string SpritePathResolver::GetSpritePath(const std::string& name,
                                              const std::string& suffix) {
    return SPRITE_BASE_PATH + name + suffix + ".png";
}

std::string SpritePathResolver::GetBlockSpritePath(
    const std::string& blockName, int frame, const std::string& levelName) {
    auto resolve = [&]() -> std::string {
        // Check level-specific directory first (e.g. 1-2/BrickBlock1.png)
        if (!levelName.empty()) {
            std::string levelDir = SPRITE_BASE_PATH + levelName + "/";

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
            std::string path = SPRITE_BASE_PATH + it->second;
            std::ifstream test(path);
            if (test.good()) return path;
        }

        // Fallback: Try constructed filenames
        std::string targetFile = blockName + ".png";
        if (frame > 0) {
            targetFile = blockName + std::to_string(frame) + ".png";
        }

        std::string path = SPRITE_BASE_PATH + targetFile;
        std::ifstream test(path);
        if (test.good()) return path;

        // Try without trailing digits
        std::string strippedName = blockName;
        while (!strippedName.empty() && std::isdigit(strippedName.back())) {
            strippedName.pop_back();
        }
        if (strippedName != blockName) {
            std::string fallbackPath = SPRITE_BASE_PATH + strippedName + ".png";
            std::ifstream testFallback(fallbackPath);
            if (testFallback.good()) return fallbackPath;
        }

        // Ultimate fallback
        return SPRITE_BASE_PATH + blockName +
               (frame > 0 ? std::to_string(frame) : "") + ".png";
    };
    return resolve();
}

std::string SpritePathResolver::GetPlayerSpritePath(const std::string& prefix,
                                                    int state, int frame,
                                                    int starState) {
    // C# reference name: "Mario" + prefix + state + frame + (starState if
    // state==3||4)
    std::string csName =
        "Mario" + prefix + std::to_string(state) + std::to_string(frame);

    if (state == 3 || state == 4) {
        csName += std::to_string(starState);
    }

    // Check the sprite mapping table FIRST.
    // Important: direct file lookup must NOT run first for regular states
    // because some C# logical names collide with existing PNG filenames that
    // belong to a different state.  Example: Big Mario running frame 2 builds
    // csName="MarioRight12", and MarioRight12.png exists — but that file is
    // the Fire Mario sprite.  The map correctly remaps to MarioRight31.png.
    auto it = PLAYER_SPRITE_MAP.find(csName);
    if (it != PLAYER_SPRITE_MAP.end()) {
        std::string path = SPRITE_BASE_PATH + it->second;
        std::ifstream test(path);
        if (test.good()) return path;
    }

    // For Star states (3/4) the sprite names are unique enough that a direct
    // file match is safe and avoids needing an exhaustive map entry for every
    // star-color variant.
    if (state == 3 || state == 4) {
        std::string directPath = SPRITE_BASE_PATH + csName + ".png";
        std::ifstream directTest(directPath);
        if (directTest.good()) {
            return directPath;
        }
        return directPath;  // Return even if missing; caller handles fallback
    }

    // Default fallback (should rarely be reached after map lookup above)
    return SPRITE_BASE_PATH + "MarioIdle.png";
}

std::string SpritePathResolver::GetEntitySpritePath(
    const std::string& entityName, int frame, const std::string& levelName) {
    auto resolve = [&]() -> std::string {
        // Check entity name overrides (frame-independent remapping)
        auto nit = ENTITY_NAME_OVERRIDE.find(entityName);
        if (nit != ENTITY_NAME_OVERRIDE.end()) {
            std::string path = SPRITE_BASE_PATH + nit->second;
            std::ifstream test(path);
            if (test.good()) return path;
        }

        // Check entity sprite overrides first (e.g. PiranhaPlant placeholder)
        if (frame >= 0) {
            std::string overrideKey = entityName + std::to_string(frame + 1);
            auto oit = ENTITY_SPRITE_OVERRIDE.find(overrideKey);
            if (oit != ENTITY_SPRITE_OVERRIDE.end()) {
                std::string path = SPRITE_BASE_PATH + oit->second;
                std::ifstream test(path);
                if (test.good()) return path;
            }
        }

        // Build level-specific sprite path
        std::string levelDir = SPRITE_BASE_PATH + levelName + "/";

        // Apply level-specific entity name remapping (e.g. 8-4 KoopaTroopa ->
        // Koopa)
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
            std::string path =
                SPRITE_BASE_PATH + entityName + std::to_string(frame) + ".png";
            std::ifstream test2(path);
            if (test2.good()) return path;

            std::string path1based = SPRITE_BASE_PATH + entityName +
                                     std::to_string(frame + 1) + ".png";
            std::ifstream test1based(path1based);
            if (test1based.good()) return path1based;
        }

        std::string mainPath = SPRITE_BASE_PATH + entityName + ".png";
        std::ifstream test3(mainPath);
        if (test3.good()) return mainPath;

        // If nothing found, return empty string to indicate missing sprite
        return "";
    };

    std::string path = resolve();
    if (path.empty()) {
        // LOG_WARN(
        //     "GetEntitySpritePath: No sprite found for '{}' (frame: {}) in "
        //     "level '{}'",
        //     entityName, frame, levelName);
        return "";  // Return empty path, caller should handle this
    }

    return path;
}

std::string SpritePathResolver::GetCastleSpritePathByID(int blockID) {
    // 8-4 castle sprites: ID 801-904 -> Sprites/8-4/tile_XXX.png
    // ID 905 (PiranhaPipeTop spawner) reuses the pipe-body-right tile (826 =
    // tile_0026) so the pipe opening row looks visually correct.
    if (blockID == 905) {
        blockID = 826;  // render as tile_0026 (right side of pipe body)
    }
    if (blockID < 801 || blockID > 904) {
        return "";  // Invalid ID for 8-4
    }

    int tileIdx = blockID - 800;  // 801 -> 1, 904 -> 104
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s8-4/tile_%04d.png",
             SPRITE_BASE_PATH.c_str(), tileIdx);

    std::string path = buffer;
    std::ifstream test(path);
    if (test.good()) {
        return path;  // File exists, return as-is (no ProcessTransparent
                      // needed)
    }
    return "";  // File doesn't exist
}

}  // namespace Mario
