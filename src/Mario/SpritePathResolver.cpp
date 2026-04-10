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
#include <map>

#include "Util/Logger.hpp"

namespace Mario {

const std::string SpritePathResolver::SPRITE_BASE_PATH =
    std::string(RESOURCE_DIR) + "/Sprites/";

std::string SpritePathResolver::ProcessTransparent(
    const std::string& originalPath) {
    if (originalPath.empty()) return originalPath;

    // Cache dictionary for transparent paths to avoid massive file IO
    static std::map<std::string, std::string> s_TransparentCache;
    auto it = s_TransparentCache.find(originalPath);
    if (it != s_TransparentCache.end()) {
        return it->second;
    }

    // Process only PNG or BMP files
    size_t dotPos = originalPath.find_last_of('.');
    std::string ext =
        (dotPos != std::string::npos) ? originalPath.substr(dotPos) : ".png";
    std::string base = (dotPos != std::string::npos)
                           ? originalPath.substr(0, dotPos)
                           : originalPath;
    if (ext != ".png" && ext != ".bmp") {
        s_TransparentCache[originalPath] = originalPath;
        return originalPath;
    }

    std::string cachePath = base + "_tp.png";

    // Check if cached file exists without loading anything
    std::ifstream cacheCheck(cachePath);
    if (cacheCheck.good()) {
        s_TransparentCache[originalPath] = cachePath;
        return cachePath;
    }
    cacheCheck.close();

    // Load and process image to make White background transparent (replicating
    // C# MakeTransparent)
    SDL_Surface* surface = IMG_Load(originalPath.c_str());
    if (!surface) {
        LOG_ERROR("ProcessTransparent failed to load: {}", originalPath);
        return originalPath;
    }

    SDL_Surface* rgbaSurface =
        SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
    SDL_FreeSurface(surface);
    if (!rgbaSurface) return originalPath;

    Uint32* pixels = static_cast<Uint32*>(rgbaSurface->pixels);
    int pixelCount = rgbaSurface->w * rgbaSurface->h;

    for (int i = 0; i < pixelCount; ++i) {
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixels[i], rgbaSurface->format, &r, &g, &b, &a);
        // Turn perfectly white or near-white pixels to fully transparent.
        // Use strict threshold (254+) to match exact white (255,255,255),
        // avoiding edge artifacts from semi-transparent anti-aliasing.
        // This matches NES sprite sheet white color exactly.
        if (r >= 254 && g >= 254 && b >= 254) {
            pixels[i] = SDL_MapRGBA(rgbaSurface->format, 255, 255, 255, 0);
        }
    }

    if (IMG_SavePNG(rgbaSurface, cachePath.c_str()) != 0) {
        LOG_ERROR("ProcessTransparent failed to save PNG: {}", cachePath);
        SDL_FreeSurface(rgbaSurface);
        s_TransparentCache[originalPath] = originalPath;
        return originalPath;
    }

    SDL_FreeSurface(rgbaSurface);
    s_TransparentCache[originalPath] = cachePath;
    return cachePath;
}

std::string SpritePathResolver::GetSpritePath(const std::string& name,
                                              const std::string& suffix) {
    return SPRITE_BASE_PATH + name + suffix + ".png";
}

std::string SpritePathResolver::GetBlockSpritePath(const std::string& blockName,
                                                   int frame) {
    auto resolve = [&]() -> std::string {
        // Build the C# resource name
        std::string csName = blockName;
        if (frame >= 0 && blockName.find("Hit") == std::string::npos &&
            blockName.find("Break") == std::string::npos) {
            csName += std::to_string(frame);
        }

        // C# exact mappings from .resx
        std::string targetFile = blockName + ".png";  // standard fallback
        if (csName == "BrickBlock20")
            targetFile = "BrickBlock1.png";
        else if (csName == "BrickBlock2Break")
            targetFile = "BrickBlockBreak1.png";
        else if (csName == "QuestionBlock10")
            targetFile = "QuestionBlock3.png";
        else if (csName == "QuestionBlock11")
            targetFile = "QuestionBlock11.png";
        else if (csName == "QuestionBlock12")
            targetFile = "QuestionBlock21.png";
        else if (csName == "SolidBlock10")
            targetFile = "SolidBlock1.png";
        else if (csName == "Ground20")
            targetFile = "Ground2.png";
        else if (csName == "Ground0")
            targetFile = "Ground.png";
        else if (csName == "BrickBlock0")
            targetFile = "BrickBlock.png";
        else if (csName == "SolidBlock0")
            targetFile = "SolidBlock.png";
        else if (csName == "QuestionBlock0")
            targetFile = "QuestionBlock.png";
        else if (csName == "QuestionBlock1")
            targetFile = "QuestionBlock1.png";
        else if (csName == "QuestionBlock2")
            targetFile = "QuestionBlock2.png";
        else if (csName == "QuestionBlockHit")
            targetFile = "QuestionBlockHit.png";
        else if (csName == "QuestionBlockHit1")
            targetFile = "QuestionBlockHit1.png";
        else if (frame > 0)
            targetFile = blockName + std::to_string(frame) + ".png";
        else
            targetFile = blockName + ".png";

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
    return ProcessTransparent(resolve());
}

std::string SpritePathResolver::GetPlayerSpritePath(const std::string& prefix,
                                                    int state, int frame) {
    // 這裡我們直接實現 C# 的 Resources.resx 映射對應
    // C# 的 name 組合總是: "Mario" + prefix + state + frame
    // 例如: state=0, frame=0, prefix="Right" => "MarioRight00"
    // 然後 C# 的 Resources.resx 將這些名字對應到具體的檔名

    std::string csName =
        "Mario" + prefix + std::to_string(state) + std::to_string(frame);
    std::string fileName = "MarioIdle.png";  // Default fallback

    // --- Idle ---
    if (csName == "MarioIdle00")
        fileName = "MarioIdle.png";
    else if (csName == "MarioIdle10")
        fileName = "MarioIdle1.png";
    else if (csName == "MarioIdle20")
        fileName = "MarioIdle2.png";
    // --- Right ---
    else if (csName == "MarioRight00")
        fileName = "MarioRight1.png";
    else if (csName == "MarioRight01")
        fileName = "MarioRight2.png";
    else if (csName == "MarioRight02")
        fileName = "MarioRight3.png";
    // Big Right
    else if (csName == "MarioRight10")
        fileName = "MarioRight11.png";
    else if (csName == "MarioRight11")
        fileName = "MarioRight21.png";
    else if (csName == "MarioRight12")
        fileName = "MarioRight31.png";
    // Fire Right
    else if (csName == "MarioRight20")
        fileName = "MarioRight12.png";
    else if (csName == "MarioRight21")
        fileName = "MarioRight22.png";
    else if (csName == "MarioRight22")
        fileName = "MarioRight32.png";
    // --- Jump ---
    else if (csName == "MarioJump00")
        fileName = "MarioJump.png";
    else if (csName == "MarioJump10")
        fileName = "MarioJump1.png";
    else if (csName == "MarioJump20")
        fileName = "MarioJump2.png";
    // --- Crouch ---
    else if (csName == "MarioCrouch10")
        fileName = "MarioCrouch1.png";
    else if (csName == "MarioCrouch20")
        fileName = "MarioCrouch2.png";
    // --- Pole ---
    else if (csName == "MarioPole00")
        fileName = "MarioPole10.png";
    else if (csName == "MarioPole01")
        fileName = "MarioPole20.png";
    else if (csName == "MarioPole10")
        fileName = "MarioPole11.png";
    else if (csName == "MarioPole11")
        fileName = "MarioPole21.png";
    else if (csName == "MarioPole20")
        fileName = "MarioPole12.png";
    else if (csName == "MarioPole21")
        fileName = "MarioPole22.png";

    // --- Fire (Throw fireball) ---
    else if (csName == "MarioFire20")
        fileName = "MarioFire2.png";

    // Star power-ups are states 3 and 4, we can add them later if needed.

    // 將計算出的 fileName 組合為相對專案根目錄的完整路徑
    std::string path = SPRITE_BASE_PATH + fileName;

    // 檢查檔案是否存在，否則使用嚴格的備用方案
    std::ifstream test(path);
    if (!test.good()) {
        LOG_WARN(
            "GetPlayerSpritePath: Could not find {}, using MarioIdle fallback",
            path);
        path = SPRITE_BASE_PATH + "MarioIdle.png";
    }

    return ProcessTransparent(path);
}

std::string SpritePathResolver::GetEntitySpritePath(
    const std::string& entityName, int frame, const std::string& levelName) {
    auto resolve = [&]() -> std::string {
        // Build level-specific sprite path
        std::string levelDir = SPRITE_BASE_PATH + levelName + "/";

        // Try frame-suffixed version first (for animated sprites)
        if (frame >= 0) {
            std::string path =
                levelDir + entityName + std::to_string(frame) + ".png";
            std::ifstream test(path);
            if (test.good()) return path;
        }

        // Fall back to base entity sprite
        std::string basePath = levelDir + entityName + ".png";
        std::ifstream test(basePath);
        if (test.good()) return basePath;

        // If level-specific not found, try main Sprites directory (fallback)
        if (frame >= 0) {
            std::string path =
                SPRITE_BASE_PATH + entityName + std::to_string(frame) + ".png";
            std::ifstream test2(path);
            if (test2.good()) return path;
        }

        std::string mainPath = SPRITE_BASE_PATH + entityName + ".png";
        std::ifstream test3(mainPath);
        if (test3.good()) return mainPath;

        // If nothing found, return empty string to indicate missing sprite
        return "";
    };

    std::string path = resolve();
    if (path.empty()) {
        LOG_WARN(
            "GetEntitySpritePath: No sprite found for '{}' (frame: {}) in "
            "level '{}'",
            entityName, frame, levelName);
        return "";  // Return empty path, caller should handle this
    }

    return ProcessTransparent(path);
}

std::string SpritePathResolver::GetCastleSpritePathByID(int blockID) {
    // 8-4 castle sprites: ID 801-904 -> Sprites/8-4/tile_XXX.png
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
