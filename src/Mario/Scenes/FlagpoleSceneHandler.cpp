/**
 * @file FlagpoleSceneHandler.cpp
 * @brief Flagpole ending sequence handler (App::State::FLAGPOLE).
 *        Handles flagpole sliding, waiting, walking to castle, and loading next level.
 * @inheritance ISceneHandler <- FlagpoleSceneHandler
 */
#include "Mario/Scenes/FlagpoleSceneHandler.hpp"

#include "App.hpp"
#include "Mario/Core/GameConfig.hpp"
#include "Mario/Services/AudioManager.hpp"
#include "Util/Logger.hpp"

namespace Mario {

void FlagpoleSceneHandler::OnEnter(App& app) {
    m_Phase = Phase::POLE_SLIDE;
    m_tick_count = 0;
    m_level_timer = -1;

    // Dynamically discover flagpole entity and goal coordinates
    m_flag_entity = app.GetFlagEntity();
    auto& level = app.GetLevel();
    if (level && !level->GetGoalBlocks().empty()) {
        const Block* goalBlock = level->GetGoalBlocks()[0];
        if (goalBlock) {
            m_goal_block_x = goalBlock->GetWorldX();
            m_goal_block_y = goalBlock->GetWorldY();
        }
    }

    auto& player = app.GetPlayer();
    if (player) {
        player->GetState().SetGrounded(false);
        player->GetState().SetVelY(0.0f);
    }
}

void FlagpoleSceneHandler::Update(App& app) {
    auto& player = app.GetPlayer();
    auto& level = app.GetLevel();
    if (!player || !level) return;

    m_tick_count++;

    auto& ps = player->GetState();

    // If walking to the castle, apply gravity so that collision snapping keeps him on the ground
    if (m_Phase == Phase::POLE_WALK) {
        float yDelta = ps.ApplyGravity();
        ps.SetY(ps.GetY() + yDelta);
    }

    switch (m_Phase) {
        case Phase::POLE_SLIDE:
            UpdatePoleSlide(*player);
            break;
        case Phase::POLE_WALK:
            UpdatePoleWalk(*player, *level);
            break;
        case Phase::ENTER_CASTLE:
            UpdateEnterCastle(*player);
            break;
        case Phase::WAIT_TRANSITION:
            if (m_tick_count > m_level_timer) {
                m_Phase = Phase::COMPLETED;
            }
            break;
        default:
            break;
    }

    // Run player-block collision resolution dynamically!
    if (m_Phase == Phase::POLE_SLIDE || m_Phase == Phase::POLE_WALK) {
        std::vector<Mario::Level::SpawnPoint> unusedSpawns;
        app.GetCollisionManager().CheckPlayerBlockCollision(
            *player, *level, app.GetCamera(), app.GetGameState(),
            app.GetUIManager(), &unusedSpawns);
        
        // Tick player animation/invincibility/star timers
        ps.Tick();
    }

    player->UpdateView(app.GetCamera().GetOffset());

    app.GetCamera().Update(player->GetWorldX(), level->GetWidthPixels(),
                           app.GetCurrentLevelName(), true);
    level->UpdateBlocks(app.GetCamera().GetOffset());

    auto& flag = m_flag_entity;
    if (flag && flag->GetState().IsActive()) {
        flag->UpdateView(app.GetCamera().GetOffset());
    }

    if (m_Phase == Phase::COMPLETED) {
        app.GetGameState().SavePowerState(player->GetState().GetState());
        app.AdvanceToNextLevel();
    }
}

void FlagpoleSceneHandler::OnRender(App& app) {
    app.ApplyBackground();
    app.GetRenderer().Update();
    app.GetUIManager().Update(Mario::UIManager::State::PLAYING);
}

void FlagpoleSceneHandler::UpdatePoleSlide(Player& player) {
    PlayerState& ps = player.GetState();

    // Slide Mario down the pole if not grounded
    if (!ps.IsGrounded()) {
        float marioY = ps.GetY() + GameConfig::FLAGPOLE_SLIDE_SPEED;
        ps.SetY(marioY);
    }

    // Slide flag entity down with Mario/until bottom
    if (m_flag_entity && m_flag_entity->GetState().IsActive()) {
        EntityState& fs = m_flag_entity->GetState();
        if (fs.GetY() <= ps.GetY()) {
            float flagY = fs.GetY() + GameConfig::FLAGPOLE_SLIDE_SPEED;
            fs.SetY(flagY);
        }
    }

    // Check transition to POLE_WALK (delay + walk phase)
    if (ps.IsGrounded()) {
        bool flagPassedMario = true;
        if (m_flag_entity && m_flag_entity->GetState().IsActive()) {
            flagPassedMario = (m_flag_entity->GetState().GetY() >= ps.GetY());
        }
        if (flagPassedMario && ps.IsFacingRight()) {
            // Shift Mario right to the other side of the pole
            ps.SetX(ps.GetX() + GameConfig::TILE_SIZE * 0.6f);
            ps.SetFacingRight(false);  // Flip facing left
            m_Phase = Phase::POLE_WALK;
            m_tick_count = 0;  // Reset tick count for the delay
            LOG_INFO("Flagpole slide complete - Transitioning to POLE_WALK");
        }
    }
}

void FlagpoleSceneHandler::UpdatePoleWalk(Player& player, Level& level) {
    PlayerState& ps = player.GetState();

    // Wait ending walk delay (20 ticks) before starting to walk
    if (m_tick_count < GameConfig::ENDING_WALK_DELAY) {
        ps.SetVelX(0);
        ps.SetMovingRight(false);
        return;
    }

    // After wait delay, Mario turns right, let's exit pole state
    if (ps.IsPoleSliding()) {
        ps.SetPoleSliding(false);
        ps.SetFacingRight(true);
    }

    // Walk right toward the castle at normal Mario speed
    float castleWalkSpeed = GameConfig::SCALED_SPEED;
    ps.SetX(ps.GetX() + castleWalkSpeed);
    ps.SetMovingRight(true);
    ps.SetFacingRight(true);

    float castleX = level.GetCastleDoorX();
    if (castleX >= 0.0f) {
        if (ps.GetX() >= castleX) {
            m_Phase = Phase::ENTER_CASTLE;
            m_tick_count = 0;
            ps.SetMovingRight(false);
            ps.SetVelX(0);
            LOG_INFO("Flagpole walk complete - reached castle at X={}",
                     castleX);
            return;
        }
    } else {
        // Fallback if no castle block found
        if (ps.GetX() > m_goal_block_x + GameConfig::TILE_SIZE * 8) {
            m_Phase = Phase::ENTER_CASTLE;
            m_tick_count = 0;
            ps.SetMovingRight(false);
            ps.SetVelX(0);
            LOG_WARN("Castle block not found - using fallback distance");
        }
    }
}

void FlagpoleSceneHandler::UpdateEnterCastle(Player& player) {
    player.SetVisible(false);

    if (m_level_timer < 0) {
        m_level_timer = m_tick_count + GameConfig::LEVEL_TRANSITION_DELAY;
    }

    m_Phase = Phase::WAIT_TRANSITION;
    LOG_DEBUG("Mario entered castle, waiting {} ticks",
              GameConfig::LEVEL_TRANSITION_DELAY);
}

}  // namespace Mario
