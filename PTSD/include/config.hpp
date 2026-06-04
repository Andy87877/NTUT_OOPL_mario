/**
 * @file config.hpp
 * @brief Global configuration parameters for the PTSD engine.
 * @inheritance None
 */
#ifndef CONFIG_H
#define CONFIG_H

#include "Util/Logger.hpp"
#include "pch.hpp"  // IWYU pragma: export

constexpr const char* TITLE = "PTSD - 113820033_電資二_謝奕宏 - SuperMarioBros";

constexpr int WINDOW_POS_X = SDL_WINDOWPOS_UNDEFINED;
constexpr int WINDOW_POS_Y = SDL_WINDOWPOS_UNDEFINED;

constexpr unsigned int WINDOW_WIDTH = 1280;
constexpr unsigned int WINDOW_HEIGHT = 720;

constexpr Util::Logger::Level DEFAULT_LOG_LEVEL = Util::Logger::Level::DEBUG;

/**
 * @brief FPS limit
 *
 * Set value to 0 to turn off FPS cap
 */
constexpr unsigned int FPS_CAP = 60;

#endif
