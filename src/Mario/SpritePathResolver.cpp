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

#include <fstream>

namespace Mario {

const std::string SpritePathResolver::SPRITE_BASE_PATH =
    std::string(RESOURCE_DIR) + "/Sprites/";

std::string SpritePathResolver::GetSpritePath(const std::string& name,
                                               const std::string& suffix) {
    return SPRITE_BASE_PATH + name + suffix + ".png";
}

std::string SpritePathResolver::GetBlockSpritePath(const std::string& blockName,
                                                    int frame) {
    // C# tries: name + "0" as resource key
    // But actual files vary:
    //   "Ground.png" (no suffix), "BrickBlock.png" (no suffix)
    //   "Hill1.png" (number is part of name from CSV, not a frame)
    //   "QuestionBlock.png" (frame 0), "QuestionBlock1.png" (frame 1)

    if (frame == 0) {
        // First try the base name without any number suffix
        std::string basePath = SPRITE_BASE_PATH + blockName + ".png";
        std::ifstream test(basePath);
        if (test.good()) {
            return basePath;
        }
        // Then try with "0" suffix (some sprites use this)
        std::string path0 = SPRITE_BASE_PATH + blockName + "0.png";
        std::ifstream test0(path0);
        if (test0.good()) {
            return path0;
        }
        // Return base path anyway (will be caught by caller)
        return basePath;
    }

    // For animated frames > 0
    std::string framePath = SPRITE_BASE_PATH + blockName +
                            std::to_string(frame) + ".png";
    std::ifstream test(framePath);
    if (test.good()) {
        return framePath;
    }
    // Fallback to base
    return SPRITE_BASE_PATH + blockName + ".png";
}

std::string SpritePathResolver::GetPlayerSpritePath(const std::string& prefix,
                                                     int state, int frame) {
    // C# naming: "Mario" + prefix + state.ToString() + currentFrame
    // state=0: "MarioIdle00" -> file "MarioIdle.png" (no digits)
    // state=1: "MarioIdle10" -> file "MarioIdle1.png" (state only)
    // walk: state=0, frame=1: "MarioRight01" -> file "MarioRight1.png"
    // walk: state=1, frame=1: "MarioRight11" -> file "MarioRight11.png"

    std::string name = "Mario" + prefix;

    if (state > 0) {
        name += std::to_string(state);
    }
    if (frame > 0) {
        name += std::to_string(frame);
    }

    std::string fullPath = SPRITE_BASE_PATH + name + ".png";
    std::ifstream test(fullPath);
    if (test.good()) {
        return fullPath;
    }

    // Fallback: if no state/frame suffix worked, try just the prefix
    if (state == 0 && frame == 0) {
        return SPRITE_BASE_PATH + "Mario" + prefix + ".png";
    }

    // Fallback: try without frame
    if (frame > 0) {
        std::string noFrame = SPRITE_BASE_PATH + "Mario" + prefix;
        if (state > 0) noFrame += std::to_string(state);
        noFrame += ".png";
        std::ifstream test2(noFrame);
        if (test2.good()) return noFrame;
    }

    return fullPath;
}

std::string SpritePathResolver::GetEntitySpritePath(const std::string& entityName,
                                                     int frame) {
    if (frame >= 0) {
        std::string path = SPRITE_BASE_PATH + entityName +
                           std::to_string(frame) + ".png";
        std::ifstream test(path);
        if (test.good()) return path;
    }
    return SPRITE_BASE_PATH + entityName + ".png";
}

} // namespace Mario
