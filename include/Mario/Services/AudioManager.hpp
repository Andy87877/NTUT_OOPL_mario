/**
 * @file AudioManager.hpp
 * @brief Concrete singleton implementation of the audio subsystem.
 *        Manages looping BGMs and one-shot SFXs via PTSD.
 * @inheritance IAudioService <- AudioManager
 */
#ifndef MARIO_AUDIO_MANAGER_HPP
#define MARIO_AUDIO_MANAGER_HPP

#include <map>
#include <memory>
#include <optional>
#include <string>

#include "Mario/Services/IAudioService.hpp"
#include "Mario/Services/AudioType.hpp"
#include "Util/BGM.hpp"
#include "Util/SFX.hpp"

namespace Mario {

// ============================================================================
// AudioPathResolver
// Static helper — builds absolute paths from RESOURCE_DIR.
// Implementation is in AudioManager.cpp.
// ============================================================================
/**
 * Builds full filesystem paths for audio files.
 * @inheritance None (static utility)
 */
class AudioPathResolver {
   public:
    static std::string GetBGMPath(const std::string& filename);
    static std::string GetSFXPath(const std::string& filename);

   private:
    AudioPathResolver() = delete;
    static constexpr const char* BGM_SUBDIR = "/Audio/BGM/";
    static constexpr const char* SFX_SUBDIR = "/Audio/SFX/";
};

// ============================================================================
// AudioManager  — concrete singleton implementation
// ============================================================================
/**
 * Singleton audio service: handles BGM looping and one-shot SFX via PTSD.
 * @inheritance IAudioService <- AudioManager
 */
class AudioManager : public IAudioService {
   public:
    static AudioManager& GetInstance();

    void PlayBGM(BGMName name) override;
    void StopBGM() override;
    void PlaySFX(SFXName name) override;
    void SetVolume(float volume) override;
    void SetMuted(bool muted) override;
    void PlayBGMForLevel(const std::string& levelName,
                         int timeRemaining) override;

   private:
    float m_Volume = 1.0f;
    bool m_Muted = false;
    std::optional<BGMName> m_CurrentBGM;
    bool m_BGMStarted = false;

    std::map<BGMName, std::shared_ptr<Util::BGM>> m_BGMs;
    std::map<SFXName, std::shared_ptr<Util::SFX>> m_SFXs;

    AudioManager();
    ~AudioManager() override = default;
};

}  // namespace Mario

#endif  // MARIO_AUDIO_MANAGER_HPP
