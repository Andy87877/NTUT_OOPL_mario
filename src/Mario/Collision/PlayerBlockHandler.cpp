/**
 * @file PlayerBlockHandler.cpp
 * @brief Implementation of the three-step Player-Block collision pipeline.
 *        100 % ported from C# Form1.cs onTick() collision section.
 *        Pipeline order: FallDetect → CeilingTrigger → BodyResolution.
 * @inheritance PlayerBlockHandler : ICollisionHandler
 */
#include "Mario/Collision/PlayerBlockHandler.hpp"

#include "Mario/Services/AudioManager.hpp"
#include "Mario/Core/Camera.hpp"
#include "Mario/Collision/BlockContactResolver.hpp"
#include "Mario/Core/GameConfig.hpp"
#include "Mario/Level/GameStateManager.hpp"
#include "Mario/Core/PhysicsEngine.hpp"
#include "Mario/UI/UIManager.hpp"
#include "Mario/Level/Block.hpp"

namespace Mario {

// ============================================================================
// Resolve — public entry point
// ============================================================================
void PlayerBlockHandler::Resolve(Player& player, Level& level, Camera& camera,
                                 GameStateManager& gameState,
                                 UIManager& uiManager,
                                 std::vector<Level::SpawnPoint>* outSpawns) {
    PlayerState& state = player.GetState();

    // Direction flags matching C# movingDown / movingUp / movingRight /
    // movingLeft.  movingDown: airborne AND falling (fallHeight exhausted).
    //              movingUp  : still on the jump arc (fallHeight > 0).
    bool movingDown = !state.IsGrounded() && state.GetFallHeight() <= 0.0;
    bool movingUp = state.GetFallHeight() > 0.0;
    bool movingRight = state.GetVelX() > 0.0f;
    bool movingLeft = state.GetVelX() < 0.0f;

    StepFallDetect(state, level);
    StepCeilingTrigger(state, level, camera, gameState, uiManager, outSpawns,
                       movingUp);
    StepBodyResolution(state, level, camera, movingRight, movingLeft,
                       movingDown, movingUp);

    // Clamp to the left screen boundary (camera scroll edge).
    if (state.GetX() < camera.GetOffset()) {
        state.SetX(camera.GetOffset());
    }
}

// ============================================================================
// Step 1 — FallDetect
// C# marioIntersect: new Rectangle(posX, posY+1, sizeX, sizeY).
// If no solid block or moving platform intersects this strip, clear grounded.
// ============================================================================
void PlayerBlockHandler::StepFallDetect(PlayerState& state, Level& level) {
    AABB body = BlockContactResolver::BodyRect(state);
    AABB fallDetect = {body.left, body.top + 1.0f, body.right,
                       body.bottom + 1.0f};

    int tileX = static_cast<int>(body.left) / GameConfig::TILE_SIZE;
    int tileY = static_cast<int>(body.top) / GameConfig::TILE_SIZE;

    bool groundFound = false;

    for (int gy = tileY; gy <= tileY + 3 && !groundFound; gy++) {
        for (int gx = tileX - 1; gx <= tileX + 2 && !groundFound; gx++) {
            Block* blk = level.GetBlockAt(gx, gy);
            if (blk && blk->IsSolid() &&
                blk->GetAABB().Intersects(fallDetect)) {
                groundFound = true;
            }
        }
    }
    // Moving platforms: use a wider strip (3px) to tolerate the sub-pixel
    // gap that carry logic can introduce when the platform rises. Without
    // this tolerance the player briefly loses grounded status and the Jump
    // sprite flashes for one frame.
    AABB platDetect = {body.left, body.top + 1.0f, body.right,
                       body.bottom + 3.0f};
    for (auto* plat : level.GetMovingPlatforms()) {
        if (!groundFound && plat && plat->IsSolid() &&
            plat->GetAABB().Intersects(platDetect)) {
            groundFound = true;
        }
    }
    if (!groundFound) {
        state.SetGrounded(false);
    }
}

// ============================================================================
// Step 2 — CeilingTrigger
// Uses the NARROW hitbox (C# GetHitBox) while Mario is rising.
// Snaps head to block bottom and triggers block contents.
// The "row-1" extra pass handles the edge case where the head top is exactly
// on a tile boundary.
// ============================================================================
void PlayerBlockHandler::StepCeilingTrigger(
    PlayerState& state, Level& level, Camera& camera,
    GameStateManager& gameState, UIManager& uiManager,
    std::vector<Level::SpawnPoint>* outSpawns, bool& movingUp) {
    if (!movingUp) return;

    AABB hb = state.GetHitbox();  // narrow hitbox
    const float TS = static_cast<float>(GameConfig::TILE_SIZE);
    const float STR = GameConfig::INTERSECT_STRICTNESS;

    int htLeft = static_cast<int>(hb.left) / GameConfig::TILE_SIZE;
    int htRight = static_cast<int>(hb.right) / GameConfig::TILE_SIZE;
    int headRow = static_cast<int>(hb.top) / GameConfig::TILE_SIZE;

    bool triggered = false;
    for (int row = headRow - 1; row <= headRow && !triggered; row++) {
        for (int gx = htLeft; gx <= htRight && !triggered; gx++) {
            Block* blk = level.GetBlockAt(gx, row);
            if (!blk || !blk->IsSolid()) continue;

            AABB bb = blk->GetAABB();
            // Velocity-adaptive threshold: scales up at high speeds (e.g. max jump speed 27.59px/frame)
            // to prevent Mario from passing through the block in a single frame, while preserving
            // the exact 0.75 strictness ratio for slow overlap contacts.
            float threshold = std::max(TS * STR, static_cast<float>(std::abs(state.GetVelY()) + 2.0f));
            // C#: Top < b.Bottom && Top > b.Bottom - threshold && intersects
            if (hb.top < bb.bottom && hb.top > bb.bottom - threshold &&
                hb.Intersects(bb)) {
                state.SetY(bb.bottom);
                state.SetFallHeight(0.0);
                state.SetVelY(0.0);
                movingUp = false;

                TriggerBlockHit(*blk, state, camera, gameState, uiManager,
                                outSpawns);
                triggered = true;
            }
        }
    }
}

// ============================================================================
// Step 3 — BodyResolution
// Iterates all nearby solid blocks and moving platforms.
// movingDown / movingUp are shared across iterations (persist once resolved).
// movingRight / movingLeft are reset per block inside ProcessSingleBlock.
// ============================================================================
void PlayerBlockHandler::StepBodyResolution(PlayerState& state, Level& level,
                                            Camera& /*camera*/,
                                            bool movingRight, bool movingLeft,
                                            bool& movingDown, bool& movingUp) {
    AABB body = BlockContactResolver::BodyRect(state);
    int tileX = static_cast<int>(body.left) / GameConfig::TILE_SIZE;
    int tileY = static_cast<int>(body.top) / GameConfig::TILE_SIZE;

    for (int gy = tileY - 1; gy <= tileY + 3; gy++) {
        for (int gx = tileX - 1; gx <= tileX + 2; gx++) {
            Block* blk = level.GetBlockAt(gx, gy);
            if (blk && blk->IsSolid()) {
                ProcessSingleBlock(state, blk, movingDown, movingUp,
                                   movingRight, movingLeft);
            }
        }
    }
    for (auto* plat : level.GetMovingPlatforms()) {
        if (plat && plat->IsSolid()) {
            ProcessSingleBlock(state, plat, movingDown, movingUp,
                               movingRight, movingLeft);
        }
    }
}

// ============================================================================
// ProcessSingleBlock
// Applies the C# per-block resolution order for one Block.
//
// Airborne : DOWN → RIGHT → LEFT → DOWN (2nd) → UP → LEFT (2nd)
// Grounded : RIGHT or LEFT (or center-based push-out when VelX == 0)
//
// movingDown / movingUp persist across blocks (by reference).
// movingRight / movingLeft are per-block copies (by value) — this matches C#
// which resets the direction per block so one block's result cannot block
// an unrelated snap on the next block.
// ============================================================================
void PlayerBlockHandler::ProcessSingleBlock(PlayerState& state, Block* blk,
                                            bool& movingDown, bool& movingUp,
                                            bool movingRight, bool movingLeft) {
    AABB bb = blk->GetAABB();
    AABB body = BlockContactResolver::BodyRect(state);
    if (!body.Intersects(bb)) return;

    // OOP Extensibility check: vertical platforms can self-determine if they
    // should be snapped vertically first.
    if (blk->ShouldResolveVerticallyFirst(body)) {
        state.SetY(bb.top - static_cast<float>(state.GetHeight()));
        state.SetGrounded(true);
        state.SetVelY(0.0);
        state.SetFallHeight(0.0);
        movingDown = false;
        return;
    }

    // Per-block copies — match C# "localMovingRight / localMovingLeft".
    bool localMovingRight = movingRight;
    bool localMovingLeft = movingLeft;

    if (!state.IsGrounded()) {
        // Airborne resolution order (C# Form1.cs order):
        //   DOWN → RIGHT → LEFT → DOWN (again) → UP → LEFT (again)

        BlockContactResolver::ResolveDown(state, bb, movingDown);

        body = BlockContactResolver::BodyRect(state);
        if (localMovingRight && body.Intersects(bb)) {
            BlockContactResolver::ResolveRight(state, bb, localMovingRight);
        }

        body = BlockContactResolver::BodyRect(state);
        if (localMovingLeft && body.Intersects(bb)) {
            BlockContactResolver::ResolveLeft(state, bb, localMovingLeft);
        }

        // Second DOWN pass: handles corner cases where a horizontal snap puts
        // Mario back onto the block top.
        body = BlockContactResolver::BodyRect(state);
        if (body.Intersects(bb)) {
            BlockContactResolver::ResolveDown(state, bb, movingDown);
        }

        // UP: position-only snap (content trigger is done in
        // StepCeilingTrigger).
        body = BlockContactResolver::BodyRect(state);
        if (body.Intersects(bb)) {
            BlockContactResolver::ResolveUp(state, bb, movingUp);
        }

        // Second LEFT pass (C# repeats this after UP).
        body = BlockContactResolver::BodyRect(state);
        if (localMovingLeft && body.Intersects(bb)) {
            BlockContactResolver::ResolveLeft(state, bb, localMovingLeft);
        }

    } else {
        // Grounded: horizontal resolution only (C# grounded branch).
        // Stationary fallback handles the crouch-induced Y-shift overlap where
        // VelX == 0. Direction is chosen from relative centres.
        if (localMovingRight) {
            BlockContactResolver::ResolveRight(state, bb, localMovingRight);
        } else if (localMovingLeft) {
            BlockContactResolver::ResolveLeft(state, bb, localMovingLeft);
        } else {
            // Push out based on which side Mario's centre is on.
            float mCenter = (body.left + body.right) * 0.5f;
            float bCenter = (bb.left + bb.right) * 0.5f;
            bool mr = (mCenter <= bCenter);
            bool ml = !mr;
            if (mr)
                BlockContactResolver::ResolveRight(state, bb, mr);
            else
                BlockContactResolver::ResolveLeft(state, bb, ml);
        }
    }
}

// ============================================================================
// TriggerBlockHit
// Handles question-mark blocks, coin blocks, brick breaking, audio, and score.
// Called from StepCeilingTrigger when Mario's head bumps a block from below.
// ============================================================================
void PlayerBlockHandler::TriggerBlockHit(
    Block& block, PlayerState& state, Camera& camera,
    GameStateManager& gameState, UIManager& uiManager,
    std::vector<Level::SpawnPoint>* outSpawns) {
    if (block.IsHit()) return;  // block already consumed

    std::string spawnEntity;
    if (block.GetDef().isContainer) {
        spawnEntity = block.GetSpawnContents(state.GetState());
    } else if (block.GetDef().spawner) {
        spawnEntity = block.GetDef().spawnEntity;
    }

    // Convert world-space block centre to PTSD screen coordinates.
    float ptsdX = GameConfig::TopLeftToPTSDX(
        block.GetWorldX(), GameConfig::TILE_SIZE, camera.GetOffset());
    float ptsdY =
        GameConfig::TopLeftToPTSDY(block.GetWorldY(), GameConfig::TILE_SIZE);

    if (spawnEntity == "CoinGet") {
        gameState.AddCoin();
        gameState.AddScore(200);
        AudioManager::GetInstance().PlaySFX(SFXName::Coin);
        uiManager.AddFloatingText(ptsdX, ptsdY, "+200", 60);
    } else if (!spawnEntity.empty() && outSpawns) {
        Level::SpawnPoint sp;
        sp.entityName = spawnEntity;
        sp.gridX = block.GetGridX();
        sp.gridY = block.GetGridY();
        sp.worldX =
            static_cast<float>(block.GetGridX() * GameConfig::TILE_SIZE);
        sp.worldY =
            static_cast<float>(block.GetGridY() * GameConfig::TILE_SIZE);
        sp.spawned = true;
        outSpawns->push_back(sp);

        if (spawnEntity == "Coin" || spawnEntity == "CoinText") {
            AudioManager::GetInstance().PlaySFX(SFXName::Coin);
        } else {
            AudioManager::GetInstance().PlaySFX(SFXName::Item);
            uiManager.AddFloatingText(ptsdX, ptsdY, "+100", 60);
        }
    }

    block.OnHit(state.GetState());
    // Brick debris is spawned by PlayingSceneHandler via block->JustBroken().
}

}  // namespace Mario
