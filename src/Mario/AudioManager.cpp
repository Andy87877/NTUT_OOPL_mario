/**
 * @file AudioManager.cpp
 * @brief Implementation of AudioManager (BGM + SFX service) and
 *        AudioPathResolver (static path-building helper).
 *        AudioPathResolver is an internal detail of AudioManager;
 *        the two are compiled together to keep the audio subsystem in one file.
 * @inheritance IAudioService <- AudioManager
 *              None          <- AudioPathResolver (static helper)
 */
#include "Mario/AudioManager.hpp"

#include <algorithm>

#include "Util/Logger.hpp"
#include "config.hpp"

namespace Mario {

AudioManager& AudioManager::GetInstance() {
    static AudioManager instance;
    return instance;
}

AudioManager::AudioManager()
    : m_Volume(1.0f), m_Muted(false), m_BGMStarted(false) {
    LOG_INFO("AudioManager initialized - Volume: {}, Muted: {}", m_Volume,
             m_Muted);
    LOG_INFO("AudioManager ready to play audio files from Resources/Audio/");
}

void AudioManager::PlayBGM(BGMName name) {
    LOG_DEBUG("PlayBGM() called with name={}", static_cast<int>(name));

    // Boundary check for BGM enum value
    int bgmIndex = static_cast<int>(name);
    if (bgmIndex < 0 || bgmIndex >= static_cast<int>(BGMPaths.size())) {
        LOG_ERROR("PlayBGM: Invalid BGM index {}! Must be 0-{}", bgmIndex,
                  BGMPaths.size() - 1);
        return;
    }

    LOG_DEBUG("Current state: m_BGMStarted={}, m_Muted={}, m_Volume={}",
              m_BGMStarted, m_Muted, m_Volume);

    // If already playing this exact BGM, don't restart it
    if (m_CurrentBGM.has_value() && m_CurrentBGM.value() == name &&
        m_BGMStarted) {
        LOG_DEBUG("BGM {} already playing, skipping restart",
                  static_cast<int>(name));
        return;
    }

    // Stop previous BGM if switching to a different one
    if (m_CurrentBGM.has_value() && m_CurrentBGM.value() != name) {
        LOG_DEBUG("Stopping previous BGM: {}",
                  static_cast<int>(m_CurrentBGM.value()));
        StopBGM();
    }

    try {
        // Load BGM if not cached
        if (m_BGMs.find(name) == m_BGMs.end()) {
            // Extract filename from the relative path in AudioType.hpp
            std::string relativePath = BGMPaths[static_cast<int>(name)];
            std::string filename =
                relativePath.substr(relativePath.find_last_of("/\\") + 1);

            // Construct full absolute path using AudioPathResolver
            std::string fullPath = AudioPathResolver::GetBGMPath(filename);
            LOG_INFO("Loading BGM: {} from path: {}", static_cast<int>(name),
                     fullPath);
            m_BGMs[name] = std::make_shared<Util::BGM>(fullPath);
            LOG_DEBUG("BGM object created");
        }

        // Check if BGM was loaded successfully
        if (!m_BGMs[name]) {
            LOG_ERROR("BGM pointer is null after creation for BGM: {}",
                      static_cast<int>(name));
            m_CurrentBGM.reset();
            m_BGMStarted = false;
            return;
        }

        LOG_DEBUG("BGM is valid, proceeding to play");

        // Mark as current BGM
        m_CurrentBGM = name;

        // Set volume
        int volume = m_Muted ? 0 : static_cast<int>(m_Volume * 128.0f);
        LOG_DEBUG("Setting BGM volume: {}", volume);
        m_BGMs[name]->SetVolume(volume);

        // Call Play() only if not yet started
        if (!m_BGMStarted) {
            LOG_DEBUG("BGM not started yet, m_BGMStarted={}", m_BGMStarted);
            if (!m_Muted && volume > 0) {
                LOG_INFO(">>> CALLING m_BGMs[name]->Play(-1) <<<");
                m_BGMs[name]->Play(-1);  // -1 = infinite loop
                m_BGMStarted = true;
                LOG_INFO(
                    ">>> BGM PLAY COMPLETE - if you don't hear music, check "
                    "system volume <<<");
            } else {
                LOG_WARN("NOT playing BGM: m_Muted={}, volume={}", m_Muted,
                         volume);
            }
        } else {
            LOG_DEBUG(
                "BGM already started (m_BGMStarted=true), skipping Play() "
                "call");
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Exception in PlayBGM: {}", e.what());
        m_BGMStarted = false;
        m_CurrentBGM.reset();
    }
}

void AudioManager::StopBGM() {
    if (m_CurrentBGM.has_value()) {
        LOG_DEBUG("Stopping BGM: {}", static_cast<int>(m_CurrentBGM.value()));
        auto bgm = m_BGMs.find(m_CurrentBGM.value());
        if (bgm != m_BGMs.end()) {
            bgm->second->FadeOut(100);  // Fade out over 100ms
        }
        m_BGMStarted = false;
        m_CurrentBGM.reset();
    }
}

void AudioManager::PlaySFX(SFXName name) {
    if (m_Muted) return;

    try {
        if (m_SFXs.find(name) == m_SFXs.end()) {
            // Extract filename from the relative path in AudioType.hpp
            std::string relativePath = SFXPaths[static_cast<int>(name)];
            std::string filename =
                relativePath.substr(relativePath.find_last_of("/\\") + 1);

            // Construct full absolute path using AudioPathResolver
            std::string fullPath = AudioPathResolver::GetSFXPath(filename);
            LOG_DEBUG("Loading SFX: {} from {}", static_cast<int>(name),
                      fullPath);
            m_SFXs[name] = std::make_shared<Util::SFX>(fullPath);
        }

        // Check if SFX was loaded successfully
        if (!m_SFXs[name]) {
            LOG_ERROR("Failed to load SFX: {} (ptr is null)",
                      static_cast<int>(name));
            return;
        }

        int volume = static_cast<int>(m_Volume * 128.0f);
        m_SFXs[name]->SetVolume(volume);

        if (volume > 0 && !m_Muted) {
            LOG_DEBUG("Playing SFX: {} (volume: {})", static_cast<int>(name),
                      volume);
            m_SFXs[name]->Play(0);  // 0 = play once, no loop
        }
    } catch (const std::exception& e) {
        LOG_WARN("Exception while playing SFX {}: {}", static_cast<int>(name),
                 e.what());
    }
}

void AudioManager::SetVolume(float volume) {
    m_Volume = std::max(0.0f, std::min(1.0f, volume));
    LOG_DEBUG("Audio volume set to: {}", m_Volume);

    if (m_CurrentBGM.has_value() &&
        m_BGMs.find(m_CurrentBGM.value()) != m_BGMs.end()) {
        int v = m_Muted ? 0 : static_cast<int>(m_Volume * 128.0f);
        m_BGMs[m_CurrentBGM.value()]->SetVolume(v);
    }
}

void AudioManager::SetMuted(bool muted) {
    if (m_Muted == muted) return;

    m_Muted = muted;
    LOG_INFO("Audio muted: {}", muted ? "true" : "false");

    if (m_BGMStarted && m_CurrentBGM.has_value()) {
        BGMName bgmName = m_CurrentBGM.value();
        if (m_BGMs.find(bgmName) != m_BGMs.end()) {
            if (m_Muted) {
                m_BGMs[bgmName]->FadeOut(100);
            } else {
                int v = static_cast<int>(m_Volume * 128.0f);
                m_BGMs[bgmName]->SetVolume(v);
                if (v > 0) {
                    m_BGMs[bgmName]->Play(-1);
                }
            }
        }
    }
}

// ============================================================================
// AudioPathResolver — implementation merged here (AudioManager is the sole
// consumer; keeping them in one file reduces compiled-unit overhead)
// ============================================================================

std::string AudioPathResolver::GetBGMPath(const std::string& filename) {
    return std::string(RESOURCE_DIR) + BGM_SUBDIR + filename;
}

std::string AudioPathResolver::GetSFXPath(const std::string& filename) {
    return std::string(RESOURCE_DIR) + SFX_SUBDIR + filename;
}

// ============================================================================
// PlayBGMForLevel — selects and plays the correct BGM for a given level.
// This is the single authoritative place that maps level names to BGM tracks,
// so App and scene handlers never need level-name strings in audio logic.
// ============================================================================
void AudioManager::PlayBGMForLevel(const std::string& levelName,
                                   int timeRemaining) {
    bool hurry = timeRemaining <= 100 && timeRemaining > 0;
    BGMName bgm;
    if (levelName == "8-4" || levelName == "8-4_sub") {
        bgm = hurry ? BGMName::CastleThemeHurryUp : BGMName::CastleTheme;
    } else if (levelName == "1-1u" || levelName == "1-2" ||
               levelName == "1-2uu" || levelName == "1-2_sub") {
        bgm = hurry ? BGMName::UndergroundThemeHurryUp
                    : BGMName::UndergroundTheme;
    } else {
        bgm = hurry ? BGMName::GroundThemeHurryUp : BGMName::GroundTheme;
    }
    PlayBGM(bgm);
}

}  // namespace Mario
