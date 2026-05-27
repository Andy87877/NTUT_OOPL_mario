/**
 * @file LevelManager.cpp
 * @brief Concrete Level Service implementation.
 *
 * @inheritance ILevelService <- LevelManager
 */
#include "Mario/Services/LevelManager.hpp"
#include "App.hpp"
#include "Mario/Level/EntityDef.hpp"
#include "Mario/Level/EntityFactory.hpp"
#include "Mario/Services/AudioManager.hpp"
#include "Mario/Services/ServiceLocator.hpp"
#include "Util/Logger.hpp"

#include <GL/glew.h>

namespace Mario {

LevelManager::LevelManager() {
    m_Level = nullptr;
    m_Player = nullptr;
    m_Entities.clear();
    m_FlagEntity = nullptr;
    m_CurrentLevelName = "";
}

void LevelManager::LoadLevel(App& app, const std::string& levelName) {
    app.GetCamera().Reset();
    m_FlagEntity = nullptr;
    m_CurrentLevelName = levelName;

    // Create and load the level
    m_Level = std::make_shared<Mario::Level>();
    if (!m_Level->Load(levelName)) {
        LOG_ERROR("Failed to load level: {}", levelName);
        app.TransitionTo(App::State::TITLE);
        return;
    }

    // Create player at spawn position from level CSV
    float spawnX = m_Level->GetPlayerSpawnX();
    float spawnY = m_Level->GetPlayerSpawnY();

    // Restore saved power state across levels
    int savedState = app.GetGameState().GetSavedPowerState();
    m_Player = std::make_shared<Mario::Player>(spawnX, spawnY, savedState);

    // Spawn entities (Goomba, KoopaTroopa, etc.) from level data
    m_Entities = Mario::EntityFactory::SpawnFromLevel(*m_Level);

    // Look for the Flag entity in spawn list
    for (auto& entity : m_Entities) {
        if (entity && entity->GetDef().type == Mario::EntityType::FLAG) {
            m_FlagEntity = entity;
            break;
        }
    }

    // Build renderer: clear and add all blocks + player + entities.
    app.GetRenderer() = Util::Renderer();
    for (const auto& block : m_Level->GetAllBlocks()) {
        app.GetRenderer().AddChild(block);
    }
    app.GetRenderer().AddChild(m_Player);
    for (const auto& entity : m_Entities) {
        app.GetRenderer().AddChild(entity);
    }

    // Hide all game-world objects during the loading/transition screen
    m_Player->SetVisible(false);
    for (const auto& entity : m_Entities) {
        entity->SetVisible(false);
    }

    // Apply appropriate background color
    ApplyBackground(app);

    LOG_INFO("Level {} loaded: {} blocks, {} entities, player at ({}, {})",
             levelName, m_Level->GetAllBlocks().size(), m_Entities.size(),
             spawnX, spawnY);
}

void LevelManager::StartLevel(App& app) {
    app.GetGameState().ResetTime();
    app.GetGameState().StartTime();

    // Reveal game-world objects hidden during the loading screen
    if (m_Player) {
        m_Player->SetVisible(true);
        m_Player->GetState().SetControllable(true);
    }
    for (const auto& entity : m_Entities) {
        entity->SetVisible(true);
    }

    PlayCurrentBGM(app);
}

void LevelManager::PlayCurrentBGM(App& app) {
    if (m_Player && m_Player->GetState().IsStar()) {
        bool hurry = app.GetGameState().GetTimeRemaining() <= 100;
        Mario::AudioManager::GetInstance().PlayBGM(
            hurry ? Mario::BGMName::InvincibilityThemeHurryUp
                  : Mario::BGMName::InvincibilityTheme);
    } else {
        // AudioManager is singleton,AudioManager manages BGM
        Mario::AudioManager::GetInstance().PlayBGMForLevel(
            m_CurrentLevelName, app.GetGameState().GetTimeRemaining());
    }
}

void LevelManager::AddEntityToGame(App& app, std::shared_ptr<Entity> entity) {
    if (!entity) return;
    app.GetRenderer().AddChild(entity);
    m_Entities.push_back(entity);
}

bool LevelManager::IsUnderground(App& app) const {
    if (app.GetGameState().IsUnderground()) return true;
    if (m_Level) return m_Level->IsUnderground();
    return false;
}

void LevelManager::ApplyBackground(App& app) {
    ApplyBackground(app, IsUnderground(app));
}

void LevelManager::ApplyBackground(App& app, bool isUnderground) {
    if (isUnderground) {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  // Castle / dungeon: black
    } else {
        glClearColor(92.0f / 255.0f, 148.0f / 255.0f, 252.0f / 255.0f,
                     0.0f);  // Surface: sky blue
    }
}

void LevelManager::AdvanceToNextLevel(App& app) {
    std::string nextLevel = app.GetGameState().AdvanceLevel();
    if (app.GetGameState().IsGameWon()) {
        LOG_INFO("Game Complete! Entering victory screen.");
        app.TransitionTo(App::State::GAME_WON);
        return;
    }
    LOG_INFO("Advancing to level {}", nextLevel);
    m_Loading = false;
    app.TransitionTo(App::State::LOADING);
}

}  // namespace Mario
