# Super Mario Bros. PTSD C++ OOP 架構設計 (Constructure)
<!-- Last synced: 2026-05-25 — OOP refactor: (1) Level::IsUnderground() — level knows its own background type (name-based); (2) AudioManager::PlayBGMForLevel(levelName, timeRemaining) added to IAudioService interface + AudioManager — centralises level→BGM mapping so App never inspects level names for audio; (3) App::IsUnderground() merges GameStateManager runtime flag + Level::IsUnderground(); App::ApplyBackground() no-arg overload delegates to IsUnderground(); (4) Eliminated duplicated 4-line underground-detection block from PlayingSceneHandler, FlagpoleSceneHandler, DeathSceneHandler, PipeWarpSceneHandler — all now call app.ApplyBackground(); (5) App::PlayCurrentBGM() reduced to a 1-liner. AxeKoopa axe-throw fix, Bowser fire fix, PiranhaPlant fixes from previous sessions still apply. -->

本專案將 C# 版本的 God Class (`Form1.cs`) 徹底解耦，轉換為符合現代 C++ 標準的
**深度物件導向架構 (Deep OOP Architecture)**。
設計上大量運用**繼承 (Inheritance)**、**多型 (Polymorphism)**、**介面 (Interfaces)** 與五大**設計模式 (Design Patterns)**。

---

## 目錄

1. 完整 UML 繼承圖
2. 所有檔案清單
3. 設計模式深度解析
4. Game Loop — 17 Phase 架構
5. App::State 狀態機轉移圖
6. Refactoring 進度總覽

---

## 1. 完整 UML 繼承圖

### 1.1 PTSD GameObject 繼承樹 (@inheritance 標記在各 .hpp)

```mermaid
classDiagram
    direction TB

    class GameObject {
        <<PTSD Framework>>
        +SetDrawable()
        +SetVisible(bool)
        +SetZIndex(float)
        +m_Transform
    }

    class Block {
        <<Util::GameObject>>
        -int m_BlockID, m_GridX, m_GridY
        -BlockDef m_Def
        -bool m_Solid
        -int m_BounceTimer
        +Update(cameraOffset)
        +OnHit(playerState)
        +Bounce() / Break()
        +GetAABB() AABB
        +IsSolid() / IsBreakable() / IsGoal()
        +JustBroken() bool
    }

    class MovingPlatform {
        <<Block>>
        -Direction m_Dir
        -float m_Velocity, m_MinBound, m_MaxBound
        -float m_LastDeltaX, m_LastDeltaY
        +StepMovement()
        +GetLastDeltaX() float
        +GetLastDeltaY() float
    }

    class Entity {
        <<Util::GameObject>>
        -EntityDef m_Def
        -EntityState m_State
        -unique_ptr~IEntityBehavior~ m_Behavior
        -string m_LevelName
        +UpdateView(cameraOffset)
        +SetBehavior(behavior)
        +GetBehavior() IEntityBehavior*
        +GetState() EntityState&
    }

    class Player {
        <<Util::GameObject>>
        -PlayerState m_State
        -bool m_Visible
        +UpdateView(cameraOffset)
        +GetState() PlayerState&
        +SetVisible(bool)
    }

    class UIImage {
        <<Util::GameObject>>
        +SetPosition(x, y)
        +SetImagePath(path)
    }

    class UIText {
        <<Util::GameObject>>
        +SetTextContent(text)
        +SetTextColor(color)
        +SetPosition(x, y)
    }

    GameObject <|-- Block
    Block <|-- MovingPlatform
    GameObject <|-- Entity
    GameObject <|-- Player
    GameObject <|-- UIImage
    GameObject <|-- UIText
```

### 1.2 ISceneHandler 繼承樹 (State Pattern)

```mermaid
classDiagram
    direction TB

    class ISceneHandler {
        <<interface>>
        +OnEnter(App&)
        +Update(App&)*
        +OnRender(App&)*
        +OnExit(App&)
        +GetName() const char**
    }

    class TitleSceneHandler { <<ISceneHandler>> }
    class LoadingSceneHandler { <<ISceneHandler>> }
    class PlayingSceneHandler {
        <<ISceneHandler>>
        17-Phase game loop
        -SpawnPlayerFireball()
        -SpawnBrickDebris()
        -CheckFlagpoleCollision()
        -CheckPipeCollision()
        -CheckAxeCollision()
        -CheckPlayerEntityCollision()
        -CheckEntityEntityCollision()
        -CleanupDeadEntities()
    }
    class FlagpoleSceneHandler { <<ISceneHandler>> }
    class PipeWarpSceneHandler { <<ISceneHandler>> }
    class AxeSequenceSceneHandler { <<ISceneHandler>> }
    class DeathSceneHandler { <<ISceneHandler>> }
    class GameOverSceneHandler { <<ISceneHandler>> }
    class GameWonSceneHandler { <<ISceneHandler>> }
    class ESCMenuSceneHandler { <<ISceneHandler>> }

    ISceneHandler <|.. TitleSceneHandler
    ISceneHandler <|.. LoadingSceneHandler
    ISceneHandler <|.. PlayingSceneHandler
    ISceneHandler <|.. FlagpoleSceneHandler
    ISceneHandler <|.. PipeWarpSceneHandler
    ISceneHandler <|.. AxeSequenceSceneHandler
    ISceneHandler <|.. DeathSceneHandler
    ISceneHandler <|.. GameOverSceneHandler
    ISceneHandler <|.. GameWonSceneHandler
    ISceneHandler <|.. ESCMenuSceneHandler
```

### 1.3 IEntityBehavior 繼承樹 (Strategy Pattern)

```mermaid
classDiagram
    direction TB

    class IEntityBehavior {
        <<interface>>
        +Update(state, level, player, timer)*
        +OnPlayerCollision(state, player, isFromAbove) bool*
        +Clone() unique_ptr*
        +GetName() const char**
        +OnFireballHit(state) bool
        +AlwaysUpdate() bool
        +OnSpawned(vx, vy) void
        +ConsumeSpawnRequest(EntityType, x, y, dir) bool
    }

    class EnemyBehavior { <<IEntityBehavior>> Goomba patrol+squish }
    class KoopaBehavior {
        <<IEntityBehavior>>
        KoopaTroopa OR Shell
        -KoopaMode m_Mode
    }
    class ParaKoopaBehavior {
        <<IEntityBehavior>>
        Flying Koopa
        -float m_FloatPhase
    }
    class AxeKoopaBehavior {
        <<IEntityBehavior>>
        Axe-throwing Koopa
        +ConsumeSpawnRequest()
    }
    class BowserBehavior {
        <<IEntityBehavior>>
        Boss 5-Phase AI
        -BowserPhase m_Phase
        -int m_HealthPoints
        +OnFireballHit()
        +ConsumeSpawnRequest()
    }
    class FireballBehavior { <<IEntityBehavior>> Parabolic fireball }
    class ItemBehavior { <<IEntityBehavior>> Mushroom/Star/FireFlower/1UP }
    class AxeBehavior { <<IEntityBehavior>> Bridge axe kill-trigger 8-4 }
    class PrincessBehavior { <<IEntityBehavior>> NPC goal marker }
    class PiranhaPlantBehavior {
        <<IEntityBehavior>>
        Pipe plant 4-Phase cycle
        -Phase m_Phase
    }
    class PodobooBehavior { <<IEntityBehavior>> Lava bubble immortal }
    class DefaultEntityBehavior { <<IEntityBehavior>> Passive coin/flag }
    class ParticleDebris {
        <<IEntityBehavior>>
        Brick debris particles
        -float m_InitialVelX, m_InitialVelY
        -int m_LifetimeFrames
        +AlwaysUpdate() true
        +OnSpawned(vx, vy): SetInitialVelocity
    }

    IEntityBehavior <|.. EnemyBehavior
    IEntityBehavior <|.. KoopaBehavior
    IEntityBehavior <|.. ParaKoopaBehavior
    IEntityBehavior <|.. AxeKoopaBehavior
    IEntityBehavior <|.. BowserBehavior
    IEntityBehavior <|.. FireballBehavior
    IEntityBehavior <|.. ItemBehavior
    IEntityBehavior <|.. AxeBehavior
    IEntityBehavior <|.. PrincessBehavior
    IEntityBehavior <|.. PiranhaPlantBehavior
    IEntityBehavior <|.. PodobooBehavior
    IEntityBehavior <|.. DefaultEntityBehavior
    IEntityBehavior <|.. ParticleDebris
```

### 1.4 IAudioService 繼承樹 (DIP)

```mermaid
classDiagram
    direction TB

    class IAudioService {
        <<interface>>
        +PlayBGM(BGMName)*
        +PlaySFX(SFXName)*
        +StopBGM()*
    }
    class AudioManager {
        <<IAudioService + Singleton>>
        -map BGMCache, SFXCache
        +GetInstance() AudioManager&
        +PlayBGM() / PlaySFX() / StopBGM()
    }
    class AudioPathResolver {
        <<static helper>>
        +GetBGMPath(name)$
        +GetSFXPath(name)$
    }
    class AudioType {
        <<enum header>>
        BGMName (21 values)
        SFXName (20 values)
    }

    IAudioService <|.. AudioManager
    AudioManager --> AudioPathResolver
    AudioManager --> AudioType : uses enums
```

### 1.5 App 全域架構圖

```mermaid
classDiagram
    direction LR

    class App {
        -State m_CurrentState
        -unique_ptr~ISceneHandler~ m_CurrentHandler
        -Camera m_Camera
        -Renderer m_Renderer
        -shared_ptr~Level~ m_Level
        -shared_ptr~Player~ m_Player
        -InputHandler m_InputHandler
        -CollisionManager m_CollisionManager
        -GameStateManager m_GameState
        -LevelCompleteController m_LevelCompleteCtrl
        -unique_ptr~UIManager~ m_UIManager
        -vector~shared_ptr~Entity~~ m_Entities
        +TransitionTo(State)
        +LoadLevel(name)
        +AddEntityToGame(entity)
        +ApplyBackground(bool)
    }

    App --> Camera
    App --> Level
    App --> CollisionManager
    App --> GameStateManager
    App --> InputHandler
    App --> LevelCompleteController
    App --> UIManager
    App --> EntityFactory : uses static
    EntityFactory --> EnemyDeathStyleFactory : selects death strategy
    App --> PhysicsEngine : uses static
    App --> AudioManager : uses singleton
    App --> ServiceLocator : registers IAudioService
    ServiceLocator --> IAudioService : holds
    EventSystem --> ISceneHandler : loose coupling (unused hooks)
```

### 1.6 MVC 完整關係圖

        -float m_PosX, m_PosY, m_VelX
        -double m_VelY, m_FallHeight
        -PowerState m_PowerState, m_MemoryState
        -int m_InvTimer, m_AnimFrame
        -unique_ptr~IPlayerDeathAnimation~ m_DeathAnimation
        +Init() / Tick()
        +ApplyMovement(speed)
        +ApplyGravity() float
        +GetAABB() AABB
        +BuildAnimationKey() string
        +TakeDamage() / IsInvincible()
        +StartDeathAnimation()
        +UpdateDeathAnimation()
        +IsDeathAnimActive() bool
        +GetMemoryState() PowerState
        +SetMemoryState(PowerState)
    }
    class IPlayerDeathAnimation {
        <<interface>>
        +Start()*
        +Tick(gravity, tickInterval, jumpVelocity, playerY)*
        +IsActive() bool*
    }
    class ClassicPlayerDeathAnimation {
        <<IPlayerDeathAnimation>>
        freeze 12 frames then launch/fall
        -bool m_Active, m_Launched
        -int m_FrameCounter
        -double m_VelY
    }
    class Player {
        <<View - Util::GameObject>>
        -PlayerState m_State
        -bool m_Visible
        +UpdateView(cameraOffset)
        +SetVisible(bool)
        NOTE: SetVisible sets m_Visible
        NOT for blink - use PTSD base directly
    }
    class InputHandler {
        <<Controller>>
        +HandleInput(PlayerState, speed)
    }

    class EntityState {
        <<Model>>
        -float m_PosX, m_PosY, m_VelX
        -double m_VelY, m_FallHeight
        -bool m_Active, m_IsEnemy, m_Deleted
        -int m_AnimFrame, m_ScoreWorth
        -unique_ptr~IEnemyDeathAnimation~ m_DeathAnimation
        +Init() / Tick()
        +GetAABB() AABB
        +SetDeleted(bool) / IsDeleted()
    }
    class IEnemyDeathAnimation {
        <<interface>>
        +Start(cause, runtime)*
        +Tick(runtime, gravity, tickInterval)*
        +IsActive() bool*
    }
    class GoombaSquishDeathAnimation {
        <<IEnemyDeathAnimation>>
        stomp -> squash hold -> delete
    }
    class KoopaRetreatDeathAnimation {
        <<IEnemyDeathAnimation>>
        stomp -> shell retreat
        fire/shell/star -> flip die
    }
    class FireballFlipDeathAnimation {
        <<IEnemyDeathAnimation>>
        flip arc then despawn
    }
    class ClassicEnemyDeathAnimation {
        <<IEnemyDeathAnimation>>
        default generic fallback
    }
    class EnemyDeathStyleFactory {
        <<Factory>>
        +CreateFor(type) unique_ptr~IEnemyDeathAnimation~
    }
    class EntityFactory {
        <<Factory>>
        +CreateEntity(def, x, y, dir, fromBlock, levelName)
    }
    class Entity {
        <<View - Util::GameObject>>
        -EntityDef m_Def
        -EntityState m_State
        -unique_ptr~IEntityBehavior~ m_Behavior
        +UpdateView(cameraOffset)
        +GetState() EntityState&
        +GetBehavior() IEntityBehavior*
    }

    InputHandler --> PlayerState : writes
    Player --> PlayerState : owns
    PlayerState --> IPlayerDeathAnimation : owns (strategy)
    IPlayerDeathAnimation <|.. ClassicPlayerDeathAnimation
    Entity --> EntityState : owns
    EntityState --> IEnemyDeathAnimation : owns (strategy)
    IEnemyDeathAnimation <|.. GoombaSquishDeathAnimation
    IEnemyDeathAnimation <|.. KoopaRetreatDeathAnimation
    IEnemyDeathAnimation <|.. FireballFlipDeathAnimation
    IEnemyDeathAnimation <|.. ClassicEnemyDeathAnimation
    EntityFactory --> EnemyDeathStyleFactory : create strategy
    Entity --> IEntityBehavior : owns (polymorphic dispatch)

```

### 1.7 ServiceLocator & EventSystem

```mermaid
classDiagram
    direction TB

    class ServiceLocator {
        <<Singleton>>
        -map~string, shared_ptr~ m_Services
        +GetInstance()$ ServiceLocator&
        +RegisterService~T~(service)
        +GetService~T~() shared_ptr~T~
        +HasService~T~() bool
    }

    class EventSystem~T~ {
        <<template>>
        -map~int, EventListener~ m_Listeners
        -int m_NextId
        +Subscribe(listener) int
        +Unsubscribe(id)
        +Publish(event)
        +Clear()
    }

    class CollisionContext {
        <<data struct>>
        +shared_ptr~Player~ player
        +shared_ptr~Level~ level
        +EntityFactory* entityFactory
        +GameStateManager* gameState
        +int gameTimer
        +int invTimer
    }

    ServiceLocator --> IAudioService : holds as shared_ptr
    IAudioService <|.. AudioManager
    EventSystem --> ISceneHandler : decoupled event hook
```

### 1.8 ICollisionHandler 繼承樹 (Strategy + Facade Pattern)

```mermaid
classDiagram
    direction TB

    class ICollisionHandler {
        <<abstract interface>>
        +~ICollisionHandler()
    }

    class BlockContactResolver {
        <<static utility — no instances>>
        +BodyRect(state)$ AABB
        +ResolveDown(state, bb, movingDown)$
        +ResolveUp(state, bb, movingUp)$
        +ResolveRight(state, bb, movingRight)$
        +ResolveLeft(state, bb, movingLeft)$
    }

    class PlayerBlockHandler {
        <<ICollisionHandler>>
        Step 1 FallDetect — strip below feet
        Step 2 CeilingTrigger — head-bump + block contents
        Step 3 BodyResolution — C# order DOWN/RIGHT/LEFT/DOWN/UP/LEFT
        +Resolve(player, level, camera, gameState, ui, spawns)
        -StepFallDetect()
        -StepCeilingTrigger()
        -StepBodyResolution()
        -ProcessSingleBlock()
        -TriggerBlockHit()
    }

    class PlayerEntityHandler {
        <<ICollisionHandler>>
        -int m_StompCombo
        Stomp NES combo ×1×2×4×8→1000
        Star power instant-kill
        Shell kick / side damage
        Power-up and coin collection
        +Resolve(player, entities, camera, gameState, ui)
        -HandleEnemyCollision()
        -HandleItemCollision()
    }

    class EntityBlockHandler {
        <<ICollisionHandler>>
        Ground snap / Wall flip
        Fireball→Explosion on wall
        Pit deactivation
        +Resolve(entity, level, outNewEntities)
        -CheckGround()
        -CheckWalls()
    }

    class EntityEntityHandler {
        <<ICollisionHandler>>
        Fireball vs Enemy → kill + delete
        Moving Shell vs Enemy → kill
        Camera culling for off-screen pairs
        +Resolve(entities, gameState, cameraOffset)
        -IsMovingShell()$
    }

    class CollisionManager {
        <<Facade — no logic of its own>>
        -PlayerBlockHandler m_PlayerBlockHandler
        -PlayerEntityHandler m_PlayerEntityHandler
        -EntityBlockHandler m_EntityBlockHandler
        -EntityEntityHandler m_EntityEntityHandler
        +CheckPlayerBlockCollision()
        +CheckPitFall() bool
        +CheckPlayerEntityCollision()
        +CheckEntityEntityCollision()
        +CheckEntityBlockCollision()
    }

    ICollisionHandler <|-- PlayerBlockHandler
    ICollisionHandler <|-- PlayerEntityHandler
    ICollisionHandler <|-- EntityBlockHandler
    ICollisionHandler <|-- EntityEntityHandler

    PlayerBlockHandler --> BlockContactResolver : static helpers
    CollisionManager *-- PlayerBlockHandler
    CollisionManager *-- PlayerEntityHandler
    CollisionManager *-- EntityBlockHandler
    CollisionManager *-- EntityEntityHandler
```

---

## 2. 所有檔案清單

### 2.1 Include Headers (`include/`)

| 檔案 | 類別 / 結構 | @inheritance | 職責 |
|------|------------|-------------|------|
| `App.hpp` | `App` | None | 持有子系統；State 切換；存取器 API；`IsUnderground()` 合併 GameStateManager + Level 的地下判斷；`ApplyBackground()` 無參數重載 |
| `Mario/GameConfig.hpp` | `GameConfig` | None (static consts) | 全域常數 + 座標轉換靜態 helpers |
| `Mario/Collider.hpp` | `AABB` | None (data struct) | AABB 矩形 + Intersects() (strict inequality) |
| `Mario/CollisionContext.hpp` | `CollisionContext` | None (data struct) | 碰撞解析資料傳遞物件 (DTO)，攜帶 Player/Level/EntityFactory/GameStateManager 參考 |
| `Mario/Camera.hpp` | `Camera` | None | 橫向捲動 offset；8-4 Boss 鎖屏；world to screen 轉換 |
| `Mario/PhysicsEngine.hpp` | `PhysicsEngine` | None (static) | ApplyGravity() + GetJumpHeight() |
| `Mario/SpritePathResolver.hpp` | `SpritePathResolver` | None (static) | Sprite 路徑解析 Block/Player/Entity（unordered_map 靜態表） |
| `Mario/EntityDef.hpp` | `EntityDef`, `BlockDef`, `EntityType` | None (data) | CSV 資料結構；EntityType 列舉 |
| `Mario/Block.hpp` | `Block` | `Util::GameObject` | 磚塊：碰撞/動畫/hit/bounce/break；靜態跨實例 Sprite Cache |
| `Mario/MovingPlatform.hpp` | `MovingPlatform` | `Block` | 移動平台（1-2 垂直 / 8-4 水平） |
| `Mario/Level.hpp` | `Level` | None | CSV 解析；Block 2D 格；SpawnPoint；`GetGoalBlocks()` 快取；`QueryBlocksInRange(out)` 零分配版；`IsUnderground()` 名稱判斷 |
| `Mario/EntityState.hpp` | `EntityState` | None (Model) | Entity MVC Model：位置/速度/動畫/死亡策略 |
| `Mario/EnemyDeathAnimation.hpp` | `IEnemyDeathAnimation`, `GoombaSquishDeathAnimation`, `KoopaRetreatDeathAnimation`, `FireballFlipDeathAnimation`, `ClassicEnemyDeathAnimation`, `EnemyDeathRuntime`, `EnemyDeathCause` | `IEnemyDeathAnimation <- {GoombaSquish, KoopaRetreat, FireballFlip, Classic}` | 敵人死亡動畫多策略（踩踏/龜殼/火球/通用） |
| `Mario/EnemyDeathStyleFactory.hpp` | `EnemyDeathStyleFactory` | None (Factory) | 依 EntityType 注入對應敵人死亡策略 |
| `Mario/Entity.hpp` | `Entity` | `Util::GameObject` | Entity View：渲染 + Strategy 行為；Z-index 自決策 |
| `Mario/EntityFactory.hpp` | `EntityFactory` | None (Factory) | 唯一 Entity 建立入口 |
| `Mario/PlayerState.hpp` | `PlayerState`, `PowerState` | None (Model) | Player MVC Model：物理/狀態/動畫key/死亡策略 |
| `Mario/PlayerDeathAnimation.hpp` | `IPlayerDeathAnimation`, `ClassicPlayerDeathAnimation` | `IPlayerDeathAnimation <- ClassicPlayerDeathAnimation` | 玩家死亡動畫策略（凍結/起跳/下墜） |
| `Mario/Player.hpp` | `Player` | `Util::GameObject` | Player View：渲染 + m_Visible 守衛 |
| `Mario/InputHandler.hpp` | `InputHandler` | None (Controller) | MVC Controller：鍵盤 to PlayerState |
| `Mario/CollisionManager.hpp` | `CollisionManager` | None (facade) | Collision 子系統協調者（Facade Pattern）；公開 API 不變；邏輯委派給 Collision/ 四個 Handler |
| `Mario/Collision/ICollisionHandler.hpp` | `ICollisionHandler` | None (abstract interface) | 所有碰撞 Handler 的標記基類；定義繼承樹頂點 |
| `Mario/Collision/BlockContactResolver.hpp` | `BlockContactResolver` | None (static utility) | 靜態 Down/Up/Right/Left AABB Snap helpers；BodyRect() 建立全寬碰撞體（C# GetRecPosition 等效） |
| `Mario/Collision/PlayerBlockHandler.hpp` | `PlayerBlockHandler` | `ICollisionHandler` | 玩家-方塊三步驟管線：FallDetect → CeilingTrigger → BodyResolution；ProcessSingleBlock 取代原 lambda |
| `Mario/Collision/PlayerEntityHandler.hpp` | `PlayerEntityHandler` | `ICollisionHandler` | 玩家-實體碰撞：踩踏 NES Combo / 傷害 / 道具收集；m_StompCombo 在此管理 |
| `Mario/Collision/EntityBlockHandler.hpp` | `EntityBlockHandler` | `ICollisionHandler` | 實體-方塊碰撞：地面 Snap / 牆壁翻向 / Fireball→Explosion / 落坑刪除 |
| `Mario/Collision/EntityEntityHandler.hpp` | `EntityEntityHandler` | `ICollisionHandler` | 實體-實體碰撞：火球 vs 敵人 / 移動龜殼 vs 敵人；viewport culling |
| `Mario/LevelCompleteController.hpp` | `LevelCompleteController`, `EndingPhase` | None | 旗杆/水管/Bowser 結局序列 |
| `Mario/GameStateManager.hpp` | `GameStateManager` | None (Service) | 分數/生命/金幣/時間/關卡進度 |
| `Mario/ISceneHandler.hpp` | `ISceneHandler` | None (interface) | State Pattern 純虛介面（11 個實作） |
| `Mario/MenuSceneHandlers.hpp` | `TitleSceneHandler`, `DeathSceneHandler`, `GameOverSceneHandler`, `GameWonSceneHandler` | `ISceneHandler` | 選單/死亡/結束場景（合併） |
| `Mario/LoadingSceneHandler.hpp` | `LoadingSceneHandler` | `ISceneHandler` | 加載畫面（顯示 WORLD X-X + LIVES） |
| `Mario/PlayingSceneHandler.hpp` | `PlayingSceneHandler` | `ISceneHandler` | 主遊戲迴圈（17-phase） |
| `Mario/FlagpoleSceneHandler.hpp` | `FlagpoleSceneHandler` | `ISceneHandler` | 旗杆滑動序列 |
| `Mario/PipeWarpSceneHandler.hpp` | `PipeWarpSceneHandler` | `ISceneHandler` | 水管傳送過場 |
| `Mario/AxeSequenceSceneHandler.hpp` | `AxeSequenceSceneHandler` | `ISceneHandler` | 8-4 Bowser 擊敗序列 |
| `Mario/ESCMenuSceneHandler.hpp` | `ESCMenuSceneHandler` | `ISceneHandler` | ESC 暫停選單；5 項選單（RESUME/1-1/1-2/8-4/**POWER**）；`OnEnter()` 從玩家當前 PowerState 初始化 `m_PowerStateIndex`；`GetPowerStateName(idx)` 靜態輔助；ENTER 鍵觸發 `ForceApplyPowerState()` |
| `Mario/AudioType.hpp` | `BGMName` (21), `SFXName` (20) | None (enum header) | 所有音效枚舉定義（從 AudioManager.hpp 拆出） |
| `Mario/AudioManager.hpp` | `IAudioService`, `AudioManager`, `AudioPathResolver` | `IAudioService <- AudioManager` | 音效全系統（Singleton + DIP 抽象）；`PlayBGMForLevel(levelName, time)` 集中管理 level→BGM 映射 |
| `Mario/IAudioService.hpp` | `IAudioService` | None (interface) | 音效抽象介面（DIP） |
| `Mario/ServiceLocator.hpp` | `ServiceLocator` | None (Service Locator) | 服務定位器 Singleton；type-safe `RegisterService<T>` / `GetService<T>` |
| `Mario/EventSystem.hpp` | `EventSystem<T>` | None (template) | 泛型 Pub/Sub 事件系統；`Subscribe` / `Unsubscribe` / `Publish` |
| `Mario/UIManager.hpp` | `UIManager` | None | HUD + FloatingText + 場景文字 + FPS 顯示；`Update(state, sel, powerStateName)` 第三參數預設"SMALL"，轉送給 `UpdateESCMenu()` 更新 POWER 選項文字 |
| `Mario/UIWidgets.hpp` | `UIImage`, `UIText` | `Util::GameObject <- UIImage/UIText` | 輕量 UI 元件（合併） |
| `Mario/FloatingText.hpp` | `FloatingText` | None | 漂浮分數文字（60 幀淡出） |
| `Mario/CoinUI.hpp` | `CoinUI` | None (composite) | 金幣動畫圖示 + 計數文字 |
| `Mario/Behaviors/IEntityBehavior.hpp` | `IEntityBehavior` | None (interface) | Strategy Pattern 純虛介面（13 個實作） |
| `Mario/Behaviors/EnemyBehavior.hpp` | `EnemyBehavior` | `IEntityBehavior` | Goomba AI |
| `Mario/Behaviors/KoopaFamily.hpp` | `KoopaBehavior`, `ParaKoopaBehavior`, `AxeKoopaBehavior` | `IEntityBehavior` | Koopa 系列 AI（合併）；`AxeKoopaBehavior` 使用 ConsumeSpawnRequest 生成斧頭（pending-flag 模式，與 BowserBehavior 一致） |
| `Mario/Behaviors/BowserBehavior.hpp` | `BowserBehavior` | `IEntityBehavior` | Boss 5-Phase AI + HP 系統 |
| `Mario/Behaviors/FireballBehavior.hpp` | `FireballBehavior` | `IEntityBehavior` | 拋物線火球 |
| `Mario/Behaviors/ItemBehavior.hpp` | `ItemBehavior` | `IEntityBehavior` | 道具彈跳收集；`ItemType` 枚舉確保各道具行為獨立 |
| `Mario/Behaviors/StaticEntityBehaviors.hpp` | `AxeBehavior`, `PrincessBehavior` | `IEntityBehavior` | 8-4 靜態觸發器/NPC（合併） |
| `Mario/Behaviors/PiranhaPlantBehavior.hpp` | `PiranhaPlantBehavior` | `IEntityBehavior` | 水管食人花 4-Phase；管口安全半徑 1.5×TILE |
| `Mario/Behaviors/PodobooBehavior.hpp` | `PodobooBehavior` | `IEntityBehavior` | 熔岩泡泡（不可擊殺） |
| `Mario/Behaviors/DefaultEntityBehavior.hpp` | `DefaultEntityBehavior` | `IEntityBehavior` | 被動實體（金幣/旗幟） |
| `Mario/Behaviors/ParticleDebris.hpp` | `ParticleDebris` | `IEntityBehavior` | 磚塊破碎粒子 |

**Note:** `GameTheater.hpp`、`SceneManager.hpp` 及其 `.cpp` 是已被 State Pattern 取代的孤兒檔案，已從磁碟**永久刪除**。

### 2.2 Source Files (`src/`)

| 檔案 | 行數 (約) | 備註 |
|------|---------|------|
| `App.cpp` | ~220 | TransitionTo + LoadLevel + accessor impls；移除 Z-index 覆寫 (Bug #18) |
| `Mario/Camera.cpp` | ~80 | 8-4 Boss 鎖屏邏輯 (Bug #17) |
| `Mario/PhysicsEngine.cpp` | ~40 | |
| `Mario/SpritePathResolver.cpp` | ~220 | 全 unordered_map 靜態表；補齊城堡/出生點 mappings (Bug #13, #14) |
| `Mario/Block.cpp` | ~200 | 靜態 s_BlockSpriteCache；像素對齊渲染 (Bug #12, #23) |
| `Mario/MovingPlatform.cpp` | ~80 | WorldToScreen 使用統一轉換 helper (Bug #24) |
| `Mario/Level.cpp` | ~300 | CSV 解析；旗幟 X 修正 (Bug #3)；出生點落入實例化 (Bug #13)；城堡門 (Bug #14) |
| `Mario/PlayerState.cpp` | ~260 | 死亡策略；蹲下高度動態調整 (Bug #20)；階梯式退化 (Bug #25)；`ForceApplyPowerState(idx)` 作弊器 — 含 Y 位置調整、`m_StarTimer` 設置 |
| `Mario/PlayerDeathAnimation.cpp` | ~60 | ClassicPlayerDeathAnimation 策略實作 |
| `Mario/Player.cpp` | ~180 | 死亡精靈鎖定 (Bug #19)；閃爍 PTSD 基類直呼叫 (Bug #5)；像素對齊 (Bug #12) |
| `Mario/InputHandler.cpp` | ~80 | 全寬 uncrouch guard (Bug #20) |
| `Mario/EntityState.cpp` | ~160 | 死亡策略整合 |
| `Mario/EnemyDeathAnimation.cpp` | ~80 | 四種死亡策略實作 |
| `Mario/EnemyDeathStyleFactory.cpp` | ~40 | 依 EntityType 選策略 |
| `Mario/Entity.cpp` | ~150 | 靜態 s_EntitySpriteCache；Z-index 自決策（PiranhaPlant, COIN）(Bug #18, #29) |
| `Mario/EntityFactory.cpp` | ~200 | AXE->AxeBehavior (Bug #8)；COIN/STAR/FIRE_FLOWER/ONE_UP ItemType 精確注入 (Bug #28) |
| `Mario/CollisionManager.cpp` | ~65 | **Facade only** — 5 個 CheckXxx 方法各自委派給對應 Handler；全部邏輯已移至 Collision/ 子系統 |
| `Mario/Collision/BlockContactResolver.cpp` | ~50 | 靜態 Down/Up/Right/Left 解析方法；BodyRect helper（原 file-scope static 函數） |
| `Mario/Collision/PlayerBlockHandler.cpp` | ~180 | 三步驟管線 + ProcessSingleBlock（取代原 lambda）+ TriggerBlockHit（原私有方法）|
| `Mario/Collision/PlayerEntityHandler.cpp` | ~160 | HandleEnemyCollision + HandleItemCollision；m_StompCombo NES Combo 計數 |
| `Mario/Collision/EntityBlockHandler.cpp` | ~90 | CheckGround + CheckWalls（Fireball→Explosion spawn）|
| `Mario/Collision/EntityEntityHandler.cpp` | ~70 | Fireball vs Enemy + Moving Shell vs Enemy；IsMovingShell() static helper |
| `Mario/LevelCompleteController.cpp` | ~320 | 旗杆 Y 修正 (Bug #15)；8-4 通關重力釋放 (Bug #27) |
| `Mario/GameStateManager.cpp` | ~80 | |
| `Mario/MenuSceneHandlers.cpp` | ~180 | 死亡場景接管生命扣除與動畫 (Bug #19)；GameWon 黑底 (Bug #26) |
| `Mario/LoadingSceneHandler.cpp` | ~60 | 強制黑色背景 (Bug #16) |
| `Mario/PlayingSceneHandler.cpp` | ~380 | X-only 旗杆 fallback (Bug #4)；pipe 展開 box (Bug #21)；viewport entity culling (Bug #23) |
| `Mario/FlagpoleSceneHandler.cpp` | ~70 | Camera lockoff 傳入 (Bug #17) |
| `Mario/PipeWarpSceneHandler.cpp` | ~60 | |
| `Mario/AxeSequenceSceneHandler.cpp` | ~90 | Camera lockoff 解除 (Bug #17) |
| `Mario/ESCMenuSceneHandler.cpp` | ~100 | 5-item menu logic; case 4 cycles power cheat + calls ForceApplyPowerState() |
| `Mario/UIManager.cpp` | ~380 | InitLoadingScreen 預載 (Bug #16)；FPS+版權文字 (Bug #22)；座標 helper (Bug #24) |
| `Mario/AudioManager.cpp` | ~220 | AudioPathResolver 實作 |
| `Mario/AudioPathResolver.cpp` | ~80 | BGM/SFX 路徑映射 |
| `Mario/CoinUI.cpp` | ~40 | |
| `Mario/FloatingText.cpp` | ~40 | |
| `Mario/Behaviors/EnemyBehavior.cpp` | ~100 | |
| `Mario/Behaviors/KoopaFamily.cpp` | ~220 | |
| `Mario/Behaviors/BowserBehavior.cpp` | ~260 | 方向修正 (Bug #9) |
| `Mario/Behaviors/FireballBehavior.cpp` | ~80 | |
| `Mario/Behaviors/ItemBehavior.cpp` | ~110 | ItemType 精確分流 (Bug #28) |
| `Mario/Behaviors/StaticEntityBehaviors.cpp` | ~60 | |
| `Mario/Behaviors/PiranhaPlantBehavior.cpp` | ~140 | 居中修正 + 安全半徑 (Bug #18) |
| `Mario/Behaviors/PodobooBehavior.cpp` | ~80 | |
| `Mario/Behaviors/DefaultEntityBehavior.cpp` | ~40 | |
| `Mario/Behaviors/ParticleDebris.cpp` | ~60 | |

**Total: 88 source files, ~9,000 lines of C++17 OOP code**

### 2.3 Resources

| 路徑 | 內容 |
|------|------|
| `Resources/Levels/1-1.csv` | 地面關卡（16x220 格） |
| `Resources/Levels/1-2.csv` | 地下關卡（16x220 格） |
| `Resources/Levels/8-4.csv` | 城堡關卡（15x392 格）— generate_8-4_map.py 生成 |
| `Resources/LookUpSheet/IDList.csv` | Block 定義表 ID to name/solid/breakable/... |
| `Resources/LookUpSheet/EntityList.csv` | Entity 定義表 ID to name/type/isEnemy/score/... |
| `Resources/Sprites/` | 所有 sprite PNG（Block/Player/Entity/UI） |
| `Resources/Audio/` | 所有 BGM（.ogg）與 SFX（.wav） |
| `Resources/Font/` | 遊戲字型 |

### 2.4 GameConfig 關鍵常數

| 常數 | 值 | 說明 |
|------|----|------|
| `TILE_SIZE` | 45 | 像素/格（720/16=45，垂直剛好填滿） |
| `DRAW_SCALE` | 45.0f/32.0f = 1.40625f | 32px sprites 縮放到 45px 格 |
| `SCALE_FACTOR` | 2.8125f | 45/16 |
| `TICK_INTERVAL` | 0.02f (50 FPS) | 每幀時間 |
| `WINDOW_WIDTH` | 1280 | 視窗寬度 |
| `WINDOW_HEIGHT` | 720 | 視窗高度 |
| `GRAVITY` | 13.7953f | 重力常數（與 JUMP_VELOCITY 對稱，確保拋物線對稱） |
| `JUMP_VELOCITY` | 13.7953f | 跳躍初速 |
| `JUMP_HIGH_VELOCITY` | 27.59f | 長按跳躍初速 |
| `JUMP_LOW_VELOCITY` | 8.4375f | 短按跳躍初速 |
| `BASE_SPEED` | 7.35f | 基礎移速（tiles/sec） |
| `SCALED_SPEED` | BASE_SPEED × TILE_SIZE × TICK_INTERVAL ≈ 6.615f | 每幀像素速度 |
| `RUN_MULTIPLIER` | 1.25f | 奔跑加速係數 |
| `INTERSECT_STRICTNESS` | 0.75f | 牆壁碰撞嚴格度 |
| `HITBOX_WIDTH_RATIO` | 0.6875f | Mario 碰撞體寬度比例 |
| `INITIAL_LIVES` | 3 | 初始生命數 |
| `INITIAL_TIME` | 400 | 初始計時 |
| `Z_BACKGROUND` | -10.0f | 背景層（山丘/草叢） |
| `Z_BLOCK` | -5.0f | 實體方塊層 |
| `Z_ENTITY` | 1.0f | 一般實體層 |
| `Z_PLAYER` | 2.0f | 玩家層 |
| `Z_EFFECT` | 10.0f | 特效層（粒子等） |
| `Z_UI` | 90.0f | UI 最頂層 |

#### 2.4.1 座標轉換 Helpers（`GameConfig` 靜態函數）

| 函數 | 公式 | 用途 |
|------|------|------|
| `WorldToPTSDX(worldX, camOffset)` | `worldX - camOffset - WINDOW_WIDTH/2` | 世界 X → PTSD 螢幕 X |
| `WorldToPTSDY(worldY)` | `WINDOW_HEIGHT/2 - worldY - TILE_SIZE/2` | 世界 Y → PTSD 螢幕 Y |
| `ScreenXToPTSD(screenX)` | `screenX - WINDOW_WIDTH/2` | 螢幕 X → PTSD X |
| `ScreenYToPTSD(screenY)` | `WINDOW_HEIGHT/2 - screenY` | 螢幕 Y → PTSD Y |

### 2.5 Python 工具腳本

| 腳本 | 用途 |
|------|------|
| `generate_8-4_map.py` | 從 NES layout 生成 8-4.csv（392x15 迷宮+Boss房） |
| `generate_sprites.py` | 批次裁切 Sprite sheet |
| `extract_8-4_sprites.py` | 提取 8-4 專用 sprites |
| `analyze_8-4_ids.py` | 分析 8-4.csv 所有 ID 出現次數 |
| `update_8-4_textures.py` | 更新 8-4 方塊紋理映射 |
| `generate_idlist_8-4.py` | 生成 IDList.csv 的 8-4 偏移區段 |

---

## 3. 設計模式深度解析

### 3.1 State Pattern — App::State 狀態機

**原問題：** 原版 `App.cpp` 在單一 switch-case 中塞入所有遊戲狀態邏輯，超過 500 行難以維護。
**解法：** GoF State Pattern。

```
Context:    App  (持有 unique_ptr<ISceneHandler>)
Interface:  ISceneHandler  (Update + OnRender + OnEnter + OnExit + GetName)
Concrete:   10 個 Handler 子類別（每個狀態獨立一個 .cpp）
Transition: App::TransitionTo(State) -> OnExit -> CreateSceneHandler() -> OnEnter
```

`App::Update()` 永遠只有兩行：

```cpp
m_CurrentHandler->Update(*this);    // game logic
m_CurrentHandler->OnRender(*this);  // drawing
```

**新增遊戲狀態只需：**

1. 新增一個 ISceneHandler 子類 (.hpp + .cpp)
2. 一個 `CreateSceneHandler()` case
3. 一個 `App::State` enum 值
**零修改 App.hpp 其他部分。**

---

### 3.2 Strategy Pattern — IEntityBehavior

**原問題：** C# Entity.cs 使用大量 `if (type == Goomba)` 判斷，難以擴展。
**解法：** Strategy Pattern — Entity 持有 `unique_ptr<IEntityBehavior>`，多型 dispatch。

| EntityType | Behavior 類 | 對應敵人 | 特性 |
|-----------|------------|---------|------|
| GOOMBA | EnemyBehavior | 栗寶寶 | 巡邏、踩死 |
| KOOPA_TROOPA | KoopaBehavior (TROOPA) | 烏龜兵 | 巡邏->Shell |
| KOOPA_SHELL | KoopaBehavior (SHELL) | 龜殼 | 靜止或反彈 |
| PARAKOOPA | ParaKoopaBehavior | 飛翔烏龜 | 正弦波浮動->著陸 |
| AXE_KOOPA | AxeKoopaBehavior | 斧頭烏龜 | 巡邏+定期拋斧 |
| BOWSER | BowserBehavior | Boss 庫巴 | 5-Phase AI + HP |
| FIRE | FireballBehavior | 玩家火球 | 拋物線軌跡 |
| MUSHROOM/STAR/FIRE_FLOWER/ONE_UP | ItemBehavior | 道具 | 彈跳+收集 |
| AXE | AxeBehavior | 橋頭斧 | 觸發橋塌序列 |
| PRINCESS | PrincessBehavior | 公主 NPC | 靜態顯示 |
| PIRANHA_PLANT | PiranhaPlantBehavior | 水管食人花 | 4-Phase 伸縮 |
| PODOBOO | PodobooBehavior | 熔岩泡泡 | 跳躍+不可殺 |
| COIN/FLAG/UNKNOWN | DefaultEntityBehavior | 被動實體 | 顯示/被動 |
| (brick break) | ParticleDebris | 磚塊碎片 | 物理粒子 |

OCP 原則：新增怪物 = 新增 XxxBehavior + EntityFactory 一個 case，**不修改任何現有類別**。

---

### 3.3 MVC Pattern — Player & Entity

```
Model      -> PlayerState / EntityState  (純資料：位置/速度/動畫key/狀態旗標)
View       -> Player      / Entity       (繼承 Util::GameObject：選 Sprite/渲染)
Controller -> InputHandler               (讀鍵盤 -> 寫 PlayerState)
           + PlayingSceneHandler         (主迴圈協調所有元件)
```

關鍵分離原則：

- `PlayerState` / `EntityState` 不依賴任何 PTSD 渲染 API
- `Player` / `Entity` 不包含遊戲邏輯，只根據 Model 選擇 Sprite
- 碰撞解析由 `CollisionManager` 處理，不放在 View 層

---

### 3.4 Factory Pattern — EntityFactory

唯一的 Entity 建立路徑（SRP 原則）：

```cpp
EntityFactory::SpawnFromLevel(entityDef, x, y, dir, fromBlock, levelName)
  -> new Entity(def, x, y, ...)
  -> switch(entityType) -> make_unique<XxxBehavior>()
  -> entity.SetBehavior(behavior)
  -> return shared_ptr<Entity>
```

---

### 3.5 Dependency Inversion — IAudioService

`AudioManager` 繼承 `IAudioService`。場景 Handler 只依賴抽象介面，方便單元測試替換為 MockAudio。
`ServiceLocator` 以類型安全的 `RegisterService<T>` / `GetService<T>` 模板 API 進一步解耦服務的提供者與消費者。

---

### 3.6 Service Locator — ServiceLocator

`ServiceLocator` 是全域單例，提供集中式服務注冊與查找，補充 DIP 的依賴注入：

```cpp
ServiceLocator::GetInstance().RegisterService<IAudioService>(audioMgr);
auto audio = ServiceLocator::GetInstance().GetService<IAudioService>();
```

無需傳遞指標即可在任何子系統存取共享服務，同時保持對介面的依賴而非具體實作。

---

### 3.7 Publish/Subscribe — EventSystem\<T\>

泛型事件系統，提供鬆耦合的組件間通信：

```cpp
EventSystem<PlayerDeadEvent> events;
int id = events.Subscribe([](const PlayerDeadEvent& e){ /* ... */ });
events.Publish(PlayerDeadEvent{ .cause = DeathCause::PIT });
events.Unsubscribe(id);
```

目前作為架構擴充預留鉤子，未來可取代部分 `TransitionTo()` 硬編碼呼叫。

---

## 4. Game Loop — 17 Phase 架構

`PlayingSceneHandler::Update(App&)` 每幀依序執行：

```
PHASE  0: ESC CHECK         — ESC -> 切換到 ESC_MENU
PHASE  1: PROCESS INPUT     — InputHandler::HandleInput(PlayerState, speed)
PHASE  2: UPDATE PHYSICS    — PlayerState::ApplyGravity() -> velY += gravity
PHASE  3: APPLY POSITION    — state.SetX/Y += velX/velY (velocity integration)
PHASE  4: COLLISION DETECT  — CollisionManager::CheckPlayerBlockCollision()
                               PIPELINE (matches C# Form1.cs onTick exactly):
                                 Step 1: FallDetect — 4px strip below feet; no block → SetGrounded(false)
                                 Step 2: Ceiling trigger (narrow hitbox) — head bump → snap + TriggerBlockHit
                                 Step 3: Per-block loop (full-body rect):
                                           Airborne  → DOWN→RIGHT→LEFT→DOWN→UP→LEFT
                                           Grounded  → RIGHT or LEFT only
PHASE  5: SPAWN ITEMS       — 處理被 block-hit 觸發的 Level::SpawnPoint
PHASE  6: PLAYER STATE TICK — PlayerState::Tick(); fire state fires fireball
PHASE  7: ENTITY AI UPDATE  — behavior->Update() for each active entity
                               entity block-collision per entity
                               ConsumeSpawnRequest() (Bowser/AxeKoopa spawn projectiles)
PHASE  8: ENTITY TICK+VIEW  — EntityState::Tick(); entity->UpdateView()
PHASE  9: PLAYER-ENTITY COL — CollisionManager::CheckPlayerEntityCollision()
PHASE 10: ENTITY-ENTITY COL — CollisionManager::CheckEntityEntityCollision()
PHASE 11: AXE/FLAG/PIPE     — CheckAxeCollision()
                               CheckFlagpoleCollision() (+ X-only jump-over fallback)
                               CheckPipeCollision()
PHASE 12: CAMERA + BLOCKS   — Camera::Update(); Level::UpdateBlocks()
PHASE 13: BRICK DEBRIS      — SpawnBrickDebris() for all JustBroken() blocks
                               MUST be after PHASE 4 so JustBroken() is not consumed early
PHASE 14: PLAYER VIEW       — Player::UpdateView(cameraOffset)
                               invincibility blink: Util::GameObject::SetVisible() ONLY
                               (NOT Player::SetVisible — that corrupts m_Visible)
PHASE 15: GAME TIMER        — GameStateManager::Tick(); time low -> hurry-up BGM switch
PHASE 16: PIT-FALL + DEATH  — CheckPitFall() -> TransitionTo(DEATH)
PHASE 17: CLEANUP           — CleanupDeadEntities() (erase deleted from m_Entities)
```

重要原則：

- Physics (PHASE 2-3) 在 Collision (PHASE 4) 之前 — 確保位置更新後才做碰撞解析
- Entity AI (PHASE 7) 在 Physics 之後 — AI 計算時看到的是本幀已更新的 Player 位置
- BrickDebris spawn (PHASE 13) 在 Ceiling collision (PHASE 4) 之後 — `JustBroken()` 旗標不被提前消費

### 特殊機制實作備註

| 機制 | 實作位置 | 關鍵細節 |
|------|---------|--------|
| 移動平台載人 | `PlayingSceneHandler.cpp` | 每幀讀 `plat->GetLastDeltaX/Y()`；Y gap < 2px 且 X overlap 即同步 Mario 座標 |
| 無敵星星殺敵 | `CollisionManager.cpp` | `ps.GetStarTimer() > 0` 時直接刪敵、計分、顯示浮動文字 |
| 連續踩踏分數 | `CollisionManager.cpp` | `m_StompCombo`；落地重置；分數序列 100→200→400→800→1000 |
| 食人花安全半徑 | `PiranhaPlantBehavior.cpp` | Mario 進入 `MARIO_SAFE_RADIUS = 45px` 時植物立即開始縮回 |
| 磚塊粒子初速 | `ParticleDebris.cpp` | 左上(-3,-6)、右上(+3,-6)、左下(-3,-4)、右下(+3,-4)；後續由 PhysicsEngine 累積重力 |

---

## 5. App::State 狀態機轉移圖

```
START -> TITLE --(RETURN)--> LOADING --(timer)--> PLAYING
                                                    |
          ESC_MENU <--(ESC)------------------------+
          ESC_MENU --(Resume)--> PLAYING
          ESC_MENU --(Quit)--> TITLE

PLAYING --(touch Goal Block 1-1)---> FLAGPOLE -> LOADING (next level)
PLAYING --(stand on pipe + DOWN, 1-2)-> PIPE_WARP -> LOADING (next level)
PLAYING --(touch Axe, 8-4)----------> AXE_SEQUENCE -> GAME_WON
PLAYING --(pit fall / enemy / time up)-> DEATH
  DEATH --(lives > 0)--> LOADING (retry same level)
  DEATH --(lives == 0)--> GAME_OVER --(RETURN)--> TITLE
GAME_WON --(RETURN)--> TITLE -> NewGame()
```

**Level sequence** (`GameStateManager::m_LevelSequence`):

```
"1-1" (ground) -> "1-2" (underground) -> "8-4" (castle + Boss) -> IsGameWon() = true
```

---

## 6. Refactoring 進度總覽

| Phase | 狀態 | 主要內容 |
|-------|------|---------|
| PHASE 1 | ✅ DONE | App.cpp 解耦；State Pattern 骨架建立 |
| PHASE 2 | ✅ DONE | 架構文件；ISceneHandler 11 個子類 |
| PHASE 3 | ✅ DONE | Runtime crash 修復；CollisionManager 獨立 |
| PHASE 4 | ✅ DONE | 旗杆/水管/死亡/GameOver 序列 |
| PHASE 5 | ✅ DONE | 計時器警告 UI；FloatingText 淡出；ESC 選單 |
| PHASE 6 | ✅ DONE | Boss 戰 5-Phase AI；Game Won 狀態 |
| PHASE 7 | ✅ DONE | 全部 13 個 IEntityBehavior 實作 |
| PHASE 8 | ✅ DONE | ParaKoopaBehavior；8-4 地圖重新生成 |
| PHASE 9 | ✅ DONE | AudioManager 整合；BGM/SFX 全面測試 |
| FINAL | ✅ DONE | 1-1 → 1-2 → 8-4 完整流程驗證 |
| BUG SESSION 1 | ✅ DONE | Bug #1–9：碰撞/粒子/旗幟/水管/Bowser 修復 |
| COLLISION REWRITE | ✅ DONE | CheckPlayerBlockCollision 全面 C# 移植；FallDetect full-body AABB；per-block 迴圈 |
| PIPE FIX | ✅ DONE | CheckPipeCollision: full-body AABB +1px；下管/右管條件 C# 精確翻譯 (Bug #10) |
| BUG SESSION 2 | ✅ DONE | Bug #11–14：Sticky Wall/邊緣 / 渲染縫隙 / 出生點視覺 / 城堡材質 |
| BUG SESSION 3 | ✅ DONE | Bug #15–18：旗杆序列 / 載入畫面 / 8-4 鏡頭鎖屏 / PiranhaPlant |
| BUG SESSION 4 | ✅ DONE | Bug #19–22：死亡動畫策略 / 蹲下碰撞 / 右管傳送 / FPS 顯示 |
| BUG SESSION 5 | ✅ DONE | Bug #23–26：效能優化 Viewport Culling / 座標系統統一 / 退化音效 / 8-4 通關清理 |
| BUG SESSION 6 | ✅ DONE | Bug #27–29：8-4 通關重力 / UnderCoin 金幣 / 金幣 Z-index |
| ARCHITECTURE+ | ✅ DONE | EventSystem\<T\> / CollisionContext DTO / AudioType 枚舉獨立 / ServiceLocator 加入 |
| OOP REFACTOR | ✅ DONE | `AlwaysUpdate()` + `OnSpawned()` 消除 string-find hack + dynamic_cast；`ConsumeSpawnRequest(EntityType&)` 型別安全；`GetGoalBlocks()` 快取；`QueryBlocksInRange(out)` 零分配；`AddEntityToGame` 統一實體生命週期 |
| CROUCH FIX | ✅ DONE | `CollisionManager` grounded 路徑：VelX==0（蹲下）時仍由中心比較推出重疊；修復 `SetCrouching(true)` posY shift 造成卡在方塊的問題 |

---

## Agent.md 開發原則遵守確認

| 原則 | 實現方式 | 狀態 |
|------|---------|------|
| 所有實體繼承 Util::GameObject | Player, Entity, Block, UIImage, UIText 全部繼承 | ✅ DONE |
| 沒有 God Class | App 只持有子系統 + TransitionTo()；邏輯分散到各 Handler/Manager | ✅ DONE |
| MVC 架構 | PlayerState(M) ← Player(V) ← InputHandler(C) | ✅ DONE |
| State Pattern | 11 個 ISceneHandler 子類；App::Update() 只有兩行 | ✅ DONE |
| Strategy Pattern | 13 個 IEntityBehavior + 4 個 IEnemyDeathAnimation + 1 個 IPlayerDeathAnimation | ✅ DONE |
| Factory Pattern | EntityFactory 唯一入口；EnemyDeathStyleFactory 策略選擇；符合 SRP | ✅ DONE |
| DIP | IAudioService 介面；AudioManager 實作；ServiceLocator 輔助注入 | ✅ DONE |
| OCP 原則 | 新增怪物/狀態不修改現有類別 | ✅ DONE |
| DRY 原則 | GameConfig 統一座標轉換 helpers；靜態 Sprite Cache；unordered_map 路徑表 | ✅ DONE |
| 不修改 CMakeLists.txt | 所有新增透過 files.cmake | ✅ DONE |
| 代碼注釋全英文 | 所有 .hpp/.cpp 注釋均為英文 | ✅ DONE |
