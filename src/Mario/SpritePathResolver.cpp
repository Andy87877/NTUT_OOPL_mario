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

#include "Util/Logger.hpp"

namespace Mario {

const std::string SpritePathResolver::SPRITE_BASE_PATH =
    std::string(RESOURCE_DIR) + "/Sprites/";

std::string SpritePathResolver::ProcessTransparent(
    const std::string& originalPath) {
    if (originalPath.empty()) return originalPath;

    // Process only PNG or BMP files
    size_t dotPos = originalPath.find_last_of('.');
    std::string ext =
        (dotPos != std::string::npos) ? originalPath.substr(dotPos) : ".png";
    std::string base = (dotPos != std::string::npos)
                           ? originalPath.substr(0, dotPos)
                           : originalPath;
    if (ext != ".png" && ext != ".bmp") return originalPath;

    std::string cachePath = base + "_tp.png";

    // Return cached transparent image if it already exists
    std::ifstream cacheFile(cachePath);
    if (cacheFile.good()) return cachePath;

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
        // Turn perfectly white or near-white pixels to fully transparent,
        // matching C#'s Color.White
        if (r >= 250 && g >= 250 && b >= 250) {
            pixels[i] = SDL_MapRGBA(rgbaSurface->format, 255, 255, 255, 0);
        }
    }

    if (IMG_SavePNG(rgbaSurface, cachePath.c_str()) != 0) {
        LOG_ERROR("ProcessTransparent failed to save PNG: {}", cachePath);
        SDL_FreeSurface(rgbaSurface);
        return originalPath;
    }

    SDL_FreeSurface(rgbaSurface);
    return cachePath;
}

std::string SpritePathResolver::GetSpritePath(const std::string& name,
                                              const std::string& suffix) {
    return SPRITE_BASE_PATH + name + suffix + ".png";
}

std::string SpritePathResolver::GetBlockSpritePath(const std::string& blockName,
                                                   int frame) {
    auto resolve = [&]() -> std::string {
        if (frame == 0) {
            std::string basePath = SPRITE_BASE_PATH + blockName + ".png";
            std::ifstream test(basePath);
            if (test.good()) return basePath;

            std::string path0 = SPRITE_BASE_PATH + blockName + "0.png";
            std::ifstream test0(path0);
            if (test0.good()) return path0;

            std::string strippedName = blockName;
            while (!strippedName.empty() && std::isdigit(strippedName.back())) {
                strippedName.pop_back();
            }

            if (strippedName != blockName) {
                std::string fallbackPath =
                    SPRITE_BASE_PATH + strippedName + ".png";
                std::ifstream testFallback(fallbackPath);
                if (testFallback.good()) return fallbackPath;
            }
            return basePath;
        }
        std::string framePath =
            SPRITE_BASE_PATH + blockName + std::to_string(frame) + ".png";
        std::ifstream test(framePath);
        if (test.good()) return framePath;
        return SPRITE_BASE_PATH + blockName + ".png";
    };
    return ProcessTransparent(resolve());
}

std::string SpritePathResolver::GetPlayerSpritePath(const std::string& prefix,
                                                    int state, int frame) {
    auto resolve = [&]() -> std::string {
        std::string name = "Mario" + prefix;
        if (state > 0) name += std::to_string(state);
        if (frame > 0) name += std::to_string(frame);

        std::string fullPath = SPRITE_BASE_PATH + name + ".png";
        std::ifstream test(fullPath);
        if (test.good()) return fullPath;

        if (state == 0 && frame == 0) {
            return SPRITE_BASE_PATH + "Mario" + prefix + ".png";
        }
        if (frame > 0) {
            std::string noFrame = SPRITE_BASE_PATH + "Mario" + prefix;
            if (state > 0) noFrame += std::to_string(state);
            noFrame += ".png";
            std::ifstream test2(noFrame);
            if (test2.good()) return noFrame;
        }
        return fullPath;
    };
    return ProcessTransparent(resolve());
}

std::string SpritePathResolver::GetEntitySpritePath(
    const std::string& entityName, int frame) {
    auto resolve = [&]() -> std::string {
        if (frame >= 0) {
            std::string path =
                SPRITE_BASE_PATH + entityName + std::to_string(frame) + ".png";
            std::ifstream test(path);
            if (test.good()) return path;
        }
        return SPRITE_BASE_PATH + entityName + ".png";
    };
    return ProcessTransparent(resolve());
}

}  // namespace Mario
