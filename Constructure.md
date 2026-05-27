# Super Mario Bros. — PTSD C++ OOP 架構設計 (Constructure)

> **Last synced:** 2026-05-25
> **關卡:** 1-1 (Ground) → 1-2 (Underground) → 8-4 (Castle + Boss)

本專案將 C# 版本的 God Class (`Form1.cs`) 徹底解耦，轉換為符合現代 C++ 標準的  
**深度物件導向架構 (Deep OOP Architecture)**。  
設計上大量運用**繼承 (Inheritance)**、**多型 (Polymorphism)**、**介面 (Interfaces)**  
與六大**設計模式 (Design Patterns)**：State、Strategy、MVC、Factory、DIP、Service Locator。

---

## 目錄

1. [完整 UML 繼承圖](#1-完整-uml-繼承圖)
   - 1.1 PTSD GameObject 繼承樹
   - 1.2 ISceneHandler 繼承樹 (State Pattern)
   - 1.3 IEntityBehavior 繼承樹 (Strategy Pattern)
   - 1.4 死亡動畫策略繼承樹
   - 1.5 ICollisionHandler 繼承樹 (Strategy + Facade)
   - 1.6 IAudioService 繼承樹 (DIP)
   - 1.7 IUIPanel 繼承樹 (Strategy Pattern)
   - 1.8 App 全域架構圖
   - 1.9 MVC 完整關係圖
   - 1.10 ServiceLocator & EventSystem & ILevelService
   - 1.11 IInputHandler 繼承樹 (DIP)
   - 1.12 IPlayerForm 繼承樹 (State Pattern)
2. [所有檔案清單](#2-所有檔案清單)
   - 2.1 Include Headers
   - 2.2 Source Files
   - 2.3 Resources
   - 2.4 GameConfig 關鍵常數
   - 2.5 Python 工具腳本
3. [設計模式深度解析](#3-設計模式深度解析)
   - 3.1 State Pattern — App::State 狀態機
   - 3.2 Strategy Pattern — IEntityBehavior
   - 3.3 State Pattern — IPlayerForm 力量型態
   - 3.4 DIP — IAudioService & AudioManager
   - 3.5 Strategy — IUIPanel & UIManager
   - 3.6 Service Locator — ServiceLocator 服務定位器
   - 3.7 Publish/Subscribe — EventSystem\<T\>
   - 3.8 Strategy + Facade — 碰撞解析子系統
   - 3.9 Strategy — 敵人死亡動畫策略
   - 3.10 Strategy — 道具多型策略
   - 3.11 DIP / Service Locator — 關卡管理服務 (LevelManager)
   - 3.12 State Pattern — 零向下轉型狀態傳參 (Zero Down-Casting State Handshake)
   - 3.13 MVC 完整運作序列圖
4. [Game Loop — 17 Phase 架構](#4-game-loop--17-phase-架構)
5. [App::State 狀態機轉移圖](#5-appstate-狀態機轉移圖)
6. [Refactoring 進度總覽](#6-refactoring-進度總覽)
7. [OOP 原則遵守確認](#7-oop-原則遵守確認)

---

## 1. 完整 UML 繼承圖

### 1.1 PTSD GameObject 繼承樹

```mermaid
classDiagram
    direction TB

    class GameObject {
        <<PTSD Framework>>
        +SetDrawable(drawable)
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
        #HandleOnHit(playerState)*
        +Bounce() / Break()
        +GetAABB() AABB
        +IsSolid() / IsBreakable() / IsGoal()
        +JustBroken() bool
        +ShouldResolveVerticallyFirst(playerBody) bool*
        +TryCarryPlayer(ps, prevAABB, onStaticBlock)*
    }

    class MovingPlatform {
        <<Block>>
        -Direction m_Dir
        -float m_Velocity, m_MinBound, m_MaxBound
        -float m_LastDeltaX, m_LastDeltaY
        +StepMovement()
        +GetLastDeltaX() float
        +GetLastDeltaY() float
        +ShouldResolveVerticallyFirst(playerBody) bool
        +TryCarryPlayer(ps, prevAABB, onStaticBlock)
    }

    class StoneBlock      { <<Block>> #HandleOnHit(playerState) }
    class BrickBlock      { <<Block>> #HandleOnHit(playerState) }
    class QuestionBlock   { <<Block>> #HandleOnHit(playerState) }
    class InvisibleBlock  { <<Block>> #HandleOnHit(playerState) }
    class GoalBlock       { <<Block>> #HandleOnHit(playerState) }
    class BackgroundBlock { <<Block>> #HandleOnHit(playerState) }
    class BridgeBlock     { <<Block>> #HandleOnHit(playerState) }

    class Entity {
        <<Util::GameObject>>
        -EntityDef m_Def
        -EntityState m_State
        -unique_ptr~IEntityBehavior~ m_Behavior
        +UpdateView(cameraOffset)
        +SetBehavior(behavior)
        +GetBehavior() IEntityBehavior*
        +GetState() EntityState&
        +GetHitbox() AABB
    }

    class Player {
        <<Util::GameObject>>
        -PlayerState m_State
        -bool m_Visible
        +UpdateView(cameraOffset)
        +GetState() PlayerState&
        +SetVisible(bool)
        note: sprite anchor = hitbox-bottom so crouch\nsprite never sinks below floor
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
    Block <|-- StoneBlock
    Block <|-- BrickBlock
    Block <|-- QuestionBlock
    Block <|-- InvisibleBlock
    Block <|-- GoalBlock
    Block <|-- BackgroundBlock
    Block <|-- BridgeBlock
    GameObject <|-- Entity
    GameObject <|-- Player
    GameObject <|-- UIImage
    GameObject <|-- UIText
```

---

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

    class TitleSceneHandler     { <<ISceneHandler>> }
    class LoadingSceneHandler   { <<ISceneHandler>> show WORLD X-X + LIVES }
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
    class FlagpoleSceneHandler  { <<ISceneHandler>> }
    class PipeWarpSceneHandler  { <<ISceneHandler>> }
    class AxeSequenceSceneHandler { <<ISceneHandler>> 8-4 boss-defeat cutscene }
    class DeathSceneHandler     { <<ISceneHandler>> }
    class GameOverSceneHandler  { <<ISceneHandler>> }
    class GameWonSceneHandler   { <<ISceneHandler>> }
    class ESCMenuSceneHandler {
        <<ISceneHandler>>
        5-item pause menu
        RESUME / 1-1 / 1-2 / 8-4 / POWER
        +ForceApplyPowerState(idx)
    }

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

> `TitleSceneHandler`, `DeathSceneHandler`, `GameOverSceneHandler`, `GameWonSceneHandler` は `MenuSceneHandlers.hpp/.cpp` に合併。

---

### 1.3 IEntityBehavior 繼承樹 (Strategy Pattern)

```mermaid
classDiagram
    direction TB

    class IEntityBehavior {
        <<interface>>
        +Update(state, level, player, timer)*
        +OnPlayerCollision(state, player, isFromAbove) bool*
        +Clone() unique_ptr~IEntityBehavior~*
        +GetName() const char**
        +AlwaysUpdate() bool
        +OnSpawned(vx, vy)
        +OnFireballHit(state) bool
        +ConsumeSpawnRequest(type, x, y, dir) bool
        +IsImmuneToStomp() bool
        +IsEnemyProjectile() bool
        +IsImmuneToStarPower() bool
        +IsPlayerFireball() bool
        +IsShell() bool
        +ExplodesOnWall() bool
        +GetVisualYOffset(levelName) float
        +GetHitbox(state) AABB
    }

    class GoombaBehavior        { Goomba patrol + squish + Smart AI }
    class KoopaBehavior         { KoopaTroopa OR Shell -m_Type:KoopaType }
    class ParaKoopaBehavior     { Flying Koopa -m_FloatPhase:float }
    class AxeKoopaBehavior      { Axe-throwing Koopa +ConsumeSpawnRequest() }
    class BowserBehavior {
        5-Phase Boss AI + HP
        -m_Phase:BowserPhase
        -m_HealthPoints:int
        -m_PendingSpawns:vector~SpawnRequest~
        +AlwaysUpdate() true
        +IsImmuneToStomp() true
        +ConsumeSpawnRequest()
        -ResolveWallAndEdge(state, level)
    }
    class FireballBehavior      { Parabolic / horizontal +AlwaysUpdate() true }
    class MushroomBehavior      { Mushroom power-up }
    class FireFlowerBehavior    { Fire Flower power-up }
    class StarBehavior          { Star bounce movement }
    class OneUpBehavior         { Green mushroom 1UP }
    class CoinBehavior          { Collectible coin }
    class AxeBehavior           { Bridge axe kill-trigger (8-4) }
    class PrincessBehavior      { NPC goal marker (8-4) }
    class AxeProjectileBehavior { Thrown axe projectile (8-4) }
    class PiranhaPlantBehavior  { Pipe plant 4-Phase cycle -m_Phase }
    class PodobooBehavior       { Lava bubble — immortal; +IsImmuneToStomp() true }
    class DefaultEntityBehavior { Passive coin / flag }
    class ParticleDebris {
        Brick debris particles
        -m_InitialVelX, m_InitialVelY:float
        -m_LifetimeFrames:int
        +AlwaysUpdate() true
        +OnSpawned(vx, vy)
    }
    class CastleFireSpawnerBehavior {
        8-4 off-screen fire spawner
        -m_PendingSpawns:vector~SpawnRequest~
        +AlwaysUpdate() true
    }

    IEntityBehavior <|.. GoombaBehavior
    IEntityBehavior <|.. KoopaBehavior
    IEntityBehavior <|.. ParaKoopaBehavior
    IEntityBehavior <|.. AxeKoopaBehavior
    IEntityBehavior <|.. BowserBehavior
    IEntityBehavior <|.. FireballBehavior
    IEntityBehavior <|.. MushroomBehavior
    IEntityBehavior <|.. FireFlowerBehavior
    IEntityBehavior <|.. StarBehavior
    IEntityBehavior <|.. OneUpBehavior
    IEntityBehavior <|.. CoinBehavior
    IEntityBehavior <|.. AxeBehavior
    IEntityBehavior <|.. PrincessBehavior
    IEntityBehavior <|.. AxeProjectileBehavior
    IEntityBehavior <|.. PiranhaPlantBehavior
    IEntityBehavior <|.. PodobooBehavior
    IEntityBehavior <|.. DefaultEntityBehavior
    IEntityBehavior <|.. ParticleDebris
    IEntityBehavior <|.. CastleFireSpawnerBehavior
```

> `KoopaBehavior`, `ParaKoopaBehavior`, `AxeKoopaBehavior` → `KoopaFamily.hpp/.cpp`  
> `AxeBehavior`, `PrincessBehavior`, `AxeProjectileBehavior` → `StaticEntityBehaviors.hpp/.cpp`

---

### 1.4 死亡動畫策略繼承樹

```mermaid
classDiagram
    direction LR

    class IEnemyDeathAnimation {
        <<interface>>
        +Start(cause, runtime)*
        +Tick(runtime, gravity, tickInterval)*
        +IsActive() bool*
    }
    class GoombaSquishDeathAnimation  { stomp → squash hold → delete }
    class KoopaRetreatDeathAnimation  { stomp → shell ; fire/shell/star → flip die }
    class FireballFlipDeathAnimation  { flip arc then despawn }
    class ClassicEnemyDeathAnimation  { generic fallback }

    IEnemyDeathAnimation <|.. GoombaSquishDeathAnimation
    IEnemyDeathAnimation <|.. KoopaRetreatDeathAnimation
    IEnemyDeathAnimation <|.. FireballFlipDeathAnimation
    IEnemyDeathAnimation <|.. ClassicEnemyDeathAnimation

    class IPlayerDeathAnimation {
        <<interface>>
        +Start()*
        +Tick(gravity, tickInterval, jumpVelocity, playerY)*
        +IsActive() bool*
    }
    class ClassicPlayerDeathAnimation {
        freeze 12 frames → launch → fall
        -m_Active, m_Launched:bool
        -m_FrameCounter:int
        -m_VelY:double
    }

    IPlayerDeathAnimation <|.. ClassicPlayerDeathAnimation
```

---

### 1.5 ICollisionHandler 繼承樹 (Strategy + Facade)

```mermaid
classDiagram
    direction TB

    class ICollisionHandler { <<abstract>> }

    class BlockContactResolver {
        <<static utility>>
        +BodyRect(state)$ AABB
        +ResolveDown(state, bb, movingDown)$
        +ResolveUp(state, bb, movingUp)$
        +ResolveRight(state, bb, movingRight)$
        +ResolveLeft(state, bb, movingLeft)$
        +IsPlayerOnStaticBlock(state, level)$ bool
    }

    class PlayerBlockHandler {
        <<ICollisionHandler>>
        Step 1 FallDetect
        Step 2 CeilingTrigger
        Step 3 BodyResolution (C# order)
        +Resolve(player, level, camera, gameState, ui, spawns)
        -ProcessSingleBlock(state, blk, movingDown, movingUp, ...)
    }

    class PlayerEntityHandler {
        <<ICollisionHandler>>
        -m_StompCombo:int
        NES combo ×1×2×4×8→1000
        Star power instant-kill
        Shell kick / side damage
        +Resolve(player, entities, camera, gameState, ui)
    }

    class EntityBlockHandler {
        <<ICollisionHandler>>
        Ground snap / Wall flip
        Fireball → Explosion on wall
        Pit deactivation
        +Resolve(entity, level, outNewEntities)
    }

    class EntityEntityHandler {
        <<ICollisionHandler>>
        Fireball vs Enemy → kill
        Moving Shell vs Enemy → kill
        Camera culling
        +Resolve(entities, gameState, cameraOffset)
    }

    class CollisionManager {
        <<Facade>>
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

### 1.6 IAudioService 繼承樹 (DIP)

```mermaid
classDiagram
    direction LR

    class IAudioService {
        <<interface>>
        +PlayBGM(BGMName)*
        +PlaySFX(SFXName)*
        +StopBGM()*
        +PlayBGMForLevel(levelName, timeRemaining)*
    }

    class AudioManager {
        <<IAudioService + Singleton>>
        -m_BGMCache:map
        -m_SFXCache:map
        +GetInstance()$ AudioManager&
        +PlayBGM() / PlaySFX() / StopBGM()
        +PlayBGMForLevel(levelName, time)
    }

    class AudioPathResolver {
        <<static helper>>
        +GetBGMPath(name)$
        +GetSFXPath(name)$
    }

    class BGMName { <<enum>> 21 values }
    class SFXName { <<enum>> 20 values }

    IAudioService <|.. AudioManager
    AudioManager --> AudioPathResolver : resolves paths
    AudioManager --> BGMName : uses
    AudioManager --> SFXName : uses
```

> `IAudioService.hpp`, `AudioPathResolver.hpp`, `AudioType.hpp` 它以單獨文件的形式存在。
> `AudioManager.hpp` 包含 `IAudioService` 定義並實作；`AudioPathResolver` 是純靜態輔助類別，提供從枚舉到路徑的映射。

---

### 1.7 IUIPanel 繼承樹 (Strategy Pattern)

每個遊戲畫面的 UI 小部件由對應的 `IUIPanel` 子類別封裝。
`UIManager` 僅持有 `unordered_map<State, IUIPanel*>` 並在 `Update()` 中查表分派，
不包含任何 `switch-case`。新增畫面：新增子類別 + 在 UIManager ctor 中 `Register()`，
**不需修改** `UIManager::Update()`（OCP）。

```mermaid
classDiagram
    class IUIPanel {
        <<interface>>
        +Register(renderer) void
        +Show() void
        +Hide() void
        +Refresh(gs) void
    }
    class HUDPanel {
        +Refresh(gs) void
        -m_HeaderMario UIText
        -m_HeaderWorld UIText
        -m_HeaderTime UIText
        -m_ScoreText UIText
        -m_WorldText UIText
        -m_TimeText UIText
        -m_CoinUI CoinUI
        -m_FlashCounter int
    }
    class TitlePanel {
        +Refresh(gs) void
        -m_TitleLabel UIText
        -m_SubLabel UIText
    }
    class LoadingPanel {
        +Refresh(gs) void
        -m_WorldLabel UIText
        -m_LivesText UIText
        -m_MarioPreview UIImage
    }
    class SimpleTextPanel {
        +Refresh(gs) void
        -m_TitleText string
        -m_TitleLabel UIText
        -m_ScoreText UIText
    }
    class ESCMenuPanel {
        +SetMenuContext(sel, power) void
        +Refresh(gs) void
        -m_Selection int
        -m_PausedLabel UIText
        -m_MenuTexts vector
    }
    class AxeEndingPanel {
        +SetShowCredits(bool) void
        +Refresh(gs) void
        -m_ShowCredits bool
        -m_Line1 UIText
        -m_Line2 UIText
    }
    IUIPanel <|.. HUDPanel
    IUIPanel <|.. TitlePanel
    IUIPanel <|.. LoadingPanel
    IUIPanel <|.. SimpleTextPanel
    IUIPanel <|.. ESCMenuPanel
    IUIPanel <|.. AxeEndingPanel
```

> `SimpleTextPanel` 被 `UIManager` 實例化兩次：`m_GameOverPanel("GAME OVER")` 與
> `m_GameWonPanel("WORLD CLEARED")`，展示 OOP 複用而不重複代碼。
> `HUDPanel` 在所有 `PLAYING` / `ESC_MENU` 狀態疊加顯示，不在 `m_PanelMap` 中。

---

### 1.8 App 全域架構圖

```mermaid
classDiagram
    direction LR

    class App {
        -m_CurrentState:State
        -m_CurrentHandler:unique_ptr~ISceneHandler~
        -m_Camera:Camera
        -m_Renderer:Renderer
        -m_LevelService:shared_ptr~ILevelService~
        -m_InputHandler:unique_ptr~IInputHandler~
        -m_CollisionManager:CollisionManager
        -m_GameState:GameStateManager
        -m_UIManager:unique_ptr~UIManager~
        +TransitionTo(State)
        +LoadLevel(name)
        +AddEntityToGame(entity)
        +IsUnderground() bool
        +ApplyBackground()
        +ApplyBackground(bool)
        +PlayCurrentBGM()
    }

    class ILevelService {
        <<interface>>
        +LoadLevel(app, name)*
        +StartLevel(app)*
        +PlayCurrentBGM(app)*
        +AddEntityToGame(app, entity)*
        +IsUnderground(app) bool*
        +ApplyBackground(app)*
        +ApplyBackground(app, bool)*
    }

    class LevelManager {
        -m_Level:shared_ptr~Level~
        -m_Player:shared_ptr~Player~
        -m_Entities:vector~shared_ptr~Entity~~
        -m_FlagEntity:shared_ptr~Entity~
    }

    ILevelService <|-- LevelManager
    App --> ILevelService : owns via shared_ptr (DIP)
    App --> Camera
    App --> CollisionManager
    App --> GameStateManager
    App --> IInputHandler       : owns via unique_ptr (DIP)
    App --> UIManager
    App --> EntityFactory      : uses static
    App --> PhysicsEngine      : uses static
    App --> AudioManager       : uses singleton
    App --> ServiceLocator     : registers IAudioService & ILevelService
    EntityFactory --> EnemyDeathStyleFactory : selects death strategy
    ServiceLocator --> IAudioService : holds as shared_ptr
    ServiceLocator --> ILevelService : holds as shared_ptr
```

---

### 1.8 MVC 完整關係圖

```mermaid
classDiagram
    direction TB

    class PlayerState {
        <<Model>>
        -m_PosX, m_PosY, m_VelX:float
        -m_VelY, m_FallHeight:double
        -m_PowerState, m_MemoryState:PowerState
        -m_InvTimer, m_AnimFrame:int
        -m_TransitionTimer:int
        -m_PrevPowerState:PowerState
        -m_DeathAnimation:unique_ptr~IPlayerDeathAnimation~
        +Init() / Tick()
        +ApplyGravity() float
        +GetAABB() AABB
        +BuildAnimationKey() string
        +TakeDamage() / IsInvincible()
        +ForceApplyPowerState(idx)
        +IsBigOrFire() bool
        +CanShootFire() bool
        +GetTransitionTimer() int
        +GetPrevPowerState() PowerState
    }

    class Player {
        <<View — Util::GameObject>>
        -m_State:PlayerState
        -m_Visible:bool
        +UpdateView(cameraOffset)
        +SetVisible(bool)
    }

    class IInputHandler {
        <<interface — Controller>>
        +HandleInput(PlayerState, speed, level)*
        +IsJumpPressed() bool*
    }

    class InputHandler {
        <<IInputHandler — Keyboard>>
        Space/Z/UP/W = Jump
        Right/D Left/A Down/S
        E/LShift = Run/Fire
        +HandleInput(PlayerState, speed, level)
    }

    IInputHandler <|.. InputHandler

    class EntityState {
        <<Model>>
        -m_PosX, m_PosY, m_VelX:float
        -m_VelY, m_FallHeight:double
        -m_Active, m_IsEnemy, m_Deleted:bool
        -m_AnimFrame, m_ScoreWorth:int
        -m_DeathAnimation:unique_ptr~IEnemyDeathAnimation~
        +Init() / Tick()
        +GetAABB() AABB
        +SetDeleted(bool) / IsDeleted()
    }

    class Entity {
        <<View — Util::GameObject>>
        -m_Def:EntityDef
        -m_State:EntityState
        -m_Behavior:unique_ptr~IEntityBehavior~
        +UpdateView(cameraOffset)
        +GetState() EntityState&
        +GetBehavior() IEntityBehavior*
    }

    InputHandler --> PlayerState : writes
    IInputHandler --> PlayerState : writes (via impl)
    Player --> PlayerState : owns
    PlayerState --> IPlayerDeathAnimation : owns (strategy)

    Entity --> EntityState : owns
    EntityState --> IEnemyDeathAnimation : owns (strategy)
    Entity --> IEntityBehavior : owns (polymorphic dispatch)
```

---

### 1.9 ServiceLocator & EventSystem

```mermaid
classDiagram
    direction TB

    class ServiceLocator {
        <<Singleton>>
        -m_Services:map~string, shared_ptr~
        +GetInstance()$ ServiceLocator&
        +RegisterService~T~(service)
        +GetService~T~() shared_ptr~T~
        +HasService~T~() bool
    }

    class EventSystem~T~ {
        <<template>>
        -m_Listeners:map~int, EventListener~
        -m_NextId:int
        +Subscribe(listener) int
        +Unsubscribe(id)
        +Publish(event)
        +Clear()
    }

    class CollisionContext {
        <<DTO>>
        +player:shared_ptr~Player~
        +level:shared_ptr~Level~
        +entityFactory:EntityFactory*
        +gameState:GameStateManager*
        +gameTimer:int
        +invTimer:int
    }

    ServiceLocator --> IAudioService : holds
    IAudioService <|.. AudioManager
```

---

### 1.10 IInputHandler 繼承樹 (DIP)

```mermaid
classDiagram
    direction LR

    class IInputHandler {
        <<interface>>
        +HandleInput(state, speed, level)*
        +IsMovingRight() bool*
        +IsMovingLeft() bool*
        +IsJumpPressed() bool*
        +IsCrouchPressed() bool*
        +IsRunPressed() bool*
    }

    class InputHandler {
        <<IInputHandler — Keyboard>>
        -m_Right, m_Left, m_Jump:bool
        -m_Crouch, m_Run:bool
        Bindings: Arrow/WASD + Space/Z/UP/W + E/LShift
        +HandleInput(state, speed, level)
    }

    IInputHandler <|.. InputHandler
```

> `IInputHandler` follows the same DIP pattern as `IAudioService`:
> `App.hpp` depends only on the abstraction; the concrete `InputHandler` is
> injected in `App::Start()`. Future extensions (gamepad, AI input, replay)
> only require a new `IInputHandler` implementation — zero changes to App or
> PlayingSceneHandler.

---

### 1.11 IPlayerForm 繼承樹 (State Pattern)

為了將 Mario 的各種力量狀態 (Small, Big, Fire, Star) 與實體行為解耦，我們導入了狀態模式 (State Pattern)。
`PlayerState` 不再以臃腫的 switch-cases 或條件判斷處理狀態行為，而是持有多型的 `IPlayerForm` 策略指標。

```mermaid
classDiagram
    direction TB

    class IPlayerForm {
        <<interface>>
        +GetPowerState() PowerState*
        +GetHeight(bool crouching) int*
        +IsBigOrFire() bool*
        +CanShootFire() bool*
        +IsStar() bool*
        +GetSpriteState(bool isFireShooting) int*
        +GetStarState(int starTimer, bool isFireShooting) int*
        +GetFormName() string*
        +TakeDamage() unique_ptr~IPlayerForm~*
        +PowerUp(PowerState) unique_ptr~IPlayerForm~*
    }

    class SmallPlayerForm     { <<IPlayerForm>> }
    class BigPlayerForm       { <<IPlayerForm>> }
    class FirePlayerForm      { <<IPlayerForm>> }
    class SmallStarPlayerForm { <<IPlayerForm>> }
    class BigStarPlayerForm   { <<IPlayerForm>> }

    IPlayerForm <|.. SmallPlayerForm
    IPlayerForm <|.. BigPlayerForm
    IPlayerForm <|.. FirePlayerForm
    IPlayerForm <|.. SmallStarPlayerForm
    IPlayerForm <|.. BigStarPlayerForm
```

> `IPlayerForm` 透過多型虛擬函式封裝了各個型態的特定維度、攻擊判定以及受傷、升級時的型態轉換邏輯。
> 新增任何 Mario 型態 (如狸貓、冰花) 僅需新增對應的子類別，而無須修改任何現有的 state 判斷程式碼，完美遵循開閉原則 (OCP)。

---

## 2. 所有檔案清單

### 2.1 Include Headers (`include/`)

| 檔案 | 類別 / 結構 | @inheritance | 職責 |
|------|------------|-------------|------|
| `App.hpp` | `App` | None | 持有子系統；State 切換；存取器 API；`IsUnderground()` 合併 GameStateManager + Level 的地下判斷；`ApplyBackground()` 無參數重載 |
| `Mario/Core/GameConfig.hpp` | `GameConfig` | None (static consts) | 全域常數 + 座標轉換靜態 helpers |
| `Mario/Core/Collider.hpp` | `AABB` | None (data struct) | AABB 矩形 + Intersects() (strict inequality) |
| `Mario/Core/CollisionContext.hpp` | `CollisionContext` | None (data struct) | 碰撞解析資料傳遞物件 (DTO)，攜帶 Player/Level/EntityFactory/GameStateManager 參考 |
| `Mario/Core/Camera.hpp` | `Camera` | None | 橫向捲動 offset；8-4 Boss 鎖屏；world to screen 轉換 |
| `Mario/Core/PhysicsEngine.hpp` | `PhysicsEngine` | None (static) | ApplyGravity() + GetJumpHeight() |
| `Mario/Core/SpritePathResolver.hpp` | `SpritePathResolver` | None (static) | Sprite 路徑解析 Block/Player/Entity（具有 s_ResolvedPathCache 解決每幀磁碟 I/O） |
| `Mario/Level/EntityDef.hpp` | `EntityDef`, `BlockDef`, `EntityType` | None (data) | CSV 資料結構；EntityType 列舉；`renderTargetWidth` 渲染縮放欄位（EntityFactory 設定，OCP） |
| `Mario/Level/Block.hpp` | `Block`, `StoneBlock`, `BrickBlock`, `QuestionBlock`, `InvisibleBlock`, `GoalBlock`, `BackgroundBlock`, `BridgeBlock` | `Util::GameObject` | 磚塊基類及 7 種多型子類別；處理 hit 彈跳/破碎/物品生成邏輯，以及自訂 visibility / bridge 屬性與 debris 粒子名稱產生 |
| `Mario/Level/MovingPlatform.hpp` | `MovingPlatform` | `Block` | 移動平台（1-2 垂直 / 8-4 水平）；override `ShouldResolveVerticallyFirst` 優先垂直 Snap；`TryCarryPlayer` 攜帶玩家 |
| `Mario/Level/Level.hpp` | `Level` | None | CSV 解析；高階 O(1) 扁平 Block 陣列；視口剔除與 visibility culling；SpawnPoint；`GetGoalBlocks()` 快取；`QueryBlocksInRange(out)` 零分配版；`IsUnderground()` 名稱判斷 |
| `Mario/Level/EntityState.hpp` | `EntityState` | None (Model) | Entity MVC Model：位置/速度/動畫/死亡策略 |
| `Mario/Level/EnemyDeathAnimation.hpp` | `IEnemyDeathAnimation`, `GoombaSquishDeathAnimation`, `KoopaRetreatDeathAnimation`, `FireballFlipDeathAnimation`, `ClassicEnemyDeathAnimation`, `EnemyDeathRuntime`, `EnemyDeathCause` | `IEnemyDeathAnimation <- {GoombaSquish, KoopaRetreat, FireballFlip, Classic}` | 敵人死亡動畫多策略（踩踏/龜殼/火球/通用） |
| `Mario/Level/EnemyDeathStyleFactory.hpp` | `EnemyDeathStyleFactory` | None (Factory) | 依 EntityType 注入對應敵人死亡策略 |
| `Mario/Level/Entity.hpp` | `Entity` | `Util::GameObject` | Entity View：渲染 + Strategy 行為；Z-index 自決策 |
| `Mario/Level/EntityFactory.hpp` | `EntityFactory` | None (Factory) | 唯一 Entity 建立入口；`SpawnProjectile()` 負責 **entity-spawned** 投射物（Bowser/AxeKoopa）；`SpawnFromPlayer()` 負責 **player-fired** 投射物，兩者均透過 `MakeProjectileDef()` 構建 def（SRP/OCP；PlayingSceneHandler 零 inline EntityDef） |
| `Mario/Player/PlayerState.hpp` | `PlayerState`, `PowerState` | None (Model) | Player MVC Model：物理/狀態/動畫key/死亡策略 |
| `Mario/Player/PlayerForm.hpp` | `IPlayerForm`, `SmallPlayerForm`, `BigPlayerForm`, `FirePlayerForm`, `SmallStarPlayerForm`, `BigStarPlayerForm` | `IPlayerForm <- {SmallPlayerForm, BigPlayerForm, FirePlayerForm, SmallStarPlayerForm, BigStarPlayerForm}` | 力量狀態多型策略（State Pattern）：尺寸、受傷退化、升級轉換 |
| `Mario/Player/PlayerDeathAnimation.hpp` | `IPlayerDeathAnimation`, `ClassicPlayerDeathAnimation` | `IPlayerDeathAnimation <- ClassicPlayerDeathAnimation` | 玩家死亡動畫策略（凍結/起跳/下墜） |
| `Mario/Player/Player.hpp` | `Player` | `Util::GameObject` | Player View：渲染 + m_Visible 守衛 |
| `Mario/Services/IInputHandler.hpp` | `IInputHandler` | None (interface) | Input abstraction (DIP); `HandleInput` + query accessors; enables keyboard, gamepad, AI/replay swap |
| `Mario/Services/InputHandler.hpp` | `InputHandler` | `IInputHandler -> InputHandler` | Keyboard controller: Arrow/WASD + Space/Z/UP/W jump + E/LShift fire |
| `Mario/CollisionManager.hpp` | `CollisionManager` | None (facade) | Collision 子系統協調者（Facade Pattern）；公開 API 不變；邏輯委派給 Collision/ 四個 Handler |
| `Mario/Collision/ICollisionHandler.hpp` | `ICollisionHandler` | None (abstract interface) | 所有碰撞 Handler 的標記基類；定義繼承樹頂點 |
| `Mario/Collision/BlockContactResolver.hpp` | `BlockContactResolver` | None (static utility) | 靜態 Down/Up/Right/Left AABB Snap helpers；BodyRect() 建立全寬碰撞體（C# GetRecPosition 等效）；`IsPlayerOnStaticBlock()` 判定玩家是否站在靜態方塊上；ResolveDown/Up 配合調優之 INTERSECT_STRICTNESS (0.35f) 實現精確地面與天花板邊緣判定，徹底修復邊緣飄浮 Bug 並保留流暢跳躍落腳物理 |
| `Mario/Collision/PlayerBlockHandler.hpp` | `PlayerBlockHandler` | `ICollisionHandler` | 玩家-方塊三步驟管線：FallDetect → CeilingTrigger → BodyResolution；ProcessSingleBlock 取代原 lambda |
| `Mario/Collision/PlayerEntityHandler.hpp` | `PlayerEntityHandler` | `ICollisionHandler` | 玩家-實體碰撞：踩踏 NES Combo / 傷害 / 道具收集；m_StompCombo 在此管理 |
| `Mario/Collision/EntityBlockHandler.hpp` | `EntityBlockHandler` | `ICollisionHandler` | 實體-方塊碰撞：地面 Snap / 牆壁翻向 / Fireball→Explosion / 落坑刪除 |
| `Mario/Collision/EntityEntityHandler.hpp` | `EntityEntityHandler` | `ICollisionHandler` | 實體-實體碰撞：火球 vs 敵人 / 移動龜殼 vs 敵人；viewport culling |
| `Mario/Level/GameStateManager.hpp` | `GameStateManager` | None (Service) | 分數/生命/金幣/時間/關卡進度 |
| `Mario/Scenes/ISceneHandler.hpp` | `ISceneHandler` | None (interface) | State Pattern 純虛介面（10 個實作） |
| `Mario/Scenes/MenuSceneHandlers.hpp` | `TitleSceneHandler`, `DeathSceneHandler`, `GameOverSceneHandler`, `GameWonSceneHandler` | `ISceneHandler` | 選單/死亡/結束場景（合併） |
| `Mario/Scenes/LoadingSceneHandler.hpp` | `LoadingSceneHandler` | `ISceneHandler` | 加載畫面（顯示 WORLD X-X + LIVES） |
| `Mario/Scenes/PlayingSceneHandler.hpp` | `PlayingSceneHandler` | `ISceneHandler` | 主遊戲迴圈（17-phase）；`m_WasStarActive` 無敵星星 BGM 轉場追蹤；`m_DebrisQueryBuffer` 零分配磚塊查詢緩衝 |
| `Mario/Scenes/FlagpoleSceneHandler.hpp` | `FlagpoleSceneHandler` | `ISceneHandler` | 旗杆滑動序列 |
| `Mario/Scenes/PipeWarpSceneHandler.hpp` | `PipeWarpSceneHandler` | `ISceneHandler` | 水管傳送過場 |
| `Mario/Scenes/AxeSequenceSceneHandler.hpp` | `AxeSequenceSceneHandler` | `ISceneHandler` | 8-4 Bowser 擊敗序列 |
| `Mario/Scenes/ESCMenuSceneHandler.hpp` | `ESCMenuSceneHandler` | `ISceneHandler` | ESC 暫停選單；5 項選單（RESUME/1-1/1-2/8-4/**POWER**）；`OnEnter()` 從玩家當前 PowerState 初始化 `m_PowerStateIndex`；`GetPowerStateName(idx)` 靜態輔助；ENTER 鍵觸發 `ForceApplyPowerState()` |
| `Mario/Services/AudioType.hpp` | `BGMName` (21), `SFXName` (20) | None (enum header) | BGM / SFX 音效枚舉定義 |
| `Mario/Services/IAudioService.hpp` | `IAudioService` | None (interface) | 音效抽象介面（DIP）；`PlayBGM` / `PlaySFX` / `StopBGM` / `PlayBGMForLevel` |
| `Mario/Services/AudioPathResolver.hpp` | `AudioPathResolver` | None (static utility) | RESOURCE_DIR 路徑解析；`GetBGMPath(filename)` / `GetSFXPath(filename)` |
| `Mario/Services/AudioManager.hpp` | `AudioManager` | `IAudioService <- AudioManager` (Singleton) | 音效全系統實作；內部 BGM/SFX cache；`PlayBGMForLevel(levelName, time)` 集中 level→BGM 映射 |
| `Mario/Services/ILevelService.hpp` | `ILevelService` | None (interface) | 關卡服務抽象介面（DIP）；關卡加載、角色/實體/背景渲染管理與狀態控制 |
| `Mario/Services/LevelManager.hpp` | `LevelManager` | `ILevelService <- LevelManager` | 關卡服務具體實作；封裝載入、啟動、背景設置、BGM 播放與遊戲世界實體集合的管理 |
| `Mario/Services/ServiceLocator.hpp` | `ServiceLocator` | None (Service Locator) | 服務定位器 Singleton；type-safe `RegisterService<T>` / `GetService<T>` |
| `Mario/Services/EventSystem.hpp` | `EventSystem<T>` | None (template) | 泛型 Pub/Sub 事件系統；`Subscribe` / `Unsubscribe` / `Publish` |
| `Mario/UI/UIPanel.hpp` | `IUIPanel` | None (interface) | Strategy 介面：`Register(renderer)` / `Show()` / `Hide()` / `Refresh(gs)`；新增畫面只需新增子類別，不需修改 `UIManager::Update()` (OCP) |
| `Mario/UI/UIPanel.hpp` | `HUDPanel` | `IUIPanel` | 遊戲中 HUD（分數/世界/時間/金幣）；封裝 `CoinUI`；時間 &lt; 100 閃爍動畫 |
| `Mario/UI/UIPanel.hpp` | `TitlePanel` | `IUIPanel` | 標題畫面："SUPER MARIO BROS" + "PRESS ENTER TO START" |
| `Mario/UI/UIPanel.hpp` | `LoadingPanel` | `IUIPanel` | 載入畫面："WORLD X-X" + 命數 + `MarioIdle.png` 預覽（建構子預載貼圖避免首幀空白） |
| `Mario/UI/UIPanel.hpp` | `SimpleTextPanel` | `IUIPanel` | 可重用文字面板；以標題字串為建構參數；`GameOverPanel` ("GAME OVER") 與 `GameWonPanel` ("WORLD CLEARED") 各建一實例 |
| `Mario/UI/UIPanel.hpp` | `ESCMenuPanel` | `IUIPanel` | 暫停選單；`SetMenuContext(int sel, string power)` 在 `Refresh()` 前注入選取索引與 POWER 名稱 |
| `Mario/UI/UIPanel.hpp` | `AxeEndingPanel` | `IUIPanel` | 8-4 結局字幕；`SetShowCredits(bool)` 控制 "THANK YOU MARIO!" 顯示 |
| `Mario/UI/UIManager.hpp` | `UIManager` | None | **薄型分派器**：持有所有 Panel 實體 + `unordered_map<State, IUIPanel*>`；`Update()` 呼叫 `HideAllScenePanels()` 後查表顯示當前 Panel；HUD 在所有遊戲進行狀態疊加顯示；`State` enum 保留供 scene handler 呼叫（不修改外部 API） |
| `Mario/UI/UIWidgets.hpp` | `UIImage`, `UIText` | `Util::GameObject <- UIImage/UIText` | 輕量 UI 元件（合併） |
| `Mario/UI/FloatingText.hpp` | `FloatingText` | None | 漂浮分數文字（60 幀淡出） |
| `Mario/UI/CoinUI.hpp` | `CoinUI` | None (composite) | 金幣動畫圖示 + 計數文字 |
| `Mario/Behaviors/IEntityBehavior.hpp` | `IEntityBehavior` | None (interface) | Strategy Pattern 純虛介面（19 個實作）；新增 GetHitbox、IsImmuneToStomp、IsEnemyProjectile、IsImmuneToStarPower、IsPlayerFireball、IsShell、ExplodesOnWall 與 GetVisualYOffset 多型以解耦所有硬編碼與自動視覺底端對齊調整 |
| `Mario/Behaviors/GoombaBehavior.hpp` | `GoombaBehavior` | `IEntityBehavior` | Goomba Smart AI (Dodge Hop, Ledge-avoidance, Proximity pursue) |
| `Mario/Behaviors/KoopaFamily.hpp` | `KoopaBehavior`, `ParaKoopaBehavior`, `AxeKoopaBehavior` | `IEntityBehavior` | Koopa 系列 AI（合併）；`AxeKoopaBehavior` 使用 ConsumeSpawnRequest 生成斧頭（pending-flag 模式） |
| `Mario/Behaviors/BowserBehavior.hpp` | `BowserBehavior` | `IEntityBehavior` | Boss 5-Phase AI + HP 系統；使用 `vector<SpawnRequest>` 佇列支援同時丟斧頭與吐火球；支援 AlwaysUpdate 以實現開局起持續越屏噴火 |
| `Mario/Behaviors/CastleFireSpawnerBehavior.hpp` | `CastleFireSpawnerBehavior` | `IEntityBehavior` | 8-4 關卡專用隱形越屏噴火器 |
| `Mario/Behaviors/FireballBehavior.hpp` | `FireballBehavior` | `IEntityBehavior` | 拋物線/水平火球；支援 AlwaysUpdate，並多型實作 IsEnemyProjectile 和 IsImmuneToStomp 以支援敵方火球越屏平移與傷害解耦 |
| `Mario/Behaviors/ItemBehaviors.hpp` | `MushroomBehavior`, `FireFlowerBehavior`, `StarBehavior`, `OneUpBehavior`, `CoinBehavior` | `IEntityBehavior` | Polymorphic power-up and collectible item strategies |
| `Mario/Behaviors/StaticEntityBehaviors.hpp` | `AxeBehavior`, `PrincessBehavior`, `AxeProjectileBehavior` | `IEntityBehavior` | 8-4 靜態觸發器/NPC/投擲斧頭投影行為（合併） |
| `Mario/Behaviors/PiranhaPlantBehavior.hpp` | `PiranhaPlantBehavior` | `IEntityBehavior` | 水管食人花 4-Phase；管口安全半徑 1.5×TILE |
| `Mario/Behaviors/PodobooBehavior.hpp` | `PodobooBehavior` | `IEntityBehavior` | 熔岩泡泡（不可擊殺） |
| `Mario/Behaviors/DefaultEntityBehavior.hpp` | `DefaultEntityBehavior` | `IEntityBehavior` | 被動實體（金幣/旗幟） |
| `Mario/Behaviors/ParticleDebris.hpp` | `ParticleDebris` | `IEntityBehavior` | 磚塊破碎粒子 |

**Note:** `GameTheater.hpp`、`SceneManager.hpp` 及其 `.cpp` 是已被 State Pattern 取代的孤兒檔案，已從磁碟**永久刪除**。

### 2.2 Source Files (`src/`)

| 檔案 | 行數 (約) | 備註 |
|------|---------|------|
| `App.cpp` | 130 | TransitionTo + delegation to ILevelService + accessor impls；移除 Z-index 覆寫 (Bug #18) |
| `Mario/Core/Camera.cpp` | 52 | 8-4 Boss 鎖屏邏輯 (Bug #17) |
| `Mario/Core/PhysicsEngine.cpp` | 47 | |
| `Mario/Core/SpritePathResolver.cpp` | 428 | 全 unordered_map 靜態表與 s_ResolvedPathCache 解決每幀磁碟 I/O；補齊城堡/出生點 mappings (Bug #13, #14) |
| `Mario/Level/Block.cpp` | 436 | 靜態 s_BlockSpriteCache；像素對齊渲染；實作 7 個子類別 HandleOnHit 多型與 OnHit Template Method (2026-05-27) |
| `Mario/Level/MovingPlatform.cpp` | 114 | WorldToScreen 使用統一轉換 helper (Bug #24) |
| `Mario/Level/Level.cpp` | 547 | CSV 解析與 O(1) 扁平 2D 陣列 Block 索引；UpdateBlocks 視口 column visibility culling 降低 CPU 迴圈；快取橋樑方塊與城堡門位置以達到 O(1) 更新/檢查效能 (2026-05-25) |
| `Mario/Player/PlayerState.cpp` | 354 | 死亡策略；蹲下高度動態調整 (Bug #20)；階梯式退化 (Bug #25)；`ForceApplyPowerState(idx)` 作弊器；`IsBigOrFire()` DRY helper (2026-05-24)；定義非內聯解構子 `~PlayerState()` 以解決前置宣告不完整類型編譯錯誤 (2026-05-25)；新增 shape-changing 視覺 transition freeze 機制 (2026-05-27) |
| `Mario/Player/PlayerForm.cpp` | 303 | IPlayerForm 及 5 種力量型態子類別多型行為實作 |
| `Mario/Player/PlayerDeathAnimation.cpp` | 35 | ClassicPlayerDeathAnimation 策略實作 |
| `Mario/Player/Player.cpp` | 166 | 死亡精靈鎖定 (Bug #19)；閃爍 PTSD 基類直呼叫 (Bug #5)；像素對齊 (Bug #12)；**crouch sprite anchored to hitbox bottom, fixes floor sinking** (2026-05-24)；定義非內聯虛擬解構子 `~Player()` 以解決前置宣告不完整類型編譯錯誤 (2026-05-25)；精緻化 transition 大小狀態交替渲染與 Y 座標接地校正 (2026-05-27) |
| `Mario/Services/InputHandler.cpp` | 122 | 全寬 uncrouch guard (Bug #20) |
| `Mario/Level/EntityState.cpp` | 206 | 死亡策略整合；簡化 GetHitbox 為預設 AABB 運算，消除硬編碼字串 (2026-05-25) |
| `Mario/Level/EnemyDeathAnimation.cpp` | 162 | 四種死亡策略實作 |
| `Mario/Level/EnemyDeathStyleFactory.cpp` | 30 | 依 EntityType 選策略 |
| `Mario/Level/Entity.cpp` | 221 | 靜態 s_EntitySpriteCache；Z-index 自決策（PiranhaPlant, COIN）；實作 GetHitbox 碰撞體委派邏輯 (2026-05-25) |
| `Mario/Level/EntityFactory.cpp` | 418 | AXE->AxeBehavior (Bug #8)；COIN/STAR/FIRE_FLOWER/ONE_UP ItemType 精確注入 (Bug #28)；**`MakeProjectileDef()` centralises all projectile EntityDef construction** (2026-05-24) |
| `Mario/CollisionManager.cpp` | 65 | **Facade only** — 5 個 CheckXxx 方法各自委派給對應 Handler；全部邏輯已移至 Collision/ 子系統 |
| `Mario/Collision/BlockContactResolver.cpp` | 114 | 靜態 Down/Up/Right/Left 解析方法；BodyRect helper（原 file-scope static 函數） |
| `Mario/Collision/PlayerBlockHandler.cpp` | 315 | 三步驟管線 + ProcessSingleBlock（取代原 lambda）+ TriggerBlockHit（原私有方法）|
| `Mario/Collision/PlayerEntityHandler.cpp` | 290 | HandleEnemyCollision + HandleItemCollision；m_StompCombo NES Combo 計數；自訂行為碰撞體支援 (2026-05-25) |
| `Mario/Collision/EntityBlockHandler.cpp` | 136 | CheckGround + CheckWalls（Fireball→Explosion spawn）；自訂行為碰撞體支援 (2026-05-25) |
| `Mario/Collision/EntityEntityHandler.cpp` | 109 | Fireball vs Enemy + Moving Shell vs Enemy；IsMovingShell() static helper；使用 thread_local 暫存快取 pre-filtering 篩選可見實體，將碰撞迴圈由 O(N^2) 降低至 O(M^2) (2026-05-25) |
| `Mario/Level/GameStateManager.cpp` | 104 | |
| `Mario/Scenes/MenuSceneHandlers.cpp` | 128 | 死亡場景接管生命扣除與動畫 (Bug #19)；GameWon 黑底 (Bug #26) |
| `Mario/Scenes/LoadingSceneHandler.cpp` | 47 | 強制黑色背景 (Bug #16) |
| `Mario/Scenes/PlayingSceneHandler.cpp` | 595 | X-only 旗杆 fallback (Bug #4)；pipe 展開 box (Bug #21)；viewport entity culling；無敵星星 Star BGM 動態轉場；重構結局/傳送邏輯委派給各自 Scene (2026-05-25) |
| `Mario/Scenes/FlagpoleSceneHandler.cpp` | 190 | 旗杆滑動與城堡進入過場邏輯；整合原 LevelCompleteController 邏輯，並以 O(1) 檢查城堡門位置；移除 13.0f 地面硬編碼，改用 Dynamic Player-Block 碰撞檢測以動態貼齊任何地形與高度 (2026-05-25) |
| `Mario/Scenes/PipeWarpSceneHandler.cpp` | 143 | 水管傳送過場邏輯；整合原 LevelCompleteController 邏輯 (2026-05-25) |
| `Mario/Scenes/AxeSequenceSceneHandler.cpp` | 166 | 8-4 Bowser 擊敗與橋樑坍塌動畫過場邏輯；整合原 LevelCompleteController 邏輯 (2026-05-25) |
| `Mario/Scenes/ESCMenuSceneHandler.cpp` | 129 | 5-item menu logic; case 4 cycles power cheat + calls ForceApplyPowerState() |
| `Mario/UI/UIManager.cpp` | 514 | InitLoadingScreen 預載 (Bug #16)；FPS+版權文字 (Bug #22)；座標 helper (Bug #24) |
| `Mario/Services/AudioManager.cpp` | 238 | AudioPathResolver 實作 |
| `Mario/Services/AudioPathResolver.cpp` | 8 | BGM/SFX 路徑映射 |
| `Mario/Services/LevelManager.cpp` | 180 | 關卡服務實作：LoadLevel, StartLevel, BGM 播放, 背景設置 |
| `Mario/UI/CoinUI.cpp` | 85 | |
| `Mario/UI/FloatingText.cpp` | 44 | |
| `Mario/Behaviors/GoombaBehavior.cpp` | 91 | Smart AI implementation |
| `Mario/Behaviors/KoopaFamily.cpp` | 241 | |
| `Mario/Behaviors/BowserBehavior.cpp` | 364 | 丟斧頭（定時/快速）與吐火球 AI 佇列生成；HP 系統 |
| `Mario/Behaviors/CastleFireSpawnerBehavior.cpp` | 81 | 隱形定時向左發射火球 |
| `Mario/Behaviors/FireballBehavior.cpp` | 86 | |
| `Mario/Behaviors/ItemBehaviors.cpp` | 134 | Implementations of specialized power-up and collectible behaviors |
| `Mario/Behaviors/StaticEntityBehaviors.cpp` | 65 | |
| `Mario/Behaviors/PiranhaPlantBehavior.cpp` | 159 | 居中修正 + 安全半徑 (Bug #18)；實作 GetHitbox 回傳精確緊湊碰撞體 (2026-05-25) |
| `Mario/Behaviors/PodobooBehavior.cpp` | 107 | |
| `Mario/Behaviors/DefaultEntityBehavior.cpp` | 51 | |
| `Mario/Behaviors/ParticleDebris.cpp` | 52 | |

**Total: 48 source files, 8628 lines of C++17 OOP code**

### 2.3 Resources

| 路徑 | 內容 |
|------|------|
| `Resources/Levels/1-1.csv` | 地面關卡（16x220 格） |
| `Resources/Levels/1-2.csv` | 地下關卡（16x220 格） |
| `Resources/Levels/8-4.csv` | 城堡關卡（15x392 格）— generate_8-4_map.py 生成；ID 961 (MovingPlatformH) 放置於 row=12 cols=70,163,176,189（水平移動平台，±4 tiles 範圍） |
| `Resources/LookUpSheet/IDList.csv` | Block 定義表 ID to name/solid/breakable/...；ID 893 與 904 均為 `Lava`（solid=0, background=1），可穿越不碰撞，Mario 掉入後落出畫面觸發 pit-fall 死亡 |
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
| `INTERSECT_STRICTNESS` | 0.35f | 牆壁碰撞與落腳判定嚴格度 (調優至 0.35f 以防邊緣飄浮 Bug，保留流暢落腳物理) |
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
| `WorldToPTSDX(worldX, camOffset)` | `worldX - camOffset - WINDOW_WIDTH/2` | 世界中心 X → PTSD 螢幕 X |
| `WorldToPTSDY(worldY)` | `WINDOW_HEIGHT/2 - worldY - TILE_SIZE/2` | 世界中心 Y → PTSD 螢幕 Y |
| `TopLeftToPTSDX(left, w, cam)` | `WorldToPTSDX(left + w*0.5, cam)` | 左邊 + 寬度 → PTSD X（統一入口，避免散落 +w/2）|
| `TopLeftToPTSDY(top, h)` | `WorldToPTSDY(top + h*0.5)` | 上邊 + 高度 → PTSD Y（同上）|
| `ScreenXToPTSD(screenX)` | `screenX - WINDOW_WIDTH/2` | 螢幕 X → PTSD X |
| `ScreenYToPTSD(screenY)` | `WINDOW_HEIGHT/2 - screenY` | 螢幕 Y → PTSD Y |

> **規範：** 所有渲染物件（Player, Entity, Block, MovingPlatform）必須透過
> `TopLeftToPTSDX/Y` 計算螢幕位置，禁止在 callsite 手動寫 `+width/2`。
> 唯一例外是 Player 的 Y 軸（蹲下 crouch 需特殊底部對齊邏輯）。

### 2.5 Python 工具腳本

| 腳本 | 用途 |
|------|------|
| `generate_8-4_map.py` | 從 NES layout 生成 8-4.csv（392×15 迷宮 + Boss 房） |
| `make_84_level.py` | 組合完整 8-4 關卡 CSV |
| `generate_sprites.py` | 批次裁切 Sprite sheet |
| `extract_8-4_sprites.py` | 提取 8-4 專用 sprites |
| `compose_8-4_enemy_sprites.py` | 合成 8-4 敵人 sprite 資源 |
| `copy_8-4_sprites.py` | 將裁好的 8-4 sprite 複製至 Resources |
| `analyze_8-4_ids.py` | 分析 8-4.csv 所有 Block ID 出現次數 |
| `check_csv.py` | 驗證 CSV 格式正確性 |
| `check_idlist.py` | 驗證 IDList.csv 所有 ID 定義完整 |
| `update_8-4_textures.py` | 更新 8-4 方塊紋理映射 |
| `update_idlist_8-4.py` | 同步更新 IDList.csv 的 8-4 區段 |
| `generate_idlist_8-4.py` | 生成 IDList.csv 的 8-4 偏移區段 |
| `find_lava_segments.py` | 定位 8-4 熔岩段落 ID |
| `mark_piranha_pipes.py` | 標記食人花水管位置 |
| `mark_podoboo_spawners.py` | 標記熔岩泡泡生成點 |

---

## 3. 設計模式深度解析

### 3.1 State Pattern — App::State 狀態機

**原問題：** 原版 `App.cpp` 在單一 switch-case 中塞入所有遊戲狀態邏輯，超過 500 行難以維護。
**解法：** GoF State Pattern。

```
Context:    App  (持有 unique_ptr<ISceneHandler>)
Interface:  ISceneHandler  (Update + OnRender + OnEnter + OnExit + GetName)
Concrete:   10 個 Handler 子類別（每個狀態獨立一個 .cpp）
Transition: App::TransitionTo(State) → OnExit → CreateSceneHandler() → OnEnter
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
| GOOMBA | GoombaBehavior | 栗寶寶 | Smart AI (Dodge Hop, Ledge-avoidance, Proximity pursue) |
| KOOPA_TROOPA | KoopaBehavior (TROOPA) | 烏龜兵 | 巡邏->Shell |
| KOOPA_SHELL | KoopaBehavior (SHELL) | 龜殼 | 靜止或反彈 |
| PARAKOOPA | ParaKoopaBehavior | 飛翔烏龜 | 正弦波浮動->著陸 |
| AXE_KOOPA | AxeKoopaBehavior | 斧頭烏龜 | 巡邏 + 定期拋斧 (ConsumeSpawnRequest) |
| BOWSER | BowserBehavior | Boss 庫巴 | 5-Phase AI + HP |
| (castle fire) | CastleFireSpawnerBehavior | 8-4 隱形噴火器 | AlwaysUpdate；越屏持續向左射出火球 |
| FIRE | FireballBehavior | 玩家火球 | 拋物線軌跡 |
| MUSHROOM | MushroomBehavior | 紅色香菇 | 成長為大瑪莉 |
| FIRE_FLOWER | FireFlowerBehavior | 火之花 | 成長為火瑪莉 |
| STAR | StarBehavior | 無敵星星 | 彈跳移動，無敵狀態 |
| ONE_UP | OneUpBehavior | 綠色香菇 | 增加生命計數 |
| COIN | CoinBehavior | 金幣 | 增加金幣與分數 |
| AXE | AxeBehavior | 橋頭斧 | 觸發橋塌序列 |
| PRINCESS | PrincessBehavior | 公主 NPC | 靜態顯示 |
| PIRANHA_PLANT | PiranhaPlantBehavior | 水管食人花 | 4-Phase 伸縮 |
| PODOBOO | PodobooBehavior | 熔岩泡泡 | 跳躍+不可殺 |
| FLAG/UNKNOWN | DefaultEntityBehavior | 被動實體 | 顯示/被動 |
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
// 一般 Entity 建立
EntityFactory::SpawnEntity(def, x, y, dir, fromBlock, levelName)
  // 1. 複製 def → 根據 EntityType / levelName 設定 renderTargetWidth
  // 2. new Entity(localDef, x, y, ...)  [Entity 不再持有 levelName 做縮放判斷]
  // 3. switch(entityType) -> make_unique<XxxBehavior>()
  // 4. entity.SetBehavior(behavior)
  // 5. return shared_ptr<Entity>

// 投射物 EntityDef 建立（取代 PlayingSceneHandler 80-line inline 段）
// 所有 Fireball / Axe 投射物的 EntityDef config 集中在此，符合 OCP：
// 新增投射物類型 = 只改這一個 switch，不動 PlayingSceneHandler
EntityFactory::MakeProjectileDef(spawnType, isEnemy, level)
  // → 查 EntityList.csv → 補 fallback def → 回傳完整設定的 EntityDef
  // 初速度 / Axe 拋物線計算仍在 PlayingSceneHandler（與玩家位置有關）
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

### 3.8 Strategy + Facade — 碰撞解析子系統

**原問題：** `CollisionManager.cpp` 曾是一個高達 800 行的巨大「義大利麵」類別，在單一檔案內混合處理了玩家-方塊、玩家-敵人、實體-方塊、實體-實體等 4 種截然不同的碰撞邏輯，內部充斥著大量的狀態標記 lambda 與高度耦合的私有輔助方法，完全違反了 SRP（單一職責原則）。

**解法：** 導入 **Facade（外觀模式）** 與 **Strategy（策略模式）**。
- `CollisionManager` 本身被重構為一個極其簡潔的 **Facade（外觀門面）**，它不包含任何具體的碰撞計算邏輯，而是將職責徹底委派給 4 個特化的策略處理器（均繼承自 `ICollisionHandler` 標記基類）。
- **四個特化處理器：**
  1. `PlayerBlockHandler`：處理玩家與方塊的碰撞（採用 C# 經典的三步驟物理 Snap 管線）。
  2. `PlayerEntityHandler`：處理玩家與各類敵方/道具實體的碰撞（如 Stomp Combo 踩踏、星星無敵殺敵、硬幣收集、變身升級）。
  3. `EntityBlockHandler`：處理所有敵人與場景方塊的碰撞（地面 Snap、面牆反向、落坑刪除）。
  4. `EntityEntityHandler`：處理實體與實體間的碰撞（玩家火球擊殺敵人、移動龜殼擊殺敵人），並透過 thread-local 快取與視口剔除將碰撞迴圈從 $O(N^2)$ 優化至 $O(M^2)$。

```mermaid
classDiagram
    direction TB
    class CollisionManager {
        <<Facade>>
        -m_PlayerBlockHandler:PlayerBlockHandler
        -m_PlayerEntityHandler:PlayerEntityHandler
        -m_EntityBlockHandler:EntityBlockHandler
        -m_EntityEntityHandler:EntityEntityHandler
        +CheckPlayerBlockCollision()
        +CheckPlayerEntityCollision()
        +CheckEntityBlockCollision()
        +CheckEntityEntityCollision()
    }
    class ICollisionHandler {
        <<interface>>
    }
    class PlayerBlockHandler { +Resolve() }
    class PlayerEntityHandler { +Resolve() }
    class EntityBlockHandler { +Resolve() }
    class EntityEntityHandler { +Resolve() }

    CollisionManager --> PlayerBlockHandler
    CollisionManager --> PlayerEntityHandler
    CollisionManager --> EntityBlockHandler
    CollisionManager --> EntityEntityHandler
    ICollisionHandler <|-- PlayerBlockHandler
    ICollisionHandler <|-- PlayerEntityHandler
    ICollisionHandler <|-- EntityBlockHandler
    ICollisionHandler <|-- EntityEntityHandler
```

---

### 3.9 Strategy — 敵人死亡動畫策略

**原問題：** 遊戲中敵人的死亡演出形式各異（栗寶寶被踩扁、烏龜被踩縮回殼中、火球擊中翻轉墜下、星星撞飛），原版實作在 `EntityState` 中以硬編碼的計時器與標記分支來控制，當需要加入新的死亡形式時，必須修改核心實體代碼，違反了 OCP（開閉原則）。

**解法：** 導入 **Strategy Pattern（策略模式）** 與 **Abstract Factory（抽象工廠）**。
- 定義抽象策略介面 `IEnemyDeathAnimation`，其包含 `Update()`、`GetSpriteKey()` 與 `IsFinished()`。
- 將不同的死亡演出封裝為獨立的策略類別：
  - `GoombaSquishDeathAnimation`：栗寶寶被踩扁的動畫（快速消失）。
  - `KoopaRetreatDeathAnimation`：烏龜被踩踏後縮入龜殼的動畫。
  - `FireballFlipDeathAnimation`：被火球或無敵星星擊中時翻轉下墜的動畫。
  - `ClassicEnemyDeathAnimation`：通用下墜動畫。
- 引入 **`EnemyDeathStyleFactory`**，負責根據 `EntityType` 與 `EnemyDeathCause` 動態建立並注入對應的死亡動畫策略。
- 實體死亡時，`EntityState` 僅需呼叫 `TriggerDeath(cause)`，隨後將每幀的動畫更新與精靈切換委派給當前的死亡策略，達成完美解耦。

---

### 3.10 Strategy — 道具多型策略

**原問題：** 遊戲中的各種道具（香菇、花、星星、1UP、硬幣）原先使用一個臃腫的 `ItemBehavior` 類別，在 `Update()` 內部使用 `if-else` 或 `switch` 判斷 `ItemType` 來實施不同的位移邏輯，造成代碼臃腫且極難擴展。

**解法：** 導入 **Strategy Pattern（策略模式）**。
- 將 `ItemBehavior` 徹底拆分為 5 個獨立且高度聚焦的策略類別，全部繼承自 `IEntityBehavior`：
  - `MushroomBehavior`：香菇行為（從方塊升起，向右平移，受重力下墜，碰牆反向）。
  - `FireFlowerBehavior`：火之花行為（靜態升起，等待玩家收集）。
  - `StarBehavior`：星星行為（落地時跳躍起伏，持續前進）。
  - `OneUpBehavior`：1UP 綠香菇行為（與紅香菇位移邏輯一致，但觸發增加生命）。
  - `CoinBehavior`：金幣行為（靜態旋轉，無重力或水平運動）。
- 移除所有道具 `ItemType` 的硬編碼分支。當需要加入新的道具類型（例如冰花、飛天貓）時，只需新增對應的策略類別，並在 `EntityFactory` 中進行註冊即可，現有的碰撞與行為引擎代碼**完全不需要修改任何一行**！

---

### 3.11 DIP / Service Locator — 關卡管理服務 (LevelManager)

**原問題：** 主控制器 `App` 曾是一個典型的 **God Class（上帝類別）**。它直接擁有了 `Player`、`Level`、`Entities`、計時器、以及 OpenGL 清理與背景渲染逻辑。這導致 `App` 本身行數飆升，且其他 Handler 或子系統必須大量依賴 `App` 的內部成員。

**解法：** 導入 **Dependency Inversion Principle (DIP)** 與 **Service Locator Pattern（服務定位器模式）**。
- **介面隔離**：我們建立了一個全新的抽象介面 `ILevelService`，聲明了關卡加載、啟動、BGM播放、背景設置等一切與關卡及遊戲世界狀態有關的 API。
- **具體實作**：實作 `LevelManager` 繼承自 `ILevelService`。將 `m_Level`、`m_Player`、`m_Entities`、`m_FlagEntity` 等所有數據成員從 `App` 中剝離，**徹底歸入 `LevelManager` 進行管理**。
- **服務定位**：在 `App::Start()` 時，我們實例化 `LevelManager`，並將其註冊至 `ServiceLocator` 中，供全域解耦使用。
- **向後相容**：為了避免修改數十個 Scene Handlers 的 `app.GetPlayer()` 調用，`App` 保留了原有的 getter 存取 API，但在底層直接委派給 `m_LevelService` 執行，這使得 `App.cpp` 大幅度減肥至僅有約 130 行，成為一個乾淨純粹的最高層級狀態機協調器！

---

### 3.12 State Pattern — 零向下轉型狀態傳參 (Zero Down-Casting State Handshake)

**原問題：** 在 `PlayingSceneHandler` 中，當觸發拉下旗桿切換至 `FLAGPOLE` 狀態，或走入水管切換至 `PIPE_WARP` 狀態時，目標狀態需要接收特定的座標和實體參數。原先的作法是在 `app.TransitionTo(...)` 之後，使用 `dynamic_cast<FlagpoleSceneHandler*>` 或 `dynamic_cast<PipeWarpSceneHandler*>` 將場景指針向下轉型，再呼叫 `Setup` 配置方法。這是一個嚴重的**向下轉型類型壞味道（Down-casting Smell）**，破壞了狀態機的封裝性。

**解法：** 導入 **Self-Configuring（狀態自適應）** 與 **State Context DTO（狀態傳遞數據物件）**。
1. **旗桿狀態自適應（Self-Configuring Flagpole）**：
   - 移除 `SetupFlagpole` 介面。讓 `FlagpoleSceneHandler` 在 `OnEnter()` 觸發時，主動向 `ILevelService` 查詢當前的 `GetFlagEntity()`，並自 Level 的 `GetGoalBlocks()` 快取中動態發現第一個目標方塊的 X 座標來完成自動定位。
2. **水管傳送上下文 DTO（Warp Context DTO）**：
   - 在 `GameStateManager` 中加入專用的輕量級 Warp 數據欄位（傳送方向、入口 X、入口 Y 座標）。
   - `PlayingSceneHandler` 在轉場前呼叫 `app.GetGameState().SetWarpInfo("Down", x, y)` 將傳送 context 存入全局的 GameState 中，然後直接呼叫 `app.TransitionTo(App::State::PIPE_WARP)`。
   - `PipeWarpSceneHandler` 在其 `OnEnter()` 生命週期勾子中，主動自 `GameState` 讀取參數並完成初始化。
- **成果：** 整個 C++ 專案中**完全清除了所有的 `dynamic_cast` 與向下轉型呼叫**，達成了 100% textbook-pure 的狀態模式多型切換！

---

### 3.13 MVC 完整運作序列圖

下圖呈現了本專案在每幀（Per Frame）遊戲主迴圈中，**Model (M) — View (V) — Controller (C)** 與各個解耦服務（LevelManager、InputHandler、CollisionManager、AudioManager）之間的時序交互關係。這保證了代碼各司其職，毫無義大利麵式的混亂耦合：

```mermaid
sequenceDiagram
    autonumber
    actor Player as Player Input
    participant App as App Context
    participant Handler as PlayingSceneHandler
    participant Service as LevelManager (ILevelService)
    participant Input as InputHandler (Controller)
    participant State as PlayerState (Model)
    participant Col as CollisionManager (Facade)
    participant View as Player (View)
    participant Sfx as AudioManager (Service)

    Player->>Input: Press Key (e.g. RIGHT / D)
    App->>Handler: Update(App&) [Per Frame Loop]
    Handler->>Input: HandleInput(State, speed, level)
    Input->>State: SetVelX(), SetMovingRight(true)
    Handler->>State: ApplyGravity() -> velY
    Handler->>Col: CheckPlayerBlockCollision()
    Col->>State: Resolve overlaps / SetGrounded()
    Handler->>Col: CheckPlayerEntityCollision()
    Col->>State: TakeDamage() or CollectItem()
    Col->>Sfx: PlaySFX()
    Handler->>State: Tick() [Update timers]
    Handler->>Service: GetEntities() -> Update behaviors
    App->>Handler: OnRender(App&) [Per Frame Render]
    Handler->>View: UpdateView(cameraOffset)
    View->>App: SetPosition() / SetSprite()
    Handler->>App: GetRenderer().Update() [OpenGL Clear + Draw]
```

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

```mermaid
stateDiagram-v2
    [*] --> START
    START --> TITLE : App::Start()
    TITLE --> LOADING : PRESS ENTER (Select World)
    LOADING --> PLAYING : LEVEL_TRANSITION_DELAY (3.0s) timer expires
    PLAYING --> ESC_MENU : PRESS ESC (Pause Game)
    ESC_MENU --> PLAYING : SELECT RESUME / PRESS ESC
    ESC_MENU --> TITLE : SELECT QUIT
    PLAYING --> FLAGPOLE : Collide with flagpole column (1-1 / 1-2)
    FLAGPOLE --> LOADING : Castle entering animation finishes
    PLAYING --> PIPE_WARP : Stand on Pipe + press DOWN or RIGHT (1-2)
    PIPE_WARP --> LOADING : Descend/rightwalk sequence completes
    PLAYING --> AXE_SEQUENCE : Collide with bridge Axe (8-4)
    AXE_SEQUENCE --> GAME_WON : Bowser defeat sequence completes
    PLAYING --> DEATH : Mario dies (damage, pit fall, time up)
    DEATH --> LOADING : Lives > 0 -> Retry same level
    DEATH --> GAME_OVER : Lives == 0
    GAME_OVER --> TITLE : PRESS ENTER
    GAME_WON --> TITLE : PRESS ENTER
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
| PHASE 2 | ✅ DONE | 架構文件；ISceneHandler 10 個子類 |
| PHASE 3 | ✅ DONE | Runtime crash 修復；CollisionManager 獨立 |
| PHASE 4 | ✅ DONE | 旗杆/水管/死亡/GameOver 序列 |
| PHASE 5 | ✅ DONE | 計時器警告 UI；FloatingText 淡出；ESC 選單 |
| PHASE 6 | ✅ DONE | Boss 戰 5-Phase AI；Game Won 狀態 |
| PHASE 7 | ✅ DONE | 全部 19 個 IEntityBehavior 實作 |
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
| OCP REFACTOR | ✅ DONE | `EntityDef::renderTargetWidth`：EntityFactory 設定縮放覆蓋，Entity.cpp 不再比較 level-name 字串；`EntityFactory::SpawnProjectile()` 取代 PlayingSceneHandler 80-line inline 投射物建立；`SpawnBrickDebris` 改用 `QueryBlocksInRange(m_DebrisQueryBuffer)` 零分配 |
| SRP FIREBALL | ✅ DONE | `EntityFactory::SpawnFromPlayer()` 新增：player-fired 投射物建立路徑從 `PlayingSceneHandler::SpawnPlayerFireball` 移至 Factory；使用 `MakeProjectileDef` 作為唯一 def 來源，消除所有 inline EntityDef 手動構建；`PlayingSceneHandler` 降至純位置計算 (3 行邏輯) |
| SPAGHETTI FIX | ✅ DONE | `Entity.cpp` 消除 `m_LevelName=="8-4"` 與 `GetName()=="..."` 字串比較：改以 `EntityDef::renderTargetWidth`（EntityFactory 注入）驅動縮放；Z-index 改用 `EntityType` 枚舉而非 name 字串 |
| STAR FIRE | ✅ DONE | `PlayerState::CanShootFire()` 新增：封裝「FIRE 狀態 OR（星星狀態且 MemoryState==FIRE）」；`SetFireShooting()` 改用 `CanShootFire()` — 消除直接比對 enum 的 hardcode；`ForceApplyPowerState()` 改用 `IsBigOrFire()` — 消除重複 inline 大小判斷；`Player::UpdateView()` 射擊時回退 FIRE 精靈（star 調色盤無射擊幀） |
| FORM REFACTOR | ✅ DONE | 導入多型 `IPlayerForm` (State Pattern) 封裝 Mario 的力量型態狀態機，消除 `PlayerState` 與 `Player` 中的所有硬編碼 enum 條件分支 |

---

## 7. OOP 原則遵守確認

| 原則 | 實現方式 | 狀態 |
|------|---------|------|
| 所有實體繼承 Util::GameObject | Player, Entity, Block, UIImage, UIText 全部繼承 | ✅ DONE |
| 沒有 God Class | App 只持有子系統 + TransitionTo()；邏輯分散到各 Handler/Manager | ✅ DONE |
| MVC 架構 | PlayerState(M) ← Player(V) ← InputHandler(C) | ✅ DONE |
| State Pattern | 10 個 ISceneHandler 子類；App::Update() 只有兩行；導入 `IPlayerForm` 多型管理力量型態 | ✅ DONE |
| Strategy Pattern | 19 個 `IEntityBehavior` + 4 個 `IEnemyDeathAnimation` + 1 個 `IPlayerDeathAnimation` + 5 個 `IPlayerForm` 力量狀態策略 | ✅ DONE |
| Factory Pattern | EntityFactory 唯一入口；EnemyDeathStyleFactory 策略選擇；符合 SRP | ✅ DONE |
| DIP | IAudioService 介面；AudioManager 實作；ServiceLocator 輔助注入 | ✅ DONE |
| OCP 原則 | 新增怪物/狀態不修改現有類別 | ✅ DONE |
| DRY 原則 | GameConfig 統一座標轉換 helpers；靜態 Sprite Cache；unordered_map 路徑表 | ✅ DONE |
| 不修改 CMakeLists.txt | 所有新增透過 files.cmake | ✅ DONE |
| 代碼注釋全英文 | 所有 .hpp/.cpp 注釋均為英文 | ✅ DONE |
