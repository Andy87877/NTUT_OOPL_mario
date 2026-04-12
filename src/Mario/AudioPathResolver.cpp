/**
 * @file AudioPathResolver.cpp
 * @brief Implementation of audio path resolution.
 * @inheritance None
 */
#include "Mario/AudioPathResolver.hpp"

#include "config.hpp"

namespace Mario {

std::string AudioPathResolver::GetBGMPath(const std::string& filename) {
    return std::string(RESOURCE_DIR) + BGM_SUBDIR + filename;
}

std::string AudioPathResolver::GetSFXPath(const std::string& filename) {
    return std::string(RESOURCE_DIR) + SFX_SUBDIR + filename;
}

}  // namespace Mario
