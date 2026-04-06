/**
 * @file GameConfig.hpp
 * @brief Global game configuration constants for Super Mario Bros.
 *        Contains physics, camera, tile size, and scale settings.
 * @inheritance None (static utility class)
 */
#ifndef MARIO_GAME_CONFIG_HPP
#define MARIO_GAME_CONFIG_HPP

namespace Mario {

/**
 * Central configuration for the entire game.
 * All constants are compile-time to avoid magic numbers.
 */
struct GameConfig {
    // -- Tile & Scale --
    static constexpr int TILE_SIZE = 32;        // Pixels per tile (scaled from 16px originals)
    static constexpr float SCALE_FACTOR = 2.0f; // Sprite scale multiplier
    static constexpr int ORIGINAL_TILE = 16;    // Original NES tile size

    // -- Viewport (in tiles) --
    static constexpr int VIEWPORT_TILES_X = 25; // Visible tiles horizontally (800/32)
    static constexpr int VIEWPORT_TILES_Y = 15; // Visible tiles vertically (480/32)

    // -- Window --
    static constexpr int WINDOW_WIDTH  = VIEWPORT_TILES_X * TILE_SIZE;  // 800
    static constexpr int WINDOW_HEIGHT = VIEWPORT_TILES_Y * TILE_SIZE;  // 480

    // -- Timing --
    static constexpr float TICK_INTERVAL = 20.0f / 1000.0f; // 20ms per tick (50 FPS)
    static constexpr int TIME_SUB_LIMIT = 40;  // Ticks per game-second

    // -- Player --
    static constexpr float BASE_SPEED = 7.35f;  // Base Mario speed (tiles/sec)
    static constexpr float RUN_MULTIPLIER = 1.25f;
    static constexpr float SCALED_SPEED = BASE_SPEED * TILE_SIZE * TICK_INTERVAL;

    // -- Physics --
    static constexpr float GRAVITY = 9.81f;
    static constexpr float JUMP_VELOCITY = 9.81f;
    static constexpr float JUMP_HIGH_VELOCITY = 19.62f;
    static constexpr float JUMP_LOW_VELOCITY = 6.0f;

    // -- Entity --
    static constexpr float ENEMY_SPEED_DIVISOR = 3.0f;
    static constexpr float SHELL_SPEED_MULTIPLIER = 1.5f;

    // -- Collision --
    static constexpr float INTERSECT_STRICTNESS = 0.75f;
    static constexpr float HITBOX_WIDTH_RATIO = 0.6875f;  // Player hitbox is narrower

    // -- UI --
    static constexpr int INITIAL_LIVES = 3;
    static constexpr int INITIAL_TIME = 400;
    static constexpr int FLAG_SPEED = 2;

    // -- Z-Index Layers --
    static constexpr float Z_BACKGROUND = -10.0f;
    static constexpr float Z_BLOCK      = -5.0f;
    static constexpr float Z_PLAYER     = 0.0f;
    static constexpr float Z_ENTITY     = 5.0f;
    static constexpr float Z_EFFECT     = 10.0f;
    static constexpr float Z_UI         = 90.0f;

    // -- Special Block IDs --
    static constexpr int MARIO_SPAWN_ID = 999;
    static constexpr int FLAG_ENTITY_ID = 29;
    static constexpr int UNDER_COIN_ID  = 49;
    static constexpr int PIPE_EXIT_ID   = 53;
};

} // namespace Mario

#endif // MARIO_GAME_CONFIG_HPP
