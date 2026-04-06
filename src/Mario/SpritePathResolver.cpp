/**
 * @file SpritePathResolver.cpp
 * @brief Implementation of sprite path resolution.
 * @inheritance None
 */
#include "Mario/SpritePathResolver.hpp"

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
    return SPRITE_BASE_PATH + blockName + std::to_string(frame) + ".png";
}

std::string SpritePathResolver::GetPlayerSpritePath(const std::string& prefix,
                                                     int state, int frame) {
    // Player sprite naming: "Mario" + prefix + state + frame
    // e.g., "MarioIdle00", "MarioRight10", "MarioJump20"
    return SPRITE_BASE_PATH + "Mario" + prefix + std::to_string(state) +
           std::to_string(frame) + ".png";
}

std::string SpritePathResolver::GetEntitySpritePath(const std::string& entityName,
                                                     int frame) {
    if (frame >= 0) {
        return SPRITE_BASE_PATH + entityName + std::to_string(frame) + ".png";
    }
    return SPRITE_BASE_PATH + entityName + ".png";
}

} // namespace Mario
