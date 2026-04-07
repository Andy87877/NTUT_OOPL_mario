/**
 * @file GameConfig.hpp
 * @brief Global game configuration constants for Super Mario Bros.
 *        Contains physics, camera, tile size, and scale settings.
 *        Window dimensions must match PTSD config.hpp (1280x720).
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
    static constexpr int TILE_SIZE =
        32;  // Pixels per tile (scaled from 16px originals)
    static constexpr float SCALE_FACTOR = 2.0f;  // Sprite scale multiplier
    static constexpr int ORIGINAL_TILE = 16;     // Original NES tile size

    // -- PTSD Window (must match PTSD config.hpp) --
    static constexpr int WINDOW_WIDTH = 1280;
    static constexpr int WINDOW_HEIGHT = 720;

    // -- Viewport (in tiles, based on window) --
    static constexpr int VIEWPORT_TILES_X = WINDOW_WIDTH / TILE_SIZE;  // 40
    static constexpr int VIEWPORT_TILES_Y =
        WINDOW_HEIGHT / TILE_SIZE;  // 22 (720/32)
    static constexpr float RENDER_Y_OFFSET =
        -104.0f;  // Offset to align 512px level bottom to 720px window bottom

    // -- Level dimensions --
    static constexpr int LEVEL_ROWS = 16;  // NES levels are 16 tiles tall
    static constexpr int LEVEL_HEIGHT_PX = LEVEL_ROWS * TILE_SIZE;  // 512px

    // -- Timing --
    static constexpr float TICK_INTERVAL =
        20.0f / 1000.0f;                       // 20ms per tick (50 FPS)
    static constexpr int TIME_SUB_LIMIT = 40;  // Ticks per game-second

    // -- Player --
    static constexpr float BASE_SPEED = 7.35f;  // Base Mario speed (tiles/sec)
    static constexpr float RUN_MULTIPLIER = 1.25f;
    static constexpr float SCALED_SPEED =
        BASE_SPEED * TILE_SIZE * TICK_INTERVAL;

    // -- Physics --
    static constexpr float GRAVITY = 9.81f;
    static constexpr float GRAVITY_ACCELERATION =
        5.0f;  // For collision ground detection
    static constexpr float JUMP_VELOCITY = 9.81f;
    static constexpr float JUMP_HIGH_VELOCITY = 19.62f;
    static constexpr float JUMP_LOW_VELOCITY = 6.0f;
    static constexpr double MUSHROOM_BOUNCE_HEIGHT =
        15.0;  // Height for spawned mushroom bounce
    static constexpr double FIREBALL_BOUNCE_HEIGHT =
        20.0;  // Height for fireball bounce

    // -- Entity --
    static constexpr float ENEMY_SPEED_DIVISOR = 3.0f;
    static constexpr float SHELL_SPEED_MULTIPLIER = 1.5f;

    // -- Collision --
    static constexpr float INTERSECT_STRICTNESS = 0.75f;
    static constexpr float HITBOX_WIDTH_RATIO =
        0.6875f;  // Player hitbox is narrower

    // -- UI --
    static constexpr int INITIAL_LIVES = 3;
    static constexpr int INITIAL_TIME = 400;
    static constexpr int FLAG_SPEED = 2;

    // -- Z-Index Layers --
    static constexpr float Z_BACKGROUND = -10.0f;
    static constexpr float Z_BLOCK = -5.0f;
    static constexpr float Z_PLAYER = 0.0f;
    static constexpr float Z_ENTITY = 5.0f;
    static constexpr float Z_EFFECT = 10.0f;
    static constexpr float Z_UI = 90.0f;

    // -- Special Block IDs --
    static constexpr int MARIO_SPAWN_ID = 999;
    static constexpr int FLAG_ENTITY_ID = 29;
    static constexpr int UNDER_COIN_ID = 49;
    static constexpr int PIPE_EXIT_ID = 53;

    // Pipe entrance block IDs (from C# Form1.cs lines 802-827)
    static constexpr int PIPE_DOWN_LEFT = 51;   // Left half of downward pipe
    static constexpr int PIPE_DOWN_RIGHT = 52;  // Right half of downward pipe
    static constexpr int PIPE_RIGHT_TOP = 42;   // Top half of rightward pipe
    static constexpr int PIPE_RIGHT_BOT = 43;   // Bottom half of rightward pipe

    // -- Flagpole / Ending --
    static constexpr int FLAGPOLE_SLIDE_SPEED = 2;  // Pixels per tick
    static constexpr int ENDING_WALK_DELAY =
        20;  // Ticks after grounded before walking
    static constexpr int LEVEL_TRANSITION_DELAY =
        50;                                    // Ticks before next level loads
    static constexpr int PIPE_ANIM_SPEED = 5;  // Mario descend speed in pipe
};

}  // namespace Mario

#endif  // MARIO_GAME_CONFIG_HPP
