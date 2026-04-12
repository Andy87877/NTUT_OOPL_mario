/**
 * @file AudioManager.hpp
 * @brief Audio management service for BGM and SFX playback.
 * @inheritance IAudioService <- AudioManager
 */
#ifndef MARIO_AUDIO_MANAGER_HPP
#define MARIO_AUDIO_MANAGER_HPP

#include <map>
#include <memory>
#include <optional>

#include "Mario/IAudioService.hpp"
#include "Util/BGM.hpp"
#include "Util/SFX.hpp"

namespace Mario {

/**
 * Concrete implementation of audio service.
 * Handles BGM (background music) and SFX (sound effects) playback.
 * Uses lazy loading to instantiate BGM and SFX.
 *
 * Key mechanism: Only calls Play() when switching or starting BGM.
 * Avoids continuous restarts of the same BGM.
 */
class AudioManager : public IAudioService {
   public:
    static AudioManager& GetInstance();

    void PlayBGM(BGMName name) override;
    void StopBGM() override;
    void PlaySFX(SFXName name) override;
    void SetVolume(float volume) override;
    void SetMuted(bool muted) override;

   private:
    float m_Volume = 1.0f;
    bool m_Muted = false;
    std::optional<BGMName> m_CurrentBGM;
    bool m_BGMStarted = false;  // Track if current BGM has been started

    std::map<BGMName, std::shared_ptr<Util::BGM>> m_BGMs;
    std::map<SFXName, std::shared_ptr<Util::SFX>> m_SFXs;

    // Singleton constructor
    AudioManager();
    ~AudioManager() override = default;
};

}  // namespace Mario

#endif  // MARIO_AUDIO_MANAGER_HPP
