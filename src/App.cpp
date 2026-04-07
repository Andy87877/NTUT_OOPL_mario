/**
 * @file App.cpp
 * @brief Implementation of the main application controller.
 *        Handles game state machine, integrates Level loading, Player MVC,
 *        collision detection, and camera following.
 *        Game loop logic ported from C# Form1.cs onTick().
 * @inheritance None (top-level controller)
 */
#include "App.hpp"

#include "Mario/PhysicsEngine.hpp"

#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

// ============================================================================
// Start: Initialize game, transition to TITLE
// ============================================================================
void App::Start() {
    LOG_TRACE("Start");
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
        m_WorldNum = 1;
        m_LevelNum = 1;
        m_Lives = Mario::GameConfig::INITIAL_LIVES;
        m_Score = 0;
        m_Coins = 0;
        m_TimeCounter = Mario::GameConfig::INITIAL_TIME;
        m_TimeSubCounter = 0;
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
        m_LoadTimer = m_Timer + 50;

        std::string levelName = std::to_string(m_WorldNum) + "-" +
                                std::to_string(m_LevelNum);
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
        LOG_INFO("Game paused - entering ESC_MENU state");
        return;
    }

    if (!m_Player || !m_Level) return;

    // -- Controller: Handle input (MVC Controller layer) --
    m_InputHandler.HandleInput(m_Player->GetState(), m_Speed);

    // -- Physics & Collision --
    m_CollisionManager.CheckPlayerBlockCollision(*m_Player, *m_Level,
                                                  m_Camera.GetOffset());

    // -- Update player state (Model tick) --
    m_Player->GetState().Tick();

    // -- Update entities --
    for (auto& entity : m_Entities) {
        if (!entity->GetState().IsActive()) continue;

        // Entity Model tick (movement, animation, gravity)
        entity->GetState().Tick();

        // Entity-block collision (walls, ground)
        if (entity->GetState().DoesCollide() && !entity->GetState().IsStatic()) {
            CheckEntityBlockCollision(*entity);
        }

        // Entity view update (sprite, screen position)
        entity->UpdateView(m_Camera.GetOffset());
    }

    // -- Player-Entity collision --
    CheckPlayerEntityCollision();

    // -- Camera follows player --
    m_Camera.Update(m_Player->GetWorldX(), m_Level->GetWidthPixels());

    // -- Update level blocks (animations, camera-based positioning) --
    m_Level->UpdateBlocks(m_Camera.GetOffset());

    // -- Update player view (sprite selection, screen position) --
    m_Player->UpdateView(m_Camera.GetOffset());

    // -- Timer (matching C# reference: 40 ticks = 1 game-second) --
    if (m_Player->GetState().IsControllable()) {
        m_TimeSubCounter++;
        if (m_TimeSubCounter >= Mario::GameConfig::TIME_SUB_LIMIT) {
            m_TimeCounter--;
            m_TimeSubCounter = 0;
        }
        if (m_TimeCounter <= 0) {
            m_Lives--;
            m_Player->GetState().SetDead(true);
        }
    }

    // -- Check pit fall --
    if (m_CollisionManager.CheckPitFall(*m_Player)) {
        m_Lives--;
        m_Player->GetState().SetDead(true);
        m_Player->GetState().SetControllable(false);
    }

    // -- Check death state transition --
    if (m_Player->GetState().IsDead()) {
        m_DeathTimer = m_Timer + 80; // ~1.6s death animation
        m_CurrentState = State::DEATH;
        LOG_INFO("Player died - entering DEATH state (Lives: {})", m_Lives);
    }

    // -- Remove dead entities from renderer --
    CleanupDeadEntities();
}

void App::UpdateDeath() {
    // Wait for death animation timer
    if (m_Timer > m_DeathTimer) {
        if (m_Lives > 0) {
            m_CurrentState = State::LOADING;
            m_Loading = false;
        } else {
            m_CurrentState = State::GAME_OVER;
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
                LOG_INFO("Resuming game");
                break;
            case 1:
                m_WorldNum = 1; m_LevelNum = 1;
                m_CurrentState = State::LOADING;
                m_Loading = false;
                LOG_INFO("Jumping to World 1-1");
                break;
            case 2:
                m_WorldNum = 1; m_LevelNum = 2;
                m_CurrentState = State::LOADING;
                m_Loading = false;
                LOG_INFO("Jumping to World 1-2");
                break;
            case 3:
                m_WorldNum = 8; m_LevelNum = 4;
                m_CurrentState = State::LOADING;
                m_Loading = false;
                LOG_INFO("Jumping to World 8-4");
                break;
        }
    }

    // ESC to resume
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        m_CurrentState = State::PLAYING;
        LOG_INFO("ESC pressed again - resuming game");
    }
}

// ============================================================================
// Level Management
// ============================================================================

void App::LoadLevel(const std::string& levelName) {
    m_Camera.Reset();

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
    m_Player = std::make_shared<Mario::Player>(spawnX, spawnY, 0);

    // Spawn entities (Goomba, KoopaTroopa, etc.) from level data
    m_Entities = Mario::EntityFactory::SpawnFromLevel(*m_Level);

    // Build renderer: clear and add all blocks + player + entities
    m_Renderer = Util::Renderer();
    for (const auto& block : m_Level->GetAllBlocks()) {
        m_Renderer.AddChild(block);
    }
    m_Renderer.AddChild(m_Player);
    for (const auto& entity : m_Entities) {
        m_Renderer.AddChild(entity);
    }

    LOG_INFO("Level {} loaded: {} blocks, {} entities, player at ({}, {})",
             levelName, m_Level->GetAllBlocks().size(), m_Entities.size(),
             spawnX, spawnY);
}

void App::StartLevel() {
    m_TimeCounter = Mario::GameConfig::INITIAL_TIME;
    m_TimeSubCounter = 0;

    if (m_Player) {
        m_Player->GetState().SetControllable(true);
    }
}

void App::RenderAll() {
    m_Renderer.Update();
}

void App::AdvanceToNextLevel() {
    if (m_WorldNum == 1 && m_LevelNum == 1) {
        m_LevelNum = 2;
    } else if (m_WorldNum == 1 && m_LevelNum == 2) {
        m_WorldNum = 8;
        m_LevelNum = 4;
    } else {
        m_CurrentState = State::TITLE;
        LOG_INFO("Game Complete! Returning to title.");
        return;
    }
    m_CurrentState = State::LOADING;
    m_Loading = false;
}

// ============================================================================
// End
// ============================================================================
void App::End() {
    LOG_TRACE("End");
}

// ============================================================================
// Entity Helpers
// ============================================================================

void App::CheckEntityBlockCollision(Mario::Entity& entity) {
    Mario::EntityState& state = entity.GetState();
    Mario::AABB box = state.GetHitbox();

    int tileSize = Mario::GameConfig::TILE_SIZE;

    // Ground check
    int leftTile  = static_cast<int>(box.left) / tileSize;
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

            if (overlapY > 0 && overlapY < Mario::GameConfig::TILE_SIZE * 0.5f
                && ps.GetVelY() >= 0 && !ps.IsGrounded()) {
                // Stomp! Kill enemy
                if (es.IsSquishable() || es.IsKoopaSquash()) {
                    es.Squish();
                    m_Score += es.GetScoreWorth();
                    // Bounce player up after stomp
                    ps.SetFallHeight(Mario::PhysicsEngine::GetJumpHeight(0) * 0.5);
                    ps.SetGrounded(false);
                    LOG_DEBUG("Stomped {} (+{} score)",
                              es.GetName(), es.GetScoreWorth());
                }
            } else if (!es.IsSquished()) {
                // Player takes damage from enemy
                ps.TakeDamage();
                LOG_DEBUG("Player hit by {}", es.GetName());
            }
        } else if (es.IsPowerUp()) {
            // Collect power-up
            int puState = es.GetPowerUpState();
            if (puState == 1) {
                // Mushroom -> Big
                if (ps.GetState() == 0) {
                    ps.PowerUp(Mario::PowerState::BIG);
                }
            } else if (puState == 2) {
                // Fire Flower
                ps.PowerUp(Mario::PowerState::FIRE);
            } else if (puState == 3) {
                // Star
                ps.StartStar();
            } else if (puState == 5) {
                // 1-Up
                m_Lives++;
            }
            m_Score += es.GetScoreWorth();
            es.Delete();
            LOG_DEBUG("Collected {} (+{} score)", es.GetName(), es.GetScoreWorth());
        } else if (es.IsCoin()) {
            m_Coins++;
            m_Score += es.GetScoreWorth();
            es.Delete();
            if (m_Coins >= 100) {
                m_Coins -= 100;
                m_Lives++;
            }
        }
    }
}

void App::CleanupDeadEntities() {
    for (auto it = m_Entities.begin(); it != m_Entities.end(); ) {
        if (!(*it)->GetState().IsActive()) {
            m_Renderer.RemoveChild(*it);
            it = m_Entities.erase(it);
        } else {
            ++it;
        }
    }
}
