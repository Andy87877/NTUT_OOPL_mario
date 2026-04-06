/**
 * @file SpritePathResolver.hpp
 * @brief Utility to resolve sprite image file paths from category/name/frame.
 * @inheritance None (static utility)
 */
#ifndef MARIO_SPRITE_PATH_RESOLVER_HPP
#define MARIO_SPRITE_PATH_RESOLVER_HPP

#include <string>

namespace Mario {

/**
 * Resolves sprite file paths within the Resources directory.
 * All sprite images are stored under Resources/Sprites/.
 */
class SpritePathResolver {
public:
    /**
     * Get the full path to a sprite image.
     * @param name Sprite name (e.g., "Ground", "MarioIdle")
     * @param suffix Optional suffix like frame number (e.g., "0", "1")
     * @return Full path string for Util::Image
     */
    static std::string GetSpritePath(const std::string& name,
                                     const std::string& suffix = "");

    /**
     * Get path for a block sprite by looking up the block name.
     * @param blockName Block sprite name from IDList.csv
     * @param frame Frame number for animated blocks
     * @return Full path string
     */
    static std::string GetBlockSpritePath(const std::string& blockName,
                                          int frame = 0);

    /**
     * Get path for a player sprite.
     * @param prefix Animation prefix (e.g., "Idle", "Right", "Jump")
     * @param state Player state (0=small, 1=big, 2=fire)
     * @param frame Frame number
     * @return Full path string
     */
    static std::string GetPlayerSpritePath(const std::string& prefix,
                                           int state, int frame);

    /**
     * Get path for an entity sprite.
     * @param entityName Entity name from EntityList.csv
     * @param frame Frame number
     * @return Full path string
     */
    static std::string GetEntitySpritePath(const std::string& entityName,
                                           int frame = 0);

private:
    static const std::string SPRITE_BASE_PATH;
};

} // namespace Mario

#endif // MARIO_SPRITE_PATH_RESOLVER_HPP
