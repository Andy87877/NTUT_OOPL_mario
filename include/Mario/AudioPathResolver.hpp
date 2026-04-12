/**
 * @file AudioPathResolver.hpp
 * @brief Audio path resolution utility using RESOURCE_DIR macro.
 * @inheritance None
 */
#ifndef MARIO_AUDIO_PATH_RESOLVER_HPP
#define MARIO_AUDIO_PATH_RESOLVER_HPP

#include <string>

namespace Mario {

/**
 * Resolves audio file paths using the RESOURCE_DIR macro.
 * Ensures audio files are found regardless of working directory.
 */
class AudioPathResolver {
   public:
    /**
     * Get the full path to a BGM file.
     * @param bgmIndex Index of BGM from BGMName enum
     * @param filename BGM filename (e.g., "01. Ground Theme.wav")
     * @return Full absolute path to BGM file
     */
    static std::string GetBGMPath(const std::string& filename);

    /**
     * Get the full path to an SFX file.
     * @param sfxIndex Index of SFX from SFXName enum
     * @param filename SFX filename (e.g., "12. Jump.wav")
     * @return Full absolute path to SFX file
     */
    static std::string GetSFXPath(const std::string& filename);

   private:
    AudioPathResolver() = delete;

    static constexpr const char* BGM_SUBDIR = "/Audio/BGM/";
    static constexpr const char* SFX_SUBDIR = "/Audio/SFX/";
};

}  // namespace Mario

#endif  // MARIO_AUDIO_PATH_RESOLVER_HPP
