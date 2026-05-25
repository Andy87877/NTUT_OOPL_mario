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
     * Checks the level-specific subdirectory first (if levelName is non-empty),
     * then falls back to the root Sprites/ directory.
     * @param blockName Block sprite name from IDList.csv
     * @param frame Frame number for animated blocks
     * @param levelName Level name for directory lookup (e.g. "1-1", "1-2")
     * @return Full path string
     */
    static std::string GetBlockSpritePath(const std::string& blockName,
                                          int frame = 0,
                                          const std::string& levelName = "");

    /**
     * Get path for a player sprite.
     * @param prefix Animation prefix (e.g., "Idle", "Right", "Jump")
     * @param state Player state (0=small, 1=big, 2=fire)
     * @param frame Frame number
     * @return Full path string
     */
    static std::string GetPlayerSpritePath(const std::string& prefix, int state,
                                           int frame, int starState = 0);

    /**
     * Get path for an entity sprite.
     * @param entityName Entity name from EntityList.csv
     * @param frame Frame number
     * @param levelName Level name for directory mapping (e.g., "1-1", "8-4")
     * @return Full path string
     */
    static std::string GetEntitySpritePath(
        const std::string& entityName, int frame = 0,
        const std::string& levelName = "1-1");

    /**
     * Get path for 8-4 castle sprites by block ID (801-904).
     * Maps directly to Sprites/8-4/tile_XXX.png
     * @param blockID Block ID from 8-4 level (801-904)
     * @return Full path string, or empty if ID out of range
     */
    static std::string GetCastleSpritePathByID(int blockID);

   private:
    static const std::string SPRITE_BASE_PATH;
};

}  // namespace Mario

#endif  // MARIO_SPRITE_PATH_RESOLVER_HPP
