/**
 * @file App.cpp
 * @brief Implementation of the main application controller.
 *        Handles game state machine, integrates Level loading, Player MVC,
 *        collision detection, camera following, flagpole/pipe sequences.
 *        Game loop logic ported from C# Form1.cs onTick().
 * @inheritance None (top-level controller)
 */
#include "App.hpp"

#include "Mario/PhysicsEngine.hpp"
#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

// OpenGL include for background clearing
#include <GL/glew.h>

// ============================================================================
// Start: Initialize game, transition to TITLE
// ============================================================================
void App::Start() {
    LOG_TRACE("Start");

    // Initialize UI Manager (manages UI state and floating texts)
    m_UIManager = std::make_unique<Mario::UIManager>(&m_GameState);

    m_CurrentState = State::TITLE;
    LOG_INFO("Game initialized - entering TITLE state");
}

// ============================================================================
// Update: Main loop dispatcher based on current state
// ============================================================================
void App::Update() {
    m_Timer++;

    switch (m_CurrentState) {
        case State::TITLE:
            UpdateTitle();
            break;
        case State::LOADING:
            UpdateLoading();
            break;
        case State::PLAYING:
            UpdatePlaying();
            break;
        case State::FLAGPOLE:
            UpdateFlagpole();
            break;
        case State::PIPE_WARP:
            UpdatePipeWarp();
            break;
        case State::DEATH:
            UpdateDeath();
            break;
        case State::GAME_OVER:
            UpdateGameOver();
            break;
        case State::ESC_MENU:
            UpdateESCMenu();
            break;
        default:
            break;
    }

    // Render the scene
    RenderAll();

    // Global exit check
    if (Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

// ============================================================================
// State Handlers
// ============================================================================

void App::UpdateTitle() {
    // Press Enter to start the game
    if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        m_GameState.NewGame();
        m_CurrentState = State::LOADING;
        m_Loading = false;
        LOG_INFO("Starting game - entering LOADING state");
    }

    // ESC to quit from title
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        m_CurrentState = State::END;
    }
}

void App::UpdateLoading() {
    if (!m_Loading) {
        m_Loading = true;
        m_LoadTimer = m_Timer + Mario::GameConfig::LEVEL_TRANSITION_DELAY;

        std::string levelName = m_GameState.GetLevelName();
        LOG_INFO("Loading World {}", levelName);
        LoadLevel(levelName);
    }

    if (m_Loading && m_LoadTimer < m_Timer) {
        m_Loading = false;
        StartLevel();
        m_CurrentState = State::PLAYING;
        LOG_INFO("Level loaded - entering PLAYING state");
    }
}

void App::UpdatePlaying() {
    // ESC to open pause menu
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        m_ESCMenuSelection = 0;
        m_CurrentState = State::ESC_MENU;
        Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Pause);
        Mario::AudioManager::GetInstance().StopBGM();
        LOG_INFO("Game paused - entering ESC_MENU state");
        return;
    }

    if (!m_Player || !m_Level) return;

    // -- Controller: Handle input (MVC Controller layer) --
    m_InputHandler.HandleInput(m_Player->GetState(), m_Speed);

    // -- Pre-collision position update --
    // Apply movement physics. This is required because CollisionManager checks
    // if the theoretical new position collides, and resolves it backward.
    auto& playerState = m_Player->GetState();
    float yDelta = playerState.ApplyGravity();
    playerState.SetX(playerState.GetX() + playerState.GetVelX());
    playerState.SetY(playerState.GetY() + yDelta);

    // -- Physics & Collision --
    std::vector<Mario::Level::SpawnPoint> newSpawns;
    m_CollisionManager.CheckPlayerBlockCollision(
        *m_Player, *m_Level, m_Camera, m_GameState, *m_UIManager, &newSpawns);

    // -- Process newly spawned entities (e.g. from hitting blocks) --
    if (!newSpawns.empty()) {
        for (auto& sp : newSpawns) {
            auto entity = Mario::EntityFactory::SpawnEntity(
                m_Level->GetEntityDefByName(sp.entityName), sp.worldX,
                sp.worldY, 1, true);
            if (entity) {
                m_Entities.push_back(entity);
                m_Renderer.AddChild(entity);
            }
        }
    }

    // -- Update player state (Model tick) --
    m_Player->GetState().Tick();

    if (m_Player->GetState().IsFireShooting() &&
        m_Player->GetState().GetSpecialCounter() == 1) {
        int dir = m_Player->GetState().IsFacingRight() ? 1 : -1;
        float fbX = m_Player->GetWorldX() +
                    (dir == 1 ? m_Player->GetState().GetWidth() / 2.0f : 0);
        float fbY =
            m_Player->GetWorldY() + (m_Player->GetState().GetHeight() / 4.0f);

        Mario::EntityDef def = m_Level->GetEntityDefByName("Fire");
        // Fallback if "Fire" is not defined in the level's config (e.g.
        // IDList.csv)
        if (def.name.empty()) {
            def.id = -1;
            def.name = "Fire";
            def.type = Mario::EntityType::FIRE;
            def.isAnimated = true;
            def.animFrames = 4;
            def.doesCollide = true;
            def.isEnemy = false;
            def.isStatic = false;
        }
        auto fbEntity = Mario::EntityFactory::SpawnEntity(
            def, fbX, fbY, dir == 1 ? 1 : 0, false, m_CurrentLevelName);
        if (fbEntity) {
            // Apply initial velocity explicitly right after spawning!
            fbEntity->GetState().SetVelX(dir * 5.0f);
            m_Entities.push_back(fbEntity);
            m_Renderer.AddChild(fbEntity);
            Mario::AudioManager::GetInstance().PlaySFX(
                Mario::SFXName::FireBall);
        }
    }

    // -- Update entities --
    for (auto& entity : m_Entities) {
        if (!entity->GetState().IsActive()) continue;

        // Entity behavior update (AI/strategy pattern)
        auto behavior = entity->GetBehavior();
        if (behavior) {
            behavior->Update(entity->GetState(), *m_Level, *m_Player, m_Timer);
        }

        auto entityType = entity->GetDef().type;
        if (entityType == Mario::EntityType::FIRE) {
            // Keep fireball moving fast
            float currentSpd = std::abs(entity->GetState().GetVelX());
            if (currentSpd < 4.0f) {
                currentSpd = 5.0f;
            }
            entity->GetState().SetVelX(entity->GetState().GetDirection() == 1
                                           ? currentSpd
                                           : -currentSpd);
        }

        // Entity Model tick (movement, animation, gravity)
        entity->GetState().Tick();

        // Entity-block collision (walls, ground)
        if (entity->GetState().DoesCollide() &&
            !entity->GetState().IsStatic()) {
            CheckEntityBlockCollision(*entity);
        }

        // Entity view update (sprite, screen position)
        entity->UpdateView(m_Camera.GetOffset());
    }

    // -- Player-Entity collision --
    CheckPlayerEntityCollision();
    CheckEntityEntityCollision();

    // -- Check flagpole collision (goal block) --
    CheckFlagpoleCollision();

    // -- Check pipe warp collision --
    CheckPipeCollision();

    // -- Camera follows player --
    m_Camera.Update(m_Player->GetWorldX(), m_Level->GetWidthPixels());

    // -- Update level blocks (animations, camera-based positioning) --
    m_Level->UpdateBlocks(m_Camera.GetOffset());

    // -- Update player view (sprite selection, screen position) --
    m_Player->UpdateView(m_Camera.GetOffset());

    // -- Timer (via GameStateManager) --
    if (m_Player->GetState().IsControllable()) {
        int oldTime = m_GameState.GetTimeRemaining();
        m_GameState.Tick();
        int newTime = m_GameState.GetTimeRemaining();
        if (oldTime > 100 && newTime <= 100) {
            PlayCurrentBGM();  // switch to hurry up theme
        }
        if (m_GameState.IsTimeUp()) {
            m_GameState.LoseLife();
            m_Player->GetState().SetDead(true);
        }
    }

    // -- Check pit fall --
    if (m_CollisionManager.CheckPitFall(*m_Player)) {
        m_GameState.LoseLife();
        m_Player->GetState().SetDead(true);
        m_Player->GetState().SetControllable(false);
    }

    // -- Check death state transition --
    if (m_Player->GetState().IsDead()) {
        m_DeathTimer = m_Timer + 80;  // ~1.6s death animation
        m_CurrentState = State::DEATH;
        Mario::AudioManager::GetInstance().PlayBGM(
            Mario::BGMName::LostALifeTheme);
        LOG_INFO("Player died - entering DEATH state (Lives: {})",
                 m_GameState.GetLives());
    }

    // -- Remove dead entities from renderer --
    CleanupDeadEntities();
}

// ============================================================================
// Flagpole Sequence (Phase 5)
// C# reference: Form1.cs lines 1174-1228, 1231-1265
// ============================================================================
void App::UpdateFlagpole() {
    if (!m_Player || !m_Level) return;

    bool stillRunning =
        m_LevelCompleteCtrl.Update(*m_Player, *m_Level, m_Camera.GetOffset());

    // Update camera and blocks during cutscene
    m_Camera.Update(m_Player->GetWorldX(), m_Level->GetWidthPixels());
    m_Level->UpdateBlocks(m_Camera.GetOffset());

    // Update flag entity view if present
    if (m_FlagEntity && m_FlagEntity->GetState().IsActive()) {
        m_FlagEntity->UpdateView(m_Camera.GetOffset());
    }

    if (!stillRunning && m_LevelCompleteCtrl.IsCompleted()) {
        // Save power state for next level (C# line 1192)
        m_GameState.SavePowerState(m_Player->GetState().GetState());
        AdvanceToNextLevel();
    }
}

// ============================================================================
// Pipe Warp Sequence (Phase 5)
// C# reference: Form1.cs lines 941-968
// ============================================================================
void App::UpdatePipeWarp() {
    if (!m_Player || !m_Level) return;

    // Play warp SFX on first frame of pipe warp
    // Ensure it plays when animation begins
    static bool warpSFXPlayed = false;
    if (!warpSFXPlayed) {
        warpSFXPlayed = true;
        // TODO: Integrate with AudioManager or PTSD audio system
        // AudioManager::GetInstance().PlaySFX("20. Warp");
        LOG_DEBUG("Playing Warp SFX: Resources/Audio/SFX/20. Warp.mp3");
    }

    bool stillRunning =
        m_LevelCompleteCtrl.Update(*m_Player, *m_Level, m_Camera.GetOffset());

    if (!stillRunning && m_LevelCompleteCtrl.IsCompleted()) {
        m_GameState.SavePowerState(m_Player->GetState().GetState());
        warpSFXPlayed = false;  // Reset for next pipe warp

        if (m_GameState.HasNextLevelOverride()) {
            // Standard warp to a completely new level (e.g. 1-2 pipe to 8-4)
            AdvanceToNextLevel();
        } else if (!m_GameState.IsUnderground()) {
            // We were in the main level, now load the sub-level (underground)
            m_GameState.SetUnderground(true);
            std::string subLevel = m_Level->GetSubLevelName();
            LOG_INFO("Loading sub-level: {}", subLevel);
            LoadLevel(subLevel);
            StartLevel();
            m_CurrentState = State::PLAYING;
        } else {
            // We were underground, now return to the main level
            m_GameState.SetUnderground(false);
            std::string mainLevel = m_GameState.GetLevelName();
            LOG_INFO("Returning to main level: {}", mainLevel);
            LoadLevel(mainLevel);
            StartLevel();

            // Position Mario at the pipe exit
            if (m_Player) {
                m_Player->GetState().SetX(m_Level->GetPipeExitX());
                m_Player->GetState().SetControllable(true);
                m_Player->SetVisible(true);
            }
            m_CurrentState = State::PLAYING;
        }
    }
}

void App::UpdateDeath() {
    // Wait for death animation timer
    if (m_Timer > m_DeathTimer) {
        if (!m_GameState.IsGameOver()) {
            m_CurrentState = State::LOADING;
            m_Loading = false;
        } else {
            m_CurrentState = State::GAME_OVER;
            Mario::AudioManager::GetInstance().PlayBGM(
                Mario::BGMName::GameOverTheme);
            LOG_INFO("No lives remaining - GAME_OVER");
        }
    }
}

void App::UpdateGameOver() {
    if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        m_CurrentState = State::TITLE;
        LOG_INFO("Game Over - returning to TITLE");
    }
}

void App::UpdateESCMenu() {
    // Menu navigation
    if (Util::Input::IsKeyDown(Util::Keycode::UP)) {
        m_ESCMenuSelection = (m_ESCMenuSelection - 1 + 4) % 4;
    }
    if (Util::Input::IsKeyDown(Util::Keycode::DOWN)) {
        m_ESCMenuSelection = (m_ESCMenuSelection + 1) % 4;
    }

    // Select
    if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        switch (m_ESCMenuSelection) {
            case 0:
                m_CurrentState = State::PLAYING;
                PlayCurrentBGM();
                LOG_INFO("Resuming game");
                break;
            case 1:
                m_GameState.SetLevel(1, 1);
                m_CurrentState = State::LOADING;
                m_Loading = false;
                LOG_INFO("Jumping to World 1-1");
                break;
            case 2:
                m_GameState.SetLevel(1, 2);
                m_CurrentState = State::LOADING;
                m_Loading = false;
                LOG_INFO("Jumping to World 1-2");
                break;
            case 3:
                m_GameState.SetLevel(8, 4);
                m_CurrentState = State::LOADING;
                m_Loading = false;
                LOG_INFO("Jumping to World 8-4");
                break;
        }
    }

    // ESC to resume
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        m_CurrentState = State::PLAYING;
        PlayCurrentBGM();
        LOG_INFO("ESC pressed again - resuming game");
    }
}

// ============================================================================
// Level Management
// ============================================================================

void App::LoadLevel(const std::string& levelName) {
    m_Camera.Reset();
    m_LevelCompleteCtrl.Reset();
    m_FlagEntity = nullptr;
    m_CurrentLevelName = levelName;

    // Create and load the level
    m_Level = std::make_shared<Mario::Level>();
    if (!m_Level->Load(levelName)) {
        LOG_ERROR("Failed to load level: {}", levelName);
        m_CurrentState = State::TITLE;
        return;
    }

    // Create player at spawn position from level CSV
    float spawnX = m_Level->GetPlayerSpawnX();
    float spawnY = m_Level->GetPlayerSpawnY();

    // Restore saved power state across levels
    int savedState = m_GameState.GetSavedPowerState();
    m_Player = std::make_shared<Mario::Player>(spawnX, spawnY, savedState);

    // Spawn entities (Goomba, KoopaTroopa, etc.) from level data
    m_Entities = Mario::EntityFactory::SpawnFromLevel(*m_Level);

    // For 8-4 level, manually spawn Bowser and Princess if not in CSV
    if (levelName == "8-4") {
        // Check if Bowser is already in entities
        bool hasBowser = false, hasPrincess = false;
        for (const auto& entity : m_Entities) {
            if (entity->GetState().GetName() == "Bowser") hasBowser = true;
            if (entity->GetState().GetName() == "Princess") hasPrincess = true;
        }

        // Manually spawn Bowser at castle position (around column 320, row 8)
        if (!hasBowser) {
            float bowserX = 320.0f * Mario::GameConfig::TILE_SIZE;
            float bowserY = 9.0f * Mario::GameConfig::TILE_SIZE;

            // Try to get Bowser definition from level
            const Mario::EntityDef* bowserDef = nullptr;
            const Mario::EntityDef& defFromLevel =
                m_Level->GetEntityDefByName("Bowser");
            if (!defFromLevel.name.empty()) {
                bowserDef = &defFromLevel;
            } else {
                // Create a fallback Bowser definition if not found
                static Mario::EntityDef fallbackBowser{
                    -1,        // id
                    "Bowser",  // name
                    Mario::EntityType::BOWSER,
                    false,  // type, isPowerUp
                    true,   // isEnemy
                    false,  // isCoin
                    0,      // powerUpState
                    false,  // isStatic
                    false,  // isBounce
                    false,  // fromBlock
                    100,    // scoreWorth
                    true,   // isAnimated (4 sprite frames: Bowser1-4)
                    4,      // animFrames
                    false,  // doesJump
                    true,   // doesCollide
                    false,  // oneLoop
                    1,      // animBuffer
                    true,   // squishable
                    true    // koopaSquash
                };
                bowserDef = &fallbackBowser;
            }

            if (bowserDef && !bowserDef->name.empty()) {
                LOG_DEBUG(
                    "Attempting to spawn Bowser with entityDef: name={}, "
                    "isEnemy={}, isStatic={}",
                    bowserDef->name, bowserDef->isEnemy, bowserDef->isStatic);
                auto bowser = Mario::EntityFactory::SpawnEntity(
                    *bowserDef, bowserX, bowserY, 1, false, levelName);
                if (bowser) {
                    m_Entities.push_back(bowser);
                    LOG_WARN("??Successfully spawned Bowser at ({}, {})",
                             bowserX, bowserY);
                } else {
                    LOG_ERROR("??SpawnEntity returned nullptr for Bowser!");
                }
            } else {
                LOG_ERROR("??bowserDef is null or name is empty!");
            }
        }

        // Manually spawn Princess at castle position (around column 320, row
        // 11)
        if (!hasPrincess) {
            float princessX = 325.0f * Mario::GameConfig::TILE_SIZE;
            float princessY = 10.0f * Mario::GameConfig::TILE_SIZE;

            // Try to get Princess definition from level
            const Mario::EntityDef* princessDef = nullptr;
            const Mario::EntityDef& defFromLevel2 =
                m_Level->GetEntityDefByName("Princess");
            if (!defFromLevel2.name.empty()) {
                princessDef = &defFromLevel2;
            } else {
                // Create a fallback Princess definition if not found
                static Mario::EntityDef fallbackPrincess{
                    -1,                           // id
                    "Princess",                   // name
                    Mario::EntityType::PRINCESS,  // type
                    false,                        // isPowerUp
                    false,                        // isEnemy
                    false,                        // isCoin
                    0,                            // powerUpState
                    true,                         // isStatic
                    false,                        // isBounce
                    false,                        // fromBlock
                    0,                            // scoreWorth
                    true,   // isAnimated (4 sprite frames: Princess1-4)
                    4,      // animFrames
                    false,  // doesJump
                    true,   // doesCollide
                    false,  // oneLoop
                    1,      // animBuffer
                    false,  // squishable
                    false   // koopaSquash
                };
                princessDef = &fallbackPrincess;
            }

            if (princessDef && !princessDef->name.empty()) {
                LOG_DEBUG(
                    "Attempting to spawn Princess with entityDef: name={}, "
                    "isEnemy={}, isStatic={}",
                    princessDef->name, princessDef->isEnemy,
                    princessDef->isStatic);
                auto princess = Mario::EntityFactory::SpawnEntity(
                    *princessDef, princessX, princessY, 1, false, levelName);
                if (princess) {
                    m_Entities.push_back(princess);
                    LOG_WARN("??Successfully spawned Princess at ({}, {})",
                             princessX, princessY);
                } else {
                    LOG_ERROR("??SpawnEntity returned nullptr for Princess!");
                }
            } else {
                LOG_ERROR("??princessDef is null or name is empty!");
            }
        }
    }

    // Look for the Flag entity in spawn list
    for (auto& entity : m_Entities) {
        if (entity->GetState().GetName() == "Flag") {
            m_FlagEntity = entity;
            break;
        }
    }

    // Build renderer: clear and add all blocks + player + entities
    m_Renderer = Util::Renderer();
    for (const auto& block : m_Level->GetAllBlocks()) {
        m_Renderer.AddChild(block);
    }
    m_Renderer.AddChild(m_Player);
    for (const auto& entity : m_Entities) {
        m_Renderer.AddChild(entity);
    }

    // Set map background color (Sky Blue or Black for underground)
    // Alpha = 0.0f for transparent background (chroma key support)
    // Determine background color: underground/castle = black, surface = sky
    // blue
    bool isUnderground = m_GameState.IsUnderground() ||
                         levelName.find("u") != std::string::npos ||
                         levelName == "1-2" || levelName == "8-4";
    if (isUnderground) {
        glClearColor(0.0f, 0.0f, 0.0f,
                     0.0f);  // Black (castle/dungeon background)
    } else {
        glClearColor(92.0f / 255.0f, 148.0f / 255.0f, 252.0f / 255.0f,
                     0.0f);  // Sky Blue (transparent)
    }

    LOG_INFO("Level {} loaded: {} blocks, {} entities, player at ({}, {})",
             levelName, m_Level->GetAllBlocks().size(), m_Entities.size(),
             spawnX, spawnY);
}

void App::PlayCurrentBGM() {
    int time = m_GameState.GetTimeRemaining();
    bool hurry = time <= 100 && time > 0;
    std::string lvl = m_CurrentLevelName;

    Mario::BGMName bgm = Mario::BGMName::GroundTheme;
    if (lvl == "8-4" || lvl == "8-4_sub") {
        bgm = hurry ? Mario::BGMName::CastleThemeHurryUp
                    : Mario::BGMName::CastleTheme;
    } else if (lvl == "1-1u" || lvl == "1-2" || lvl == "1-2uu" ||
               lvl == "1-2_sub") {
        bgm = hurry ? Mario::BGMName::UndergroundThemeHurryUp
                    : Mario::BGMName::UndergroundTheme;
    } else {
        bgm = hurry ? Mario::BGMName::GroundThemeHurryUp
                    : Mario::BGMName::GroundTheme;
    }
    Mario::AudioManager::GetInstance().PlayBGM(bgm);
}

void App::StartLevel() {
    m_GameState.ResetTime();
    m_GameState.StartTime();

    if (m_Player) {
        m_Player->GetState().SetControllable(true);
    }
    PlayCurrentBGM();
}

void App::RenderAll() {
    // Rendering based on current game state
    // UIManager now only handles UI state (floating texts)

    if (!m_UIManager) {
        m_Renderer.Update();
        return;
    }

    switch (m_CurrentState) {
        case State::TITLE:
            glClearColor(92.0f / 255.0f, 148.0f / 255.0f, 252.0f / 255.0f,
                         0.0f);
            m_Renderer.Update();
            m_UIManager->Update(Mario::UIManager::State::TITLE);
            break;

        case State::LOADING: {
            // Set background color based on level being loaded
            std::string currentLevel = m_GameState.GetLevelName();
            bool isUnderground = m_GameState.IsUnderground() ||
                                 currentLevel.find("u") != std::string::npos ||
                                 currentLevel == "1-2" || currentLevel == "8-4";
            if (isUnderground) {
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  // Black
            } else {
                glClearColor(92.0f / 255.0f, 148.0f / 255.0f, 252.0f / 255.0f,
                             0.0f);  // Sky blue
            }
            m_Renderer.Update();
            m_UIManager->Update(Mario::UIManager::State::LOADING);
            break;
        }

        case State::PLAYING:
        case State::FLAGPOLE:
        case State::PIPE_WARP: {
            // Determine if current level should have underground/castle
            // background
            std::string currentLevel = m_GameState.GetLevelName();
            bool isUnderground = m_GameState.IsUnderground() ||
                                 currentLevel.find("u") != std::string::npos ||
                                 currentLevel == "1-2" || currentLevel == "8-4";
            if (isUnderground) {
                glClearColor(0.0f, 0.0f, 0.0f,
                             0.0f);  // Black for castle/dungeon
            } else {
                glClearColor(92.0f / 255.0f, 148.0f / 255.0f, 252.0f / 255.0f,
                             0.0f);  // Sky blue for surface
            }
            m_Renderer.Update();

            // Update UI elements
            m_UIManager->Update(Mario::UIManager::State::PLAYING);
            break;
        }

        case State::DEATH:
            m_Renderer.Update();
            m_UIManager->Update(Mario::UIManager::State::PLAYING);
            break;

        case State::GAME_OVER:
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            m_Renderer.Update();
            m_UIManager->Update(Mario::UIManager::State::GAME_OVER);
            break;

        case State::ESC_MENU:
            m_Renderer.Update();
            m_UIManager->Update(Mario::UIManager::State::ESC_MENU,
                                m_ESCMenuSelection);
            break;

        case State::START:
        case State::END:
        default:
            m_Renderer.Update();
            break;
    }
}

void App::AdvanceToNextLevel() {
    std::string nextLevel = m_GameState.AdvanceLevel();
    if (m_GameState.IsGameWon()) {
        // All levels beaten
        m_CurrentState = State::TITLE;
        LOG_INFO("Game Complete! Returning to title.");
        return;
    }
    m_CurrentState = State::LOADING;
    m_Loading = false;
    LOG_INFO("Advancing to level {}", nextLevel);
}

// ============================================================================
// End
// ============================================================================
void App::End() { LOG_TRACE("End"); }

// ============================================================================
// Flagpole & Pipe Detection
// ============================================================================

void App::CheckFlagpoleCollision() {
    if (!m_Player || m_Player->GetState().IsDead()) return;
    if (m_LevelCompleteCtrl.IsActive()) return;

    Mario::PlayerState& ps = m_Player->GetState();
    Mario::AABB playerBox = ps.GetHitbox();

    for (const auto& block : m_Level->GetAllBlocks()) {
        if (!block->IsGoal()) continue;

        Mario::AABB blockBox = block->GetAABB();
        if (!playerBox.Intersects(blockBox)) continue;

        // Player touched the flagpole goal!
        LOG_INFO("Flagpole reached at block ({}, {})", block->GetGridX(),
                 block->GetGridY());

        Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Flagpole);
        Mario::AudioManager::GetInstance().PlayBGM(
            m_CurrentLevelName == "8-4" ? Mario::BGMName::CastleCompleteTheme
                                        : Mario::BGMName::LevelCompleteTheme);

        m_LevelCompleteCtrl.StartFlagpole(*m_Player, m_FlagEntity, block.get());
        m_GameState.StopTime();
        m_CurrentState = State::FLAGPOLE;
        return;
    }
}

void App::CheckPipeCollision() {
    if (!m_Player || m_Player->GetState().IsDead()) return;
    if (m_LevelCompleteCtrl.IsActive()) return;

    Mario::PlayerState& ps = m_Player->GetState();
    Mario::AABB playerBox = ps.GetHitbox();

    bool pipeDown1 = false, pipeDown2 = false;
    bool pipeRight1 = false, pipeRight2 = false;
    float pipeDX = 0.0f, pipeDY = 0.0f;
    float pipeRX = 0.0f, pipeRY = 0.0f;

    for (const auto& block : m_Level->GetAllBlocks()) {
        Mario::AABB bBox = block->GetAABB();
        if (!playerBox.Intersects(bBox)) continue;

        int id = block->GetBlockID();

        // Down pipe check (C# lines 802-813)
        if (id == Mario::GameConfig::PIPE_DOWN_LEFT) {
            pipeDown1 = true;
            pipeDX = block->GetWorldX();
            pipeDY = block->GetWorldY();
        }
        if (id == Mario::GameConfig::PIPE_DOWN_RIGHT) {
            pipeDown2 = true;
        }

        // Right pipe check (C# lines 815-827)
        if (id == Mario::GameConfig::PIPE_RIGHT_TOP) {
            pipeRight1 = true;
            pipeRX = block->GetWorldX();
            pipeRY = block->GetWorldY();
        }
        if (id == Mario::GameConfig::PIPE_RIGHT_BOT) {
            pipeRight2 = true;
            if (!pipeRight1) {
                pipeRX = block->GetWorldX();
                pipeRY = block->GetWorldY();
            }
        }
    }

    // Down pipe: need both halves + player centered + pressing Down
    // C# line 830: pipeCheck1 && pipeCheck2 && position centered && direction
    // == "Down"
    if (pipeDown1 && pipeDown2 &&
        Util::Input::IsKeyPressed(Util::Keycode::DOWN)) {
        // Check Mario is centered on the pipe
        float pipeCenter = pipeDX + Mario::GameConfig::TILE_SIZE;
        float playerCenter = ps.GetX() + ps.GetWidth() / 2.0f;
        if (std::abs(playerCenter - pipeCenter) <
            Mario::GameConfig::TILE_SIZE * 0.6f) {
            LOG_INFO("Entering pipe DOWN at ({}, {})", pipeDX, pipeDY);
            m_LevelCompleteCtrl.StartPipeWarp(*m_Player, "Down", pipeDX,
                                              pipeDY);
            Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Warp);
            int time = m_GameState.GetTimeRemaining();
            Mario::AudioManager::GetInstance().PlayBGM(
                (time <= 100 && time > 0) ? Mario::BGMName::IntoThePipeHurryUp
                                          : Mario::BGMName::IntoThePipeTheme);
            m_GameState.StopTime();
            m_CurrentState = State::PIPE_WARP;
            return;
        }
    }

    // Right pipe: need any pipe half + player grounded + pressing Right
    // C# lines 834-837
    if ((pipeRight1 || pipeRight2) && ps.IsGrounded() &&
        Util::Input::IsKeyPressed(Util::Keycode::RIGHT)) {
        // Ensure Mario isn't above the pipe trying to jump over it
        float playerBot = ps.GetY() - ps.GetHeight();
        if (playerBot < pipeRY + Mario::GameConfig::TILE_SIZE * 0.1f) {
            LOG_INFO("Entering pipe RIGHT at ({}, {})", pipeRX, pipeRY);
            m_LevelCompleteCtrl.StartPipeWarp(*m_Player, "Right", pipeRX,
                                              pipeRY);
            Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Warp);
            int time = m_GameState.GetTimeRemaining();
            Mario::AudioManager::GetInstance().PlayBGM(
                (time <= 100 && time > 0) ? Mario::BGMName::IntoThePipeHurryUp
                                          : Mario::BGMName::IntoThePipeTheme);
            m_GameState.StopTime();
            m_CurrentState = State::PIPE_WARP;

            // Warp exactly to 8-4 from 1-2 based on C# code spec "from pipe
            // teleport to 8-4"
            if (m_GameState.GetLevelName() == "1-2") {
                m_GameState.SetNextLevelOverride("8-4");
            }

            return;
        }
    }

    // Auto-trigger pipe warp for ID 42/43 (right pipe in 1-2)
    // Simply touching these pipe blocks triggers the warp animation
    // regardless of key input (automatic pipe entry for seamless progression)
    if (m_GameState.GetLevelName() == "1-2") {
        // Expand Mario's hitbox slightly since the collision manager will
        // prevent actual intersection with the solid pipe block
        Mario::AABB expandedBox = playerBox;
        expandedBox.left -= 2.0f;
        expandedBox.right += 2.0f;

        for (const auto& block : m_Level->GetAllBlocks()) {
            int id = block->GetBlockID();
            if (id == Mario::GameConfig::PIPE_RIGHT_TOP ||
                id == Mario::GameConfig::PIPE_RIGHT_BOT) {
                Mario::AABB bBox = block->GetAABB();
                if (expandedBox.Intersects(bBox) && ps.IsGrounded()) {
                    LOG_INFO("Auto-warp triggered by pipe contact at 1-2");
                    m_LevelCompleteCtrl.StartPipeWarp(*m_Player, "Right",
                                                      block->GetWorldX(),
                                                      block->GetWorldY());
                    Mario::AudioManager::GetInstance().PlaySFX(
                        Mario::SFXName::Warp);
                    int time = m_GameState.GetTimeRemaining();
                    Mario::AudioManager::GetInstance().PlayBGM(
                        (time <= 100 && time > 0)
                            ? Mario::BGMName::IntoThePipeHurryUp
                            : Mario::BGMName::IntoThePipeTheme);
                    LOG_DEBUG("Warp SFX should play now: 20. Warp.mp3");
                    m_GameState.StopTime();
                    m_CurrentState = State::PIPE_WARP;
                    m_GameState.SetNextLevelOverride("8-4");
                    return;
                }
            }
        }
    }
}

// ============================================================================
// Entity Helpers
// ============================================================================

void App::CheckEntityBlockCollision(Mario::Entity& entity) {
    Mario::EntityState& state = entity.GetState();
    Mario::AABB box = state.GetHitbox();

    int tileSize = Mario::GameConfig::TILE_SIZE;

    // Ground check
    int leftTile = static_cast<int>(box.left) / tileSize;
    int rightTile = static_cast<int>(box.right - 1) / tileSize;
    int bottomTile = static_cast<int>(box.bottom) / tileSize;

    bool onGround = false;
    for (int x = leftTile; x <= rightTile; x++) {
        Mario::Block* block = m_Level->GetBlockAt(x, bottomTile);
        if (block && block->IsSolid()) {
            Mario::AABB bb = block->GetAABB();
            if (box.Intersects(bb)) {
                float overlap = box.bottom - bb.top;
                if (overlap > 0 && overlap < tileSize * 0.75f) {
                    state.SetY(bb.top - state.GetHeight());
                    state.SetVelY(0.0f);
                    state.SetGrounded(true);
                    onGround = true;
                }
            }
        }
    }
    if (!onGround && state.IsGrounded()) {
        state.SetGrounded(false);
    }

    // Wall check: flip direction on wall collision
    Mario::AABB updatedBox = state.GetHitbox();
    if (state.GetVelX() > 0) {
        int rtile = static_cast<int>(updatedBox.right) / tileSize;
        for (int y = static_cast<int>(updatedBox.top) / tileSize;
             y <= static_cast<int>(updatedBox.bottom - 1) / tileSize; y++) {
            Mario::Block* block = m_Level->GetBlockAt(rtile, y);
            if (block && block->IsSolid()) {
                state.FlipDirection();
                state.SetX(block->GetWorldX() - state.GetWidth());
                break;
            }
        }
    } else if (state.GetVelX() < 0) {
        int ltile = static_cast<int>(updatedBox.left) / tileSize;
        for (int y = static_cast<int>(updatedBox.top) / tileSize;
             y <= static_cast<int>(updatedBox.bottom - 1) / tileSize; y++) {
            Mario::Block* block = m_Level->GetBlockAt(ltile, y);
            if (block && block->IsSolid()) {
                state.FlipDirection();
                state.SetX(block->GetWorldX() + tileSize);
                break;
            }
        }
    }

    // Pit fall: deactivate entity if below level
    if (state.GetY() > Mario::GameConfig::LEVEL_HEIGHT_PX + tileSize) {
        state.Delete();
    }
}

void App::CheckPlayerEntityCollision() {
    if (!m_Player || m_Player->GetState().IsDead()) return;

    Mario::PlayerState& ps = m_Player->GetState();
    Mario::AABB playerBox = ps.GetHitbox();

    for (auto& entity : m_Entities) {
        Mario::EntityState& es = entity->GetState();
        if (!es.IsActive()) continue;

        Mario::AABB entityBox = es.GetHitbox();
        if (!playerBox.Intersects(entityBox)) continue;

        if (es.IsEnemy()) {
            // Check if player is stomping (falling from above)
            float playerBottom = playerBox.bottom;
            float entityTop = entityBox.top;
            float overlapY = playerBottom - entityTop;

            if (overlapY > 0 &&
                overlapY < Mario::GameConfig::TILE_SIZE * 0.5f &&
                ps.GetVelY() >= 0 && !ps.IsGrounded()) {
                // Stomp! Kill enemy
                if (es.IsSquishable() || es.IsKoopaSquash()) {
                    es.Squish();
                    if (es.GetName() == "Bowser") {
                        Mario::AudioManager::GetInstance().PlaySFX(
                            Mario::SFXName::BowserDie);
                    } else {
                        Mario::AudioManager::GetInstance().PlaySFX(
                            Mario::SFXName::Squish);
                    }
                    int scoreWorth = es.GetScoreWorth();
                    m_GameState.AddScore(scoreWorth);

                    // Display floating score text at enemy position
                    float enemyWorldX =
                        es.GetWorldX() + Mario::GameConfig::TILE_SIZE * 0.5f;
                    float enemyWorldY = es.GetWorldY();
                    float screenPixelX = m_Camera.WorldToScreenX(enemyWorldX);
                    float screenPixelY = m_Camera.WorldToScreenY(enemyWorldY);
                    float ptsdX = screenPixelX - 640.0f;
                    float ptsdY = 360.0f - screenPixelY;
                    m_UIManager->AddFloatingText(
                        ptsdX, ptsdY, "+" + std::to_string(scoreWorth), 60);

                    // Bounce player up after stomp
                    ps.SetFallHeight(Mario::PhysicsEngine::GetJumpHeight(0) *
                                     0.5);
                    ps.SetGrounded(false);
                    LOG_DEBUG("Stomped {} (+{} score)", es.GetName(),
                              scoreWorth);
                }
            } else if (!es.IsSquished()) {
                // Player takes damage from enemy
                if (ps.IsStar()) {
                    es.Squish();
                    m_GameState.AddScore(es.GetScoreWorth());
                } else {
                    if (ps.IsStar()) {
                        es.Squish();
                        m_GameState.AddScore(es.GetScoreWorth());
                    } else {
                        ps.TakeDamage();
                    }
                }
                LOG_DEBUG("Player hit by {}", es.GetName());
            }
        } else if (es.IsPowerUp()) {
            // Collect power-up
            int puState = es.GetPowerUpState();
            if (puState == 1) {
                // Mushroom -> Big
                if (ps.GetState() == 0) {
                    // When growing from small to big, adjust Y position upward
                    // (small=32px height, big=64px height, so move up by 32px)
                    ps.SetY(ps.GetY() - Mario::GameConfig::TILE_SIZE);
                }
                // Always apply the power-up
                ps.PowerUp(Mario::PowerState::BIG);
                Mario::AudioManager::GetInstance().PlaySFX(
                    Mario::SFXName::Powerup);
            } else if (puState == 2) {
                // Fire Flower
                if (ps.GetState() == 0) {
                    // Same adjustment for small->fire
                    ps.SetY(ps.GetY() - Mario::GameConfig::TILE_SIZE);
                }
                // Big Mario eats fire flower -> fires shots
                // Small Mario eats fire flower -> grows to Big and fires shots
                // But we just set state to FIRE regardless
                ps.PowerUp(Mario::PowerState::FIRE);
                Mario::AudioManager::GetInstance().PlaySFX(
                    Mario::SFXName::Powerup);
            } else if (puState == 3) {
                // Star
                ps.StartStar();
                Mario::AudioManager::GetInstance().PlaySFX(
                    Mario::SFXName::Powerup);
            } else if (puState == 5) {
                // 1-Up
                m_GameState.AddLife();
                Mario::AudioManager::GetInstance().PlaySFX(
                    Mario::SFXName::_1up);

                // Display "+1UP" floating text
                float oneupWorldX =
                    es.GetWorldX() + Mario::GameConfig::TILE_SIZE * 0.5f;
                float oneupWorldY = es.GetWorldY();
                float oneupScreenPixelX = m_Camera.WorldToScreenX(oneupWorldX);
                float oneupScreenPixelY = m_Camera.WorldToScreenY(oneupWorldY);
                float oneupPtsdX = oneupScreenPixelX - 640.0f;
                float oneupPtsdY = 360.0f - oneupScreenPixelY;
                m_UIManager->AddFloatingText(oneupPtsdX, oneupPtsdY, "+1UP",
                                             60);
            }
            int scoreWorth = es.GetScoreWorth();
            m_GameState.AddScore(scoreWorth);

            // Display floating score text at powerup position
            float puWorldX =
                es.GetWorldX() + Mario::GameConfig::TILE_SIZE * 0.5f;
            float puWorldY = es.GetWorldY();
            float puScreenPixelX = m_Camera.WorldToScreenX(puWorldX);
            float puScreenPixelY = m_Camera.WorldToScreenY(puWorldY);
            float puPtsdX = puScreenPixelX - 640.0f;
            float puPtsdY = 360.0f - puScreenPixelY;
            m_UIManager->AddFloatingText(puPtsdX, puPtsdY,
                                         "+" + std::to_string(scoreWorth), 60);

            es.Delete();
            LOG_DEBUG("Collected {} (+{} score)", es.GetName(), scoreWorth);
        } else if (es.IsCoin()) {
            m_GameState.AddCoin();
            int coinScore = es.GetScoreWorth();
            m_GameState.AddScore(coinScore);
            Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Coin);

            // Display floating score text at coin position
            float coinWorldX =
                es.GetWorldX() + Mario::GameConfig::TILE_SIZE * 0.5f;
            float coinWorldY = es.GetWorldY();
            float coinScreenPixelX = m_Camera.WorldToScreenX(coinWorldX);
            float coinScreenPixelY = m_Camera.WorldToScreenY(coinWorldY);
            float coinPtsdX = coinScreenPixelX - 640.0f;
            float coinPtsdY = 360.0f - coinScreenPixelY;
            m_UIManager->AddFloatingText(coinPtsdX, coinPtsdY,
                                         "+" + std::to_string(coinScore), 60);

            es.Delete();
        }
    }
}

void App::CleanupDeadEntities() {
    for (auto it = m_Entities.begin(); it != m_Entities.end();) {
        if (!(*it)->GetState().IsActive()) {
            m_Renderer.RemoveChild(*it);
            it = m_Entities.erase(it);
        } else {
            ++it;
        }
    }
}
void App::CheckEntityEntityCollision() {
    for (auto& proj : m_Entities) {
        if (!proj->GetState().IsActive()) continue;
        if (proj->GetState().GetName() == "Fire" ||
            (proj->GetState().GetName() == "KoopaTroopaShell" &&
             proj->GetState().GetVelX() != 0)) {
            Mario::AABB pBox = proj->GetState().GetHitbox();
            for (auto& enemy : m_Entities) {
                if (proj == enemy || !enemy->GetState().IsActive() ||
                    !enemy->GetState().IsEnemy() || enemy->GetState().IsDead())
                    continue;
                if (pBox.Intersects(enemy->GetState().GetHitbox())) {
                    enemy->GetState().Squish();
                    Mario::AudioManager::GetInstance().PlaySFX(
                        Mario::SFXName::Kick);
                    int worth = enemy->GetState().GetScoreWorth();
                    if (worth > 0) m_GameState.AddScore(worth);
                    if (proj->GetState().GetName() == "Fire") {
                        proj->GetState().Delete();
                    }
                }
            }
        }
    }
}
