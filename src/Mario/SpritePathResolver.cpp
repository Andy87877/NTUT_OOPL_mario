/**
 * @file SpritePathResolver.cpp
 * @brief Implementation of sprite path resolution.
 *        Handles the C# reference's inconsistent sprite naming conventions.
 * @inheritance None
 */
#include "Mario/SpritePathResolver.hpp"

#include <fstream>

namespace Mario {

// RESOURCE_DIR is defined by CMake at compile time
const std::string SpritePathResolver::SPRITE_BASE_PATH =
    std::string(RESOURCE_DIR) + "/Sprites/";

std::string SpritePathResolver::GetSpritePath(const std::string& name,
                                               const std::string& suffix) {
    return SPRITE_BASE_PATH + name + suffix + ".png";
}

std::string SpritePathResolver::GetBlockSpritePath(const std::string& blockName,
                                                    int frame) {
    // C# sprites have inconsistent naming:
    //   "Ground.png" (no number), "BrickBlock.png", "BrickBlock1.png"
    //   "QuestionBlock.png", "QuestionBlock1.png", "QuestionBlock2.png", etc.
    // Try with frame number first, then without
    if (frame == 0) {
        // First try the base name without number
        std::string basePath = SPRITE_BASE_PATH + blockName + ".png";
        std::ifstream test(basePath);
        if (test.good()) {
            return basePath;
        }
    }
    return SPRITE_BASE_PATH + blockName + std::to_string(frame) + ".png";
}

std::string SpritePathResolver::GetPlayerSpritePath(const std::string& prefix,
                                                     int state, int frame) {
    return SPRITE_BASE_PATH + "Mario" + prefix + std::to_string(state) +
           std::to_string(frame) + ".png";
}

std::string SpritePathResolver::GetEntitySpritePath(const std::string& entityName,
                                                     int frame) {
    if (frame >= 0) {
        // Try with frame number for animated entities
        std::string path = SPRITE_BASE_PATH + entityName +
                           std::to_string(frame + 1) + ".png";
        std::ifstream test(path);
        if (test.good()) return path;

        // Also try zero-indexed
        path = SPRITE_BASE_PATH + entityName + std::to_string(frame) + ".png";
        std::ifstream test2(path);
        if (test2.good()) return path;
    }
    return SPRITE_BASE_PATH + entityName + ".png";
}

} // namespace Mario
