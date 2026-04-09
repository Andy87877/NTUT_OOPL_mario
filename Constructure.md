# Super Mario Bros. 專案架構概覽

# Super Mario Bros. 專案架構概覽 (簡易版)

這份文件旨在以最簡單、直觀的方式，展示本專案的核心**物件導向 (OOP) 架構**與系統運作方式。

---

## 一、核心遊戲物件 (OOP 繼承關係)

遊戲畫面中看到的所有會動、會畫出來的東西，全部都繼承自最基礎的 `GameObject`。我們將相似的功能封裝在父類別，再透過繼承來實作具體物件。

```mermaid
classDiagram
    direction TB

    %% 基礎物件
    class GameObject["Util::GameObject (基礎物件)"] {
        +位置 (X, Y)
        +圖片渲染
        +顯示隱藏設定
    }

    %% 衍生類別
    class Player["Player (瑪利歐)"] {
        +玩家控制 (跑、跳、下蹲)
        +狀態管理 (變大、火力、無敵)
    }

    class Entity["Entity (動態實體)"] {
        +物理引擎 (重力、移動)
        +通用碰撞行為
        +代表：敵人 (Goomba)、道具 (Mushroom)、火球
    }

    class Block["Block (靜態方塊)"] {
        +方塊屬性 (可破壞、是否為容器)
        +撞擊彈跳特效
        +代表：磚塊、問號方塊、水管
    }

    class MovingPlatform["MovingPlatform (移動平台)"] {
        +來回移動邏輯
        +承載玩家移動
    }

    class UIText["UIText (UI 文字)"] {
        +包裝 Util::Text
        +設置位置、顏色、內容
        +HUD、選單、浮動文字基礎
    }

    class UIElement["UI特效類"] {
        +浮動文字 (FloatingText)
        +死亡畫面 (DeathScreen)
    }

    %% 繼承關係
    GameObject <|-- Player : 繼承 (View layer)
    GameObject <|-- Entity : 繼承
    GameObject <|-- Block : 繼承
    GameObject <|-- UIText : 繼承
    GameObject <|-- UIElement : 繼承
    Block <|-- MovingPlatform : 繼承
```

---

## 一-補：Phase 4 行為策略模式 (Strategy Pattern for Behaviors)

Phase 4 實作了**策略模式**，讓實體的行為邏輯獨立於實體類別本身。這使得敵人、道具、火球的 AI 都能動態切換。

```mermaid
classDiagram
    direction TB

    class IEntityBehavior["IEntityBehavior (接口)"] {
        +Update(state, level, player, timer)*
        +OnPlayerCollision(state, player, isFromAbove)*
        +Clone() Behavior*
        +GetName() string*
    }

    class DefaultEntityBehavior["DefaultEntityBehavior"] {
        +被動實體（金幣、道具）
        +不主動行動
    }

    class EnemyBehavior["EnemyBehavior"] {
        +巡邏邏輯
        +碰撞方向轉向
        +類型: Goomba, Koopa, Shell
    }

    class ItemBehavior["ItemBehavior"] {
        +橫向移動＋彈跳
        +類型: Mushroom, Star, etc
    }

    class FireballBehavior["FireballBehavior"] {
        +拋物線軌跡
        +類型: Player / Bowser
    }

    class BowserBehavior["BowserBehavior"] {
        +Boss AI 多階段
        +巡邏→火球→跳躍→敗北
    }

    %% 繼承關係
    IEntityBehavior <|.. DefaultEntityBehavior : 實作
    IEntityBehavior <|.. EnemyBehavior : 實作
    IEntityBehavior <|.. ItemBehavior : 實作
    IEntityBehavior <|.. FireballBehavior : 實作
    IEntityBehavior <|.. BowserBehavior : 實作
```

### 行為系統的優勢

- **策略模式**：實體行為獨立，易於新增敵人類型
- **複合性**：多個實體合營同一個行為類別
- **可測試性**：行為邏輯與渲染分離

### 已實作類別說明

| 類別 | 繼承自 | 角色 (MVC) | 說明 |
|------|--------|-----------|------|
| `Block` | `Util::GameObject` | View | Terrain tile with collision, animation, hit-bounce |
| `Player` | `Util::GameObject` | View | Mario rendering, sprite cache, direction flip |
| `Entity` | `Util::GameObject` | View | Enemy/power-up/coin rendering, direction flip |
| `PlayerState` | None (data) | Model | Position, velocity, power state, animation keys |
| `EntityState` | None (data) | Model | Entity position, velocity, squish/death state |
| `InputHandler` | None | Controller | Keyboard input -> PlayerState |
| `Level` | None | Model | CSV parsing, block grid, spawn point tracking |
| `EntityFactory` | None | Factory | Creates Entity instances from level spawn data |
| `LevelCompleteController` | None | Controller | Flagpole slide, walk-to-castle, pipe warp |
| `GameStateManager` | None | Service | Score, lives, coins, time, level progression |
| `Camera` | None | Service | Viewport scrolling following player |
| `PhysicsEngine` | None | Service | Gravity, jump parabola calculation |
| `CollisionManager` | None | Service | Player-Block collision detection & resolution |
| `GameConfig` | None | Config | Global constants (tile size, physics, Z-layers) |
| `Collider (AABB)` | None | Data | Axis-aligned bounding box |
| `EntityDef / BlockDef` | None | Data | CSV lookup data structures |
| `SpritePathResolver` | None | Utility | Sprite path name builder |
| `UIText` | `Util::GameObject` | View | Wraps Util::Text, renders HUD/menu text |
| `FloatingText` | None | Component | Manages floating score text with upward motion |
| **IEntityBehavior** | **None** | **Interface** | **Strategy pattern base for entity AI** |
| **DefaultEntityBehavior** | **IEntityBehavior** | **Strategy** | **Passive entities (coins, power-ups)** |
| **EnemyBehavior** | **IEntityBehavior** | **Strategy** | **Goomba & Koopa Troopa patrol AI** |
| **ItemBehavior** | **IEntityBehavior** | **Strategy** | **Power-up bouncing behavior** |
| **FireballBehavior** | **IEntityBehavior** | **Strategy** | **Projectile trajectory & collision** |
| **BowserBehavior** | **IEntityBehavior** | **Strategy** | **Boss AI (8-4 duel phases)** |
| `UIManager` | None | Manager | HUD/Menu rendering, floating text control, scene UI state |

---

## 二、遊戲大腦與管理器 (App & Managers)

`App` 類別是整個遊戲的「大腦」，負責在每一幀 (Frame) 去呼叫底下各自獨立的「管理器 (Managers)」，把繁雜的工作分派出去，保持程式碼乾淨。

```mermaid
classDiagram
    direction LR

    class App["App (遊戲主迴圈)"] {
        +Start() 初始化
        +Update() 每幀更新
        +End() 遊戲結束
        +LoadLevel() 載入關卡
        +State: TITLE/LOADING/PLAYING/DEATH/GAME_OVER/ESC_MENU
    }

    class Managers["Subsystems (Managers)"] {
        +Level (CSV map loading, block grid)
        +PhysicsEngine (gravity & jump)
        +CollisionManager (player-block, entity-block)
        +InputHandler (keyboard -> PlayerState)
        +EntityFactory (spawn entities from level data)
        +Camera (viewport scrolling)
        +SpritePathResolver (sprite paths)
    }

    App *-- Managers : coordinates
```

---

## 三、遊戲場景流程 (Scene Flow)

遊戲玩法的切換是由場景狀態機來控制的，每個階段都有專屬的邏輯處理器。

```mermaid
stateDiagram-v2
    [*] --> TitleScene : 啟動遊戲
    TitleScene --> PlayingScene : 按下開始
    
    PlayingScene --> PauseMenu : 按 ESC 暫停
    PauseMenu --> PlayingScene : 恢復遊戲
    
    PlayingScene --> DeathScene : 瑪利歐死亡
    DeathScene --> PlayingScene : 復活重試
    DeathScene --> GameOverScene : 生命耗盡
    
    PlayingScene --> LoadingScene : 抵達旗桿過關
    LoadingScene --> PlayingScene : 載入下一關
```

---

## 四、圖層渲染順序 (Z-Index)

為了保證畫面疊加正確，每個遊戲物件在產生時都會被賦予一個圖層高度 (Z-Index)，數字越大的畫在越上層。

| 圖層高度 (Z-Index) | 負責內容 | 說明 |
| :---: | :--- | :--- |
| **90+** | **系統介面層** | 暫停選單、死亡畫面覆蓋層、過關結算 |
| **10** | **特效層** | 吃到金幣或踩死敵人時噴出的「浮動分數文字」 |
| **5** | **動態物件層** | 敵人 (Goomba, Koopa)、道具 (蘑菇, 星星)、火球 |
| **0** | **玩家層** | 瑪利歐本人 |
| **-5** | **地形方塊層** | 地板、磚塊、問號方塊、水管 |
| **-10** | **背景層** | 藍天背景色、背景山脈與雲朵裝飾 |

---

## 五、專案程式碼目錄結構

專案資料夾按照**領域職責 (Domain-based)** 分類，對應 MVC 架構。分為 **5 大領域**：

```mermaid
graph TD
    Root["Mario/ 根目錄"]

    Root --> Core["🔵 Core<br/>底層引擎與工具"]
    Root --> Managers["🟢 Managers<br/>跨場景全域服務"]
    Root --> Entities["🟡 Entities<br/>遊戲物件 (Model + View)"]
    Root --> Controllers["🔴 Controllers<br/>遊戲流程協調"]
    Root --> Scenes["🟣 Scenes<br/>場景生命週期"]

    Core --> C1["AppCore / GameTheater"]
    Core --> C2["GameConfig / PhysicsConstants"]
    Core --> C3["PhysicsEngine / Camera"]
    Core --> C4["SpritePathResolver"]

    Managers --> M1["AudioManager / IAudioService"]
    Managers --> M2["GameStateManager / LevelManager"]
    Managers --> M3["CollisionManager / RenderManager"]
    Managers --> M4["SceneManager / UIManager"]
    Managers --> M5["DeathScreenManager / FloatingText"]

    Entities --> E1["Entity (View) ← EntityModel (Model)"]
    Entities --> E2["Player (View) ← PlayerState (Model)"]
    Entities --> E3["Block / BlockState / MovingPlatform"]
    Entities --> E4["EntityFactory / EntityDef"]
    Entities --> E5["Level / Collider / CollisionContext"]
    Entities --> E6["Blocks/ 方塊策略（已存在）"]
    Entities --> E7["Entities/ 敵人/道具/飛彈（已存在）"]

    Controllers --> CT1["PlayingSceneController"]
    Controllers --> CT2["EndingSequenceController"]
    Controllers --> CT3["PipeWarpController / InputHandler"]

    Scenes --> S1["ISceneHandler (介面)"]
    Scenes --> S2["TitleSceneHandler / LoadingSceneHandler"]
    Scenes --> S3["GameOverSceneHandler / DeathSceneHandler"]
    Scenes --> S4["ESCMenuSceneHandler / ESCMenuState"]
    Scenes --> S5["IGameState"]
```

---

### `include/Mario/` — 標頭檔

```text
include/Mario/
├── Core/
│   ├── AppCore.hpp
│   ├── GameTheater.hpp
│   ├── GameConfig.hpp
│   ├── PhysicsConstants.hpp
│   ├── PhysicsEngine.hpp
│   ├── Camera.hpp
│   └── SpritePathResolver.hpp
│
├── Managers/
│   ├── AudioManager.hpp
│   ├── IAudioService.hpp
│   ├── GameStateManager.hpp
│   ├── LevelManager.hpp
│   ├── CollisionManager.hpp
│   ├── RenderManager.hpp
│   ├── SceneManager.hpp
│   ├── UIManager.hpp
│   ├── DeathScreenManager.hpp
│   └── FloatingText.hpp
│
├── Entities/
│   ├── Entity.hpp          ← View 層 (繼承 EntityModel + Util::GameObject)
│   ├── EntityModel.hpp     ← Model 層（純邏輯、可單元測試）
│   ├── EntityDef.hpp
│   ├── EntityFactory.hpp
│   ├── Level.hpp
│   ├── MovingPlatform.hpp
│   ├── Block.hpp           ← View 層 (繼承 Util::GameObject)
│   ├── BlockState.hpp
│   ├── Collider.hpp
│   ├── CollisionContext.hpp
│   ├── Player.hpp          ← View 層 (委派給 PlayerState)
│   ├── PlayerState.hpp     ← Model 層（狀態機、可單元測試）
│   ├── Blocks/             ← 方塊碰撞策略（已存在）
│   └── Entities/           ← 敵人/道具/飛彈模型（已存在）
│
├── Controllers/
│   ├── PlayingSceneController.hpp
│   ├── EndingSequenceController.hpp
│   ├── PipeWarpController.hpp
│   └── InputHandler.hpp
│
└── Scenes/
    ├── ISceneHandler.hpp   ← 場景介面（OCP 擴充點）
    ├── IGameState.hpp
    ├── TitleSceneHandler.hpp
    ├── LoadingSceneHandler.hpp
    ├── GameOverSceneHandler.hpp
    ├── DeathSceneHandler.hpp
    ├── ESCMenuSceneHandler.hpp
    └── ESCMenuState.hpp
```

---

### `src/Mario/` — 實作檔

```text
src/Mario/
├── Core/
│   ├── AppCore.cpp
│   ├── GameTheater.cpp
│   ├── GameConfig.cpp
│   ├── PhysicsEngine.cpp
│   ├── Camera.cpp
│   └── SpritePathResolver.cpp
│
├── Managers/
│   ├── AudioManager.cpp
│   ├── GameStateManager.cpp
│   ├── LevelManager.cpp
│   ├── CollisionManager.cpp
│   ├── RenderManager.cpp
│   ├── SceneManager.cpp
│   ├── UIManager.cpp
│   ├── DeathScreenManager.cpp
│   └── FloatingText.cpp
│
├── Entities/
│   ├── Entity.cpp
│   ├── EntityModel.cpp
│   ├── EntityFactory.cpp
│   ├── Level.cpp
│   ├── MovingPlatform.cpp
│   ├── Block.cpp
│   ├── Player.cpp
│   ├── PlayerState.cpp
│   ├── Blocks/             ← 方塊策略實作
│   └── Entities/           ← 敵人/道具/飛彈實作
│
├── Controllers/
│   ├── PlayingSceneController.cpp
│   ├── EndingSequenceController.cpp
│   ├── PipeWarpController.cpp
│   └── InputHandler.cpp
│
└── Scenes/
    ├── TitleSceneHandler.cpp
    ├── LoadingSceneHandler.cpp
    ├── GameOverSceneHandler.cpp
    ├── DeathSceneHandler.cpp
    └── ESCMenuSceneHandler.cpp
```

---

**架構核心精神**：

- **Core**：最底層，不依賴任何其他 Mario 元件。
- **Managers**：跨場景的全域服務，只被 Controller 調用，不主動呼叫 Controller。
- **Entities**：同時含 Model（`EntityModel`, `PlayerState`）與 View（`Entity`, `Player`, `Block`），將邏輯計算與渲染分開。
- **Controllers**：協調 Model 與 View，決定每幀的遊戲流程（SRP）。
- **Scenes**：實作 `ISceneHandler` 介面，每個場景獨立管理生命週期（OCP）。

## 一、完整類別繼承圖（View 層 — 繼承 `Util::GameObject`）

```mermaid
classDiagram
    direction TB

    class GameObject["Util::GameObject"] {
        +Transform m_Transform
        #shared_ptr~Drawable~ m_Drawable
        #bool m_Visible
        +SetDrawable(drawable)
        +SetZIndex(z)
        +SetVisible(bool)
        +GetVisible() bool
    }

    class Entity {
        -shared_ptr~Drawable~ m_Drawable
        -shared_ptr~Animation~ m_Animation
        -unique_ptr~IEntityBehavior~ m_Behavior
        -int m_SquishCounter
        -bool m_Flipped
        +Entity(name, worldX, worldY, sizeX, sizeY, entityLookUp, dir, jumpMultiply)
        +Update(dt, hardMode)
        +SetBehavior(unique_ptr~IEntityBehavior~)
        +OnPlayerTouch(CollisionContext) bool
        +OnPlayerStomp(CollisionContext) bool
        +Squish()
        +FlipDirection()
        +GetAABB() AABB
        +GetFallDetectAABB() AABB
        -SetupSprite(typeName)
    }

    class Player {
        -PlayerState m_PS
        -string m_LastSpriteKey
        -map~string, shared_ptr~Image~~ m_SpriteCache
        +Player(worldX, worldY, startState)
        +Update(dt)
        +MoveRight(dt) / MoveLeft(dt)
        +StartJump() / SetCrouch(bool)
        +SetState(int) / GetState() int
        +SetControllable(bool) / IsControllable()
        +StartDeathAnimation() / IsDeathAnimActive()
        +StartTransform(from, to) / IsTransforming()
        +StartInvincibility(frames) / IsInvincible()
        +GetAABB() AABB / GetHitBox() AABB
        +GetPlayerState() PlayerState
        -UpdateSprite()
    }

    class Block {
        -int m_BlockID
        -int m_GridX, m_GridY
        -string m_Name
        -bool m_Solid, m_Breakable, m_Background
        -bool m_IsGoal, m_IsContainer, m_IsHit
        -bool m_Spawner, m_BounceBack
        -string m_Contents, m_SpawnEntity
        -shared_ptr~Image~ m_Sprite, m_HitSprite
        -shared_ptr~Animation~ m_Animation
        +Block(blockID, gridX, gridY, lookUp)
        +Update(cameraOffset)
        +OnHit(playerState)
        +Bounce() / Break()
        +IsSolid() / IsBreakable() / IsContainer()
        +virtual GetWorldX() / GetWorldY() float
        +virtual GetAABB() AABB
        +virtual GetCarrierVelX/Y() float
        +GetSpawnContents(playerState) string
    }

    class MovingPlatform {
        -Axis m_Axis
        -float m_Phase, m_AngularSpeed, m_Range
        -float m_OriginX, m_OriginY
        -float m_VelocityX, m_VelocityY
        +MovingPlatform(blockID, gridX, gridY, lookUp, range, period, axis)
        +UpdatePlatform(dt)
        +GetWorldX() float override
        +GetWorldY() float override
        +GetAABB() AABB override
        +GetCarrierVelX/Y() float override
    }

    class FloatingText {
        -float m_X, m_Y
        -int m_Score
        -float m_Timer
        +FloatingText(x, y, score, fontPath)
        +Update() bool
    }

    class DeathScreenManager {
        +ShowDeathScreen()
        +HideDeathScreen()
        +Update()
    }

    %% ── 繼承關係 ──
    GameObject <|-- Entity : View層 (多重繼承 EntityModel)
    GameObject <|-- Player : View層 (委派 PlayerState)
    GameObject <|-- Block : View層
    GameObject <|-- FloatingText : View層
    Block <|-- MovingPlatform : 移動平台
```

## 二、Model 層類別圖（純邏輯，可單元測試）

```mermaid
classDiagram
    direction TB

    class EntityModel {
        #float m_WorldX, m_WorldY
        #float m_VelX, m_VelY
        #float m_SizeX, m_SizeY
        #int m_EntityID
        #string m_Name
        #Direction m_Direction
        #bool m_Active, m_IsPowerUp, m_IsEnemy
        #bool m_IsCoin, m_IsStatic, m_IsBounce
        #bool m_IsGrounded, m_DoesJump, m_DoesCollide
        #bool m_Squishable, m_Squashed
        #int m_ScoreWorth, m_PowerUpState
        #bool m_HasWings
        #int m_BowserTimer
        #bool m_ShouldBreatheFire, m_ShouldThrowAxe
        +EntityModel(typeName, worldX, worldY, sizeX, sizeY, entityLookUp, dir, jumpMultiply)
        +Jump() / LoseWings() / FlipDirection() / Delete()
        +Getters 與 Setters (位置, 物理, 分類, AI)
    }

    class PlayerState {
        -float m_WorldX, m_WorldY
        -float m_VelX, m_VelY
        -State m_State
        -AnimState m_AnimState
        -string m_Direction
        -bool m_IsGrounded, m_Controllable
        -bool m_IsCrouching, m_Moving, m_Running
        -bool m_DeathAnimActive, m_IsTransforming
        -int m_InvFrames
        -bool m_IsSwimming, m_IsClimbing
        -float m_AccelX
        +PlayerState(worldX, worldY, startState)
        +MoveRight(dt) / MoveLeft(dt)
        +StartJump() / SetState(int)
        +UpdateLogic(dt) bool
        +StartDeathAnimation() / StartTransform(from, to)
        +StartInvincibility(frames)
        +GetAABB() / GetFallDetectAABB() / GetHitBox() AABB
        +GetSpriteKey() string
    }

    class BlockState {
        +SetHit(bool)
        +IsHit() bool
        +CanBreak(playerState) bool
    }

    class GameStateManager {
        -int m_Score, m_Coins, m_Lives
        -int m_World, m_Level
        -int m_TimeRemaining
        +GameStateManager()
        +AddScore(int) / AddCoin()
        +GetScore() / GetCoins() / GetLives() int
        +AdvanceLevel() / ResetLevel()
        +CountdownTimeBonus() int
    }

    class ESCMenuState {
        -int m_SelectedIndex
        -bool m_IsVisible
        +ESCMenuState()
        +Show() / Hide()
        +MoveUp() / MoveDown()
        +GetSelectedIndex() int
        +IsVisible() bool
    }

    class IGameState {
        <<interface>>
        +Enter()
        +Update(dt)
        +Exit()
    }

    %% Entity 是 EntityModel 的子類 (多重繼承)
    EntityModel <|-- Entity : Model 基類
```

## 三、Controller 層類別圖

```mermaid
classDiagram
    direction TB

    class App {
        -State m_CurrentState
        -InputHandler m_InputHandler
        -GameStateManager m_GameState
        -PhysicsEngine m_Physics
        -UIManager m_UI
        -SceneManager m_SceneManager
        -CollisionManager m_CollisionManager
        -LevelManager m_LevelManager
        -RenderManager m_RenderManager
        -DeathScreenManager m_DeathScreenManager
        -EntityFactory m_EntityFactory
        -Level m_Level
        -shared_ptr~Player~ m_Player
        -Camera m_Camera
        -Renderer m_Root
        -unique_ptr~TitleSceneHandler~ m_TitleHandler
        -unique_ptr~LoadingSceneHandler~ m_LoadingHandler
        -unique_ptr~PlayingSceneController~ m_PlayingController
        -unique_ptr~GameOverSceneHandler~ m_GameOverHandler
        -unique_ptr~DeathSceneHandler~ m_DeathHandler
        -unique_ptr~ESCMenuSceneHandler~ m_ESCMenuHandler
        +Start() / Update() / End()
        -UpdatePlaying(dt) / UpdateLevelComplete(dt)
    }

    class PlayingSceneController {
        -InputHandler* m_InputHandler
        -GameStateManager* m_GameState
        -PhysicsEngine* m_Physics
        -CollisionManager* m_CollisionManager
        -RenderManager* m_RenderManager
        -UIManager* m_UI
        -EntityFactory* m_EntityFactory
        -LevelManager* m_LevelManager
        -Camera* m_Camera
        -SceneManager* m_SceneManager
        -shared_ptr~Player~ m_Player
        -shared_ptr~Level~ m_Level
        -unique_ptr~EndingSequenceController~ m_EndingController
        -unique_ptr~PipeWarpController~ m_PipeWarpController
        +PlayingSceneController(各 Manager 引用)
        +Update(dt) NextSceneSignal
        +SetGameObjects(player, level)
        +SetDependentControllers(ending, pipeWarp)
        -HandleInput(dt) / UpdatePhysics(dt)
        -CheckCollisions() / UpdateEntities(dt)
        -CheckDeathConditions() bool
        -UpdateTimeAndBonus(dt) / UpdateUI()
    }

    class EndingSequenceController {
        -GameStateManager* m_GameState
        -UIManager* m_UI
        -Camera* m_Camera
        -SceneManager* m_SceneManager
        +Update(player, level, gameTimer, ...) bool
        +TriggerFlagpole(gameTimer)
        +ShouldStartEnding(gameTimer) bool
        +ShouldAdvanceLevel(gameTimer) bool
        +Reset()
    }

    class PipeWarpController {
        -GameStateManager* m_GameState
        -RenderManager* m_RenderManager
        -LevelManager* m_LevelManager
        +Update(state, pipeDirection, ...) bool
        +InitiatePipeWarp(direction, pipeX, pipeY)
        +IsWarpInProgress() bool
        +Reset()
    }

    class ISceneHandler {
        <<interface>>
        +Update(dt)*
        +OnEnter()
        +OnExit()
    }

    class TitleSceneHandler {
        +Update(dt) override
        +OnEnter() override
    }

    class LoadingSceneHandler {
        +Update(dt) override
        +OnEnter() override
    }

    class GameOverSceneHandler {
        +Update(dt) override
        +OnEnter() override
    }

    class DeathSceneHandler {
        +Update(dt) override
    }

    class ESCMenuSceneHandler {
        +Update(dt) override
        +OnEnter() override
        +OnExit() override
    }

    %% ── 繼承關係 ──
    ISceneHandler <|.. TitleSceneHandler
    ISceneHandler <|.. LoadingSceneHandler
    ISceneHandler <|.. GameOverSceneHandler
    ISceneHandler <|.. DeathSceneHandler
    ISceneHandler <|.. ESCMenuSceneHandler

    %% ── 組合關係 ──
    App *-- PlayingSceneController
    App *-- TitleSceneHandler
    App *-- LoadingSceneHandler
    App *-- GameOverSceneHandler
    App *-- DeathSceneHandler
    App *-- ESCMenuSceneHandler
    PlayingSceneController *-- EndingSequenceController
    PlayingSceneController *-- PipeWarpController
```

## 四、Strategy Pattern — 行為策略類別圖

```mermaid
classDiagram
    direction TB

    class IEntityBehavior {
        <<interface>>
        +Update(Entity, dt) bool
        +OnPlayerTouch(Entity, CollisionContext) bool
        +OnPlayerStomp(Entity, CollisionContext) bool
        +OnWallCollide(Entity, Block, CollisionContext)
    }

    class DefaultEntityBehavior {
        %% 使用 IEntityBehavior 預設實作
    }
    class GoombaBehavior {
        +OnPlayerStomp(Entity, CollisionContext) bool override
    }
    class KoopaBehavior {
        +OnPlayerStomp(Entity, CollisionContext) bool override
    }
    class KoopaShellBehavior {
        +OnPlayerTouch(Entity, CollisionContext) bool override
    }
    class BowserBehavior {
        +Update(Entity, dt) bool override
    }
    class ItemBehavior {
        +OnPlayerTouch(Entity, CollisionContext) bool override
    }
    class CoinBehavior {
        +OnPlayerTouch(Entity, CollisionContext) bool override
    }
    class CoinGetBehavior {
        +Update(Entity, dt) bool override
    }
    class FireballBehavior {
        +Update(Entity, dt) bool override
    }
    class BowserFireBehavior {
        +Update(Entity, dt) bool override
    }

    IEntityBehavior <|.. DefaultEntityBehavior
    IEntityBehavior <|.. GoombaBehavior
    IEntityBehavior <|.. KoopaBehavior
    IEntityBehavior <|.. KoopaShellBehavior
    IEntityBehavior <|.. BowserBehavior
    IEntityBehavior <|.. ItemBehavior
    IEntityBehavior <|.. CoinBehavior
    IEntityBehavior <|.. CoinGetBehavior
    IEntityBehavior <|.. FireballBehavior
    IEntityBehavior <|.. BowserFireBehavior
```

## 五、碰撞策略類別圖

```mermaid
classDiagram
    direction TB

    class ICollisionStrategy {
        <<interface>>
        +OnPlayerHeadBump(ctx, block, playerHitBox) bool*
        +OnPlayerContact(ctx, block, playerHitBox, flag) bool*
    }

    class GoalCollisionStrategy {
        +OnPlayerHeadBump(...) bool override
        +OnPlayerContact(...) bool override
    }
    class ContainerCollisionStrategy {
        +OnPlayerHeadBump(...) bool override
        +OnPlayerContact(...) bool override
    }
    class BreakableCollisionStrategy {
        +OnPlayerHeadBump(...) bool override
        +OnPlayerContact(...) bool override
    }

    class CollisionStrategyFactory {
        <<static>>
        +CreateStrategy(Block) unique_ptr~ICollisionStrategy~
    }

    ICollisionStrategy <|.. GoalCollisionStrategy
    ICollisionStrategy <|.. ContainerCollisionStrategy
    ICollisionStrategy <|.. BreakableCollisionStrategy
    CollisionStrategyFactory ..> ICollisionStrategy : creates
```

## 六、場景狀態管理（GameTheater — State Pattern）

```mermaid
classDiagram
    direction TB

    class Scene {
        <<abstract>>
        +Update(deltaTime, App)*
        +Render(Renderer)*
        +GetName() string*
        +GetNextScene() Scene
        +OnEnter() / OnExit()
    }

    class TitleScene {
        -float m_elapsedTime
        +Update(deltaTime, App) override
        +Render(Renderer) override
        +GetName() string override
        +GetNextScene() Scene override
    }

    class PlayingScene {
        -float m_levelTime
        +Update(deltaTime, App) override
        +Render(Renderer) override
        +GetName() string override
        +OnEnter() override
    }

    class PauseScene {
        -unique_ptr~Scene~ m_resumeTarget
        +PauseScene(resumeTarget)
        +Update(deltaTime, App) override
        +Render(Renderer) override
        +GetName() string override
    }

    class GameTheater {
        -App* m_app
        -vector~unique_ptr~Scene~~ m_sceneStack
        +GameTheater(App)
        +Update(deltaTime)
        +Render(Renderer)
        +PushScene(unique_ptr~Scene~)
        +PopScene()
        +SwitchScene(unique_ptr~Scene~)
        +GetCurrentScene() Scene
        +HasScene() bool
    }

    Scene <|-- TitleScene
    Scene <|-- PlayingScene
    Scene <|-- PauseScene
    GameTheater o-- Scene : 管理場景堆
```

## 七、基礎設施與服務層

```mermaid
classDiagram
    direction TB

    class ServiceLocator {
        <<static>>
        -shared_ptr~EventSystem~ s_EventSystem
        -shared_ptr~AudioManager~ s_AudioManager
        -bool s_Initialized
        +Initialize()
        +Shutdown()
        +GetEventSystem() EventSystem
        +GetAudioManager() AudioManager
        +SetEventSystem(shared_ptr) [測試用]
        +SetAudioManager(shared_ptr) [測試用]
        +IsInitialized() bool
    }

    class IAudioService {
        <<interface>>
        +PlaySFX(sfxType)*
        +PlayBGM(bgmType)*
        +StopBGM()* / PauseBGM()* / ResumeBGM()*
    }

    class AudioManager {
        +PlaySFX(sfxType) override
        +PlayBGM(bgmType) override
        +StopBGM() / PauseBGM() / ResumeBGM() override
    }

    class EventSystem {
        +Publish(EventType, data)
        +Subscribe(EventType, callback)
    }

    class EntityFactory {
        +CreateEntity(name, worldX, worldY, dir, jumpMul) shared_ptr~Entity~
    }

    class SpritePathResolver {
        <<static>>
        +GetSpritePath(category, name, frame) string
    }

    IAudioService <|.. AudioManager
    ServiceLocator ..> EventSystem : provides
    ServiceLocator ..> AudioManager : provides
```

## 八、資料結構（Data Classes）

```mermaid
classDiagram
    direction TB

    class EntityDef {
        +string name
        +int id
        +float sizeX, sizeY
        +int isPowerUp, isEnemy, isCoin, isStatic
        +int doesCollide, doesJump, squishable
        +int scoreWorth, pState
        +int isBounce, fromBlock, isAnimated
        +int animFrames, animBuffer, oneLoop, koopaSquash
    }

    class BlockDef {
        +int id
        +string name
        +bool solid, breakable, background
        +bool isGoal, isContainer
        +int contentID
        +int r, g, b
        +string contents, hitSpriteName
        +bool animated, bounceBack, spawner
        +int animationFrames
        +string spawnEntity
    }

    class CollisionContext {
        +shared_ptr~Player~ player
        +shared_ptr~Level~ level
        +EntityFactory* entityFactory
        +GameStateManager* gameState
        +Renderer* root
        +int gameTimer
        +int invTimer
    }

    class AABB["Collider::AABB"] {
        +float left, top, right, bottom
        +Intersects(AABB) bool
        +Contains(x, y) bool
    }

    class PhysicsConfig {
        <<struct>>
        +constexpr GRAVITY
        +constexpr JUMP_VELOCITY
        +constexpr MOVE_VELOCITY
        +constexpr RUN_MULTIPLIER
    }

    class CameraConfig {
        <<struct>>
        +constexpr WINDOW_WIDTH
        +constexpr WINDOW_HEIGHT
        +constexpr VIEWPORT_TILES_X/Y
        +constexpr TILE_SIZE, SCALE_FACTOR
    }

    class EnemyConfig {
        +int ID
        +string name
        +float velocityX, velocityY
        +float sizeX, sizeY
        +int health
        +bool isFlying
    }

    class LevelRuleConfig {
        +int world, level
        +bool startWithFireFlower, startWithStar
        +int initialLives, timeLimit
        +string bgmType, theme
    }

    class GameConfig {
        <<static>>
        +constexpr PhysicsConfig Physics
        +constexpr CameraConfig Camera
        +GetEnemyConfig(entityID) EnemyConfig
        +GetLevelRuleConfig(world, level) LevelRuleConfig
        +Initialize()
    }

    GameConfig *-- PhysicsConfig
    GameConfig *-- CameraConfig
    GameConfig o-- EnemyConfig
    GameConfig o-- LevelRuleConfig
```

## 九、系統組件關係圖

```mermaid
classDiagram
    direction LR

    App *-- InputHandler
    App *-- GameStateManager
    App *-- PhysicsEngine
    App *-- UIManager
    App *-- SceneManager
    App *-- CollisionManager
    App *-- LevelManager
    App *-- RenderManager
    App *-- DeathScreenManager
    App *-- EntityFactory
    App *-- Camera
    App *-- Level
    App o-- Player
    App o-- Entity : Flag

    App *-- PlayingSceneController
    PlayingSceneController *-- EndingSequenceController
    PlayingSceneController *-- PipeWarpController

    App *-- TitleSceneHandler
    App *-- LoadingSceneHandler
    App *-- GameOverSceneHandler
    App *-- DeathSceneHandler
    App *-- ESCMenuSceneHandler

    Player o-- PlayerState : Model 委派
    Entity --|> EntityModel : Model 繼承
    Entity o-- IEntityBehavior : Strategy
    Block o-- BlockDef : 定義資料
    CollisionManager ..> ICollisionStrategy : 使用策略
    CollisionManager ..> CollisionStrategyFactory : 取得策略
    EntityFactory ..> IEntityBehavior : 注入行為

    ServiceLocator ..> EventSystem : 提供
    ServiceLocator ..> AudioManager : 提供
    AudioManager ..|> IAudioService : 實作
```

---

## 十、MVC 架構對應說明

### 10.1 Model 層（純邏輯，可單元測試）

| 類別 | 職責 |
|------|------|
| `EntityModel` | 實體物理狀態、分類標誌、AI 信號 |
| `PlayerState` | 玩家移動、跳躍、變身、無敵、碰撞箱、精靈鍵值 |
| `BlockState` | 方塊撞擊與內容物釋放邏輯 |
| `GameStateManager` | 分數、金幣、生命值、關卡進度、時間管理 |
| `ESCMenuState` | 暫停選單選項狀態 |
| `GameConfig` | 全域配置（物理常數、敵人表、關卡規則） |
| `PhysicsConstants` | 物理常量定義 |

### 10.2 View 層（繼承 `Util::GameObject`，負責渲染）

| 類別 | 職責 |
|------|------|
| `Player` | 玩家精靈渲染、動畫切換，委派邏輯給 `PlayerState` |
| `Entity` | 敵人/道具/投射物渲染，多重繼承 `EntityModel` + `GameObject` |
| `Block` | 方塊渲染、彈跳動畫、碰撞箱 |
| `MovingPlatform` | 正弦波振盪平台（繼承 `Block`） |
| `FloatingText` | 浮動分數文字特效 |
| `DeathScreenManager` | 死亡畫面覆蓋層 |
| `UIManager` | HUD 渲染（金幣、生命、分數、時間） |
| `Camera` | 攝影機跟隨與邊界限制 |

### 10.3 Controller 層（協調 Model 與 View）

| 類別 | 職責 |
|------|------|
| `App` | 主迴圈入口、場景協調、生命週期管理 |
| `PlayingSceneController` | 遊玩場景核心邏輯協調器 |
| `EndingSequenceController` | 通關結局流程（旗桿→城堡→下一關） |
| `PipeWarpController` | 水管傳送系統（地上↔地下切換） |
| `InputHandler` / `InputDispatcher` | 鍵盤輸入封裝與分派 |
| `CollisionManager` | 玩家↔方塊、玩家↔實體碰撞偵測 |
| `SceneManager` | 場景切換狀態機 |
| `ISceneHandler` 實作群 | 各場景獨立處理器（Title, Loading, Death, GameOver, ESC） |

### 10.4 基礎設施層

| 類別 | 職責 |
|------|------|
| `ServiceLocator` | 服務定位器（替代 Singleton，支援 DI） |
| `EventSystem` | 發布/訂閱事件系統 |
| `AudioManager` / `IAudioService` | 音效與背景音樂播放（DIP 抽象介面） |
| `EntityFactory` | 工廠模式，建立敵人/道具/投射物 |
| `SpritePathResolver` | 精靈圖路徑解析 |
| `RenderManager` | 統一管理可渲染物件 |
| `LevelManager` | 多關卡與地下室載入切換 |

---

## 十一、設計模式摘要

| 設計模式 | 應用位置 | 說明 |
|---------|---------|------|
| **Strategy** | `IEntityBehavior` | 敵人/道具行為可插拔替換（Goomba, Koopa, Bowser, Item...） |
| **Strategy** | `ICollisionStrategy` | 方塊碰撞邏輯可插拔替換（Goal, Container, Breakable） |
| **Strategy** | `ISceneHandler` | 場景處理器介面，新場景只需新增 handler |
| **Factory** | `EntityFactory` / `CollisionStrategyFactory` | 建立具體實體與碰撞策略 |
| **State** | `GameTheater` + `Scene` | 場景堆管理（Title→Playing→Pause→Death） |
| **Delegation** | `Player` → `PlayerState` | View 委派所有邏輯給 Model |
| **MVC** | 全專案 | Model（純邏輯）/ View（渲染）/ Controller（協調） |
| **Service Locator** | `ServiceLocator` | 替代 Singleton 的 DI 容器 |
| **Facade** | `App` / `AppCore` | 簡化子系統的統一入口 |
| **Coordinator** | `PlayingSceneController` | 協調遊玩場景的所有子系統 |
| **Template Method** | `ISceneHandler` | 場景生命週期模板（OnEnter → Update → OnExit） |
| **Observer** | `EventSystem` | 發布/訂閱事件（物件間鬆散耦合） |

---

## 十二 (新增): Phase 5 - 場景管理與管理器系統架構

### 12.1 Phase 5 新增模組概覽

Phase 5 實作了**完整的場景管理系統**與**全域管理器架構**，將遊戲流程從單一 `App` 狀態機轉變為模組化的場景堆棧系統。

#### 新增設計亮點

1. **策略模式場景處理** (`ISceneHandler`)
   - 每個場景獨立實作 `OnEnter()` / `Update()` / `OnExit()` 生命週期
   - 支援無限場景組合（可加入新場景無須修改 App）
   - 遵守開放封閉原則 (OCP)

2. **場景堆管理** (`SceneManager`)
   - 基於棧的場景管理，支援場景推入/彈出/切換
   - 自動呼叫 `OnEnter()` / `OnExit()` 鉤子
   - 支援場景轉換時的自動訊號處理

3. **服務定位器** (`ServiceLocator`)
   - 統一的依賴注入容器
   - 替代全域 Singleton 的更乾淨做法
   - 支援測試時模擬服務

4. **事件系統** (`EventSystem`)
   - 物件間的訊息傳遞解耦
   - 發布/訂閱模式
   - 支援多種事件類型

5. **音訊服務** (`IAudioService` + `AudioManager`)
   - 依賴反演原則 (DIP) 設計
   - BGM 與 SFX 分離管理
   - 便於單元測試（可注入 Mock 實作）

6. **UI 管理器** (`UIManager` + `FloatingText`)
   - 統一管理 HUD 渲染
   - 支援浮動文字特效
   - 場景轉換時自動清理

### 12.2 Phase 5 類別繼承關係

```mermaid
classDiagram
    direction TB

    %% Scene Handler Pattern
    class ISceneHandler["ISceneHandler (介面)"] {
        +OnEnter()*
        +Update() bool*
        +OnExit()*
        +GetNextSceneName() string*
        +GetName() string*
    }

    class TitleSceneHandler["TitleSceneHandler"] {
        +等待玩家開始
        +轉移到 LoadingScene
    }

    class LoadingSceneHandler["LoadingSceneHandler"] {
        +顯示關卡資訊
        +轉移到 PlayingScene
    }

    class DeathSceneHandler["DeathSceneHandler"] {
        +顯示死亡動畫
        +轉移到 LoadingScene 或 GameOverScene
    }

    class GameOverSceneHandler["GameOverSceneHandler"] {
        +顯示 GAME OVER
        +等待重新開始  
    }

    class ESCMenuSceneHandler["ESCMenuSceneHandler"] {
        +暫停選單
        +選項轉移
    }

    %% Manager Pattern
    class SceneManager["SceneManager (服務)"] {
        -stack~ISceneHandler~ m_SceneStack
        +PushScene(name)
        +PopScene()
        +ReplaceScene(name)
        +Update() bool
    }

    class UIManager["UIManager (服務)"] {
        -GameStateManager* m_GameState
        +RenderHUD()
        +RenderDeathOverlay()
        +AddFloatingText(x, y, text)
    }

    class FloatingText["FloatingText (特效)"] {
        -float m_FloatDistance
        -int m_LifetimeCounter
        +Update()
        +Draw()
    }

    class AudioManager["AudioManager (服務)"] {
        -float m_Volume
        -bool m_Muted
        +PlayBGM(name)
        +PlaySFX(name)
        +SetVolume(v) / SetMuted(b)
    }

    class IAudioService["IAudioService (介面)"] {
        +PlayBGM(name)*
        +PlaySFX(name)*
        +SetVolume(v)*
    }

    class GameTheater["GameTheater (協調器)"] {
        -SceneManager m_SceneManager
        +Initialize(startScene)
        +Update() bool
        +TransitionToScene(name)
    }

    class ServiceLocator["ServiceLocator (DI)"] {
        -map~type, service~
        +RegisterService(T, service)
        +GetService(T) service
        +HasService(T) bool
    }

    class EventSystem["EventSystem~T~ (通用)"] {
        -map~id, callback~ m_Listeners
        +Subscribe(cb) id
        +Unsubscribe(id)
        +Publish(event)
    }

    %% 繼承與實裝
    ISceneHandler <|.. TitleSceneHandler
    ISceneHandler <|.. LoadingSceneHandler
    ISceneHandler <|.. DeathSceneHandler
    ISceneHandler <|.. GameOverSceneHandler
    ISceneHandler <|.. ESCMenuSceneHandler

    IAudioService <|.. AudioManager

    UIManager o-- FloatingText : 創建與管理
    SceneManager o-- ISceneHandler : 管理場景堆
    GameTheater o-- SceneManager : 使用場景堆
    ServiceLocator ..> AudioManager : 提供服務
    ServiceLocator ..> EventSystem : 提供服務
```

### 12.3 Phase 5 設計特色

#### 12.3.1 遊戲流程狀態機

```
[初始化] 
    ↓
[Title Scene] --按 Enter--> [Loading Scene] --自動--> [Playing Scene]
                                                           ↓
                                                    [ESC 暫停選單]
                                                           ↓
[Game Over Scene] <--生命耗盡-- [Death Scene] <--Mario 死亡-- [Playing Scene]
                                                           ↓
                                                    [Loading Scene] --自動--> [Playing Scene] (下一關)
```

#### 12.3.2 服務定位器模式優勢

- **DI 容器**：替代全域 Singleton，更便於測試
- **單責原則**：每個服務各司其職
- **易於擴充**：新增服務只需註冊，無須修改現有程式碼

#### 12.3.3 事件系統應用

```cpp
// 訂閱事件
EventSystem<PlayerDeadEvent> events;
events.Subscribe([](const PlayerDeadEvent& e) {
    std::cout << "Player died at: " << e.x << ", " << e.y << std::endl;
});

// 發佈事件
PlayerDeadEvent evt{mario.GetX(), mario.GetY()};
events.Publish(evt);
```

---

## 十三、層級控制 (Z-Index 渲染順序)

| Z-Index | 層級 | 內容 |
|---------|------|------|
| 90+ | UI 覆蓋層 | HUD、暫停選單、死亡畫面 |
| 10 | 前景層 | 浮動分數文字 |
| 5 | 實體層 | 敵人、道具、投射物 |
| 0 | 玩家層 | Player 精靈 |
| -5 | 方塊層 | 地面、磚塊、問號方塊 |
| -10 | 背景層 | 背景色、裝飾方塊 |

---

## 十三、專案目錄結構 (Modular Architecture)

```
NTUT_OOPL_mario_V2/
├── include/
│   ├── App.hpp                          # 主控制器
│   └── Mario/
│       ├── Behaviors/                   # Strategy 行為模式
│       │   ├── IEntityBehavior.hpp      #   介面
│       │   ├── DefaultEntityBehavior.hpp
│       │   ├── EnemyBehavior.hpp        #   Goomba / Koopa / KoopaShell
│       │   ├── BowserBehavior.hpp       #   Bowser AI
│       │   ├── ItemBehavior.hpp         #   Item / Coin / CoinGet
│       │   └── FireballBehavior.hpp     #   Fireball / BowserFire
│       ├── AppCore.hpp                  # 應用核心生命週期
│       ├── Block.hpp / BlockState.hpp   # 方塊 View + Model
│       ├── Camera.hpp                   # 攝影機
│       ├── Collider.hpp                 # AABB 碰撞箱
│       ├── CollisionContext.hpp         # 碰撞上下文
│       ├── CollisionManager.hpp         # 碰撞管理器
│       ├── CollisionStrategy.hpp        # 碰撞策略介面 + 實作
│       ├── CollisionStrategyFactory.hpp # 碰撞策略工廠
│       ├── DeathScreenManager.hpp       # 死亡畫面
│       ├── DeathSceneHandler.hpp        # 死亡場景 handler
│       ├── EndingSequenceController.hpp # 結局流程控制器
│       ├── Entity.hpp / EntityModel.hpp # 實體 View + Model
│       ├── EntityDef.hpp                # 實體定義結構
│       ├── EntityFactory.hpp            # 實體工廠
│       ├── ESCMenuState.hpp             # ESC 選單 Model
│       ├── ESCMenuSceneHandler.hpp      # ESC 選單場景 handler
│       ├── EventSystem.hpp              # 事件系統
│       ├── FloatingText.hpp             # 浮動文字
│       ├── GameConfig.hpp               # 全域配置
│       ├── GameOverSceneHandler.hpp     # GameOver 場景 handler
│       ├── GameStateManager.hpp         # 遊戲狀態管理 (Model)
│       ├── GameTheater.hpp              # 場景堆管理器 (State Pattern)
│       ├── IAudioService.hpp            # 音訊服務介面 (DIP)
│       ├── IGameState.hpp               # 遊戲狀態介面
│       ├── ISceneHandler.hpp            # 場景處理器介面 (Strategy)
│       ├── InputDispatcher.hpp          # 輸入分派
│       ├── InputHandler.hpp             # 輸入處理
│       ├── Level.hpp                    # 關卡資料與實體管理
│       ├── LevelManager.hpp             # 多關卡載入切換
│       ├── LoadingSceneHandler.hpp      # 載入場景 handler
│       ├── MovingPlatform.hpp           # 移動平台 (繼承 Block)
│       ├── PhysicsConstants.hpp         # 物理常量
│       ├── PhysicsCoordinator.hpp       # 物理協調器
│       ├── PhysicsEngine.hpp            # 物理引擎
│       ├── PipeWarpController.hpp       # 水管傳送控制器
│       ├── Player.hpp / PlayerState.hpp # 玩家 View + Model
│       ├── PlayingSceneController.hpp   # 遊玩場景控制器
│       ├── RenderManager.hpp            # 渲染管理器
│       ├── SceneManager.hpp             # 場景切換
│       ├── ServiceLocator.hpp           # 服務定位器 (DI)
│       ├── SpritePathResolver.hpp       # 精靈路徑解析
│       ├── TitleSceneHandler.hpp        # 標題場景 handler
│       ├── AudioManager.hpp             # 音訊管理器
│       ├── UIManager.hpp                # HUD 管理器
│       └── UIOrchestrator.hpp           # UI 協調器
├── src/
│   ├── main.cpp                         # 程式進入點
│   ├── App.cpp                          # 主控制器實作
│   └── Mario/
│       ├── Behaviors/                   # 行為策略實作
│       ├── *.cpp                        # 各類別對應實作檔
│       └── ...
├── test/
│   ├── ut_player.cpp                    # Player / PlayerState 測試
│   ├── ut_entity.cpp                    # Entity 測試
│   ├── ut_block.cpp                     # Block 測試
│   ├── ut_collider.cpp                  # AABB 碰撞測試
│   ├── ut_collision_integration.cpp     # 碰撞整合測試
│   ├── ut_event_system.cpp              # 事件系統測試
│   ├── ut_game_state.cpp                # GameState 測試
│   ├── ut_game_state_integration.cpp    # GameState 整合測試
│   ├── ut_input_handler.cpp             # InputHandler 測試
│   ├── ut_level.cpp / ut_level_loading.cpp # Level 測試
│   ├── ut_esc_menu.cpp                  # ESC 選單測試
│   ├── ut_managers.cpp                  # 管理器測試
│   ├── ut_moving_platform.cpp           # 移動平台測試
│   ├── ut_playing_scene_controller.cpp  # 遊玩場景控制器測試
│   ├── ut_ending_sequence_controller.cpp # 結局控制器測試
│   ├── ut_pipe_warp_controller.cpp      # 水管傳送測試
│   ├── ut_sprite_level_validation.cpp   # 精靈與關卡驗證
│   └── ut_misc.cpp                      # 雜項測試
└── files.cmake                          # CMake 檔案清單
```

---

## 十四、檔案清單 (File Manifest)

### include/ (48+ 個標頭檔)

| 檔案路徑 | 類別 | 架構層 |
|----------|------|--------|
| `App.hpp` | `App` | Controller |
| `Mario/AppCore.hpp` | `AppCore` | Controller |
| `Mario/Entity.hpp` | `Entity` | View |
| `Mario/EntityModel.hpp` | `EntityModel` | Model |
| `Mario/EntityDef.hpp` | `EntityDef` | Data |
| `Mario/EntityFactory.hpp` | `EntityFactory` | Infrastructure |
| `Mario/Player.hpp` | `Player` | View |
| `Mario/PlayerState.hpp` | `PlayerState` | Model |
| `Mario/Block.hpp` | `Block` | View |
| `Mario/BlockState.hpp` | `BlockState` | Model |
| `Mario/MovingPlatform.hpp` | `MovingPlatform` | View |
| `Mario/Camera.hpp` | `Camera` | View |
| `Mario/Collider.hpp` | `AABB` | Data |
| `Mario/CollisionContext.hpp` | `CollisionContext` | Data |
| `Mario/CollisionManager.hpp` | `CollisionManager` | Controller |
| `Mario/CollisionStrategy.hpp` | `ICollisionStrategy` + 3 實作 | Controller |
| `Mario/CollisionStrategyFactory.hpp` | `CollisionStrategyFactory` | Infrastructure |
| `Mario/PhysicsConstants.hpp` | 物理常量 | Data |
| `Mario/PhysicsEngine.hpp` | `PhysicsEngine` | Controller |
| `Mario/PhysicsCoordinator.hpp` | `PhysicsCoordinator` | Controller |
| `Mario/GameConfig.hpp` | `GameConfig` | Configuration |
| `Mario/GameStateManager.hpp` | `GameStateManager` | Model |
| `Mario/GameTheater.hpp` | `GameTheater` + `Scene` | Controller |
| `Mario/SceneManager.hpp` | `SceneManager` | Controller |
| `Mario/ISceneHandler.hpp` | `ISceneHandler` | Controller (Interface) |
| `Mario/TitleSceneHandler.hpp` | `TitleSceneHandler` | Controller |
| `Mario/LoadingSceneHandler.hpp` | `LoadingSceneHandler` | Controller |
| `Mario/GameOverSceneHandler.hpp` | `GameOverSceneHandler` | Controller |
| `Mario/DeathSceneHandler.hpp` | `DeathSceneHandler` | Controller |
| `Mario/ESCMenuSceneHandler.hpp` | `ESCMenuSceneHandler` | Controller |
| `Mario/PlayingSceneController.hpp` | `PlayingSceneController` | Controller |
| `Mario/EndingSequenceController.hpp` | `EndingSequenceController` | Controller |
| `Mario/PipeWarpController.hpp` | `PipeWarpController` | Controller |
| `Mario/ESCMenuState.hpp` | `ESCMenuState` | Model |
| `Mario/Level.hpp` | `Level` | Model |
| `Mario/LevelManager.hpp` | `LevelManager` | Controller |
| `Mario/InputHandler.hpp` | `InputHandler` | Controller |
| `Mario/InputDispatcher.hpp` | `InputDispatcher` | Controller |
| `Mario/EventSystem.hpp` | `EventSystem` | Infrastructure |
| `Mario/IAudioService.hpp` | `IAudioService` | Infrastructure (Interface) |
| `Mario/AudioManager.hpp` | `AudioManager` | Infrastructure |
| `Mario/ServiceLocator.hpp` | `ServiceLocator` | Infrastructure |
| `Mario/UIManager.hpp` | `UIManager` | View |
| `Mario/UIOrchestrator.hpp` | `UIOrchestrator` | Controller |
| `Mario/RenderManager.hpp` | `RenderManager` | View |
| `Mario/FloatingText.hpp` | `FloatingText` | View |
| `Mario/DeathScreenManager.hpp` | `DeathScreenManager` | View |
| `Mario/SpritePathResolver.hpp` | `SpritePathResolver` | Infrastructure |
| `Mario/IGameState.hpp` | `IGameState` | Model (Interface) |
| `Mario/Behaviors/IEntityBehavior.hpp` | `IEntityBehavior` | Strategy (Interface) |
| `Mario/Behaviors/DefaultEntityBehavior.hpp` | `DefaultEntityBehavior` | Strategy |
| `Mario/Behaviors/EnemyBehavior.hpp` | `Goomba/Koopa/KoopaShellBehavior` | Strategy |
| `Mario/Behaviors/BowserBehavior.hpp` | `BowserBehavior` | Strategy |
| `Mario/Behaviors/ItemBehavior.hpp` | `Item/Coin/CoinGetBehavior` | Strategy |
| `Mario/Behaviors/FireballBehavior.hpp` | `Fireball/BowserFireBehavior` | Strategy |

### src/ (43+ 個原始檔)

| 檔案路徑 | 對應類別 | 說明 |
|----------|---------|------|
| `main.cpp` | — | 程式進入點，驅動 `App::Start/Update/End` |
| `App.cpp` | `App` | 主迴圈、場景切換、初始化 |
| `Mario/AppCore.cpp` | `AppCore` | 應用核心生命週期 |
| `Mario/Entity.cpp` | `Entity` | 實體渲染、行為委派、碰撞 |
| `Mario/EntityModel.cpp` | `EntityModel` | 實體模型初始化、物理邏輯 |
| `Mario/EntityFactory.cpp` | `EntityFactory` | 實體工廠建立 |
| `Mario/Player.cpp` | `Player` | 玩家渲染、精靈快取 |
| `Mario/PlayerState.cpp` | `PlayerState` | 玩家邏輯（60+ 測試覆蓋） |
| `Mario/Block.cpp` | `Block` | 方塊渲染、彈跳動畫 |
| `Mario/BlockState.cpp` | `BlockState` | 方塊邏輯 |
| `Mario/MovingPlatform.cpp` | `MovingPlatform` | 正弦振盪平台 |
| `Mario/Camera.cpp` | `Camera` | 攝影機跟隨 |
| `Mario/CollisionManager.cpp` | `CollisionManager` | 所有碰撞偵測邏輯 |
| `Mario/CollisionStrategy.cpp` | 碰撞策略實作 | Goal, Container, Breakable |
| `Mario/CollisionStrategyFactory.cpp` | 碰撞策略工廠 | 根據方塊類型建立策略 |
| `Mario/PhysicsEngine.cpp` | `PhysicsEngine` | 重力、跳躍曲線 |
| `Mario/PhysicsCoordinator.cpp` | `PhysicsCoordinator` | 物理協調 |
| `Mario/GameConfig.cpp` | `GameConfig` | 配置初始化 |
| `Mario/GameStateManager.cpp` | `GameStateManager` | 遊戲狀態管理 |
| `Mario/GameTheater.cpp` | `GameTheater` | 場景堆管理 |
| `Mario/SceneManager.cpp` | `SceneManager` | 場景切換 |
| `Mario/PlayingSceneController.cpp` | `PlayingSceneController` | 遊玩場景控制器 |
| `Mario/EndingSequenceController.cpp` | `EndingSequenceController` | 結局控制器 |
| `Mario/PipeWarpController.cpp` | `PipeWarpController` | 水管傳送控制器 |
| `Mario/TitleSceneHandler.cpp` | `TitleSceneHandler` | 標題場景 |
| `Mario/LoadingSceneHandler.cpp` | `LoadingSceneHandler` | 載入場景 |
| `Mario/GameOverSceneHandler.cpp` | `GameOverSceneHandler` | GameOver 場景 |
| `Mario/DeathSceneHandler.cpp` | `DeathSceneHandler` | 死亡場景 |
| `Mario/ESCMenuSceneHandler.cpp` | `ESCMenuSceneHandler` | ESC 選單場景 |
| `Mario/ESCMenuState.cpp` | `ESCMenuState` | ESC 選單邏輯 |
| `Mario/Level.cpp` | `Level` | 關卡載入、實體管理 |
| `Mario/LevelManager.cpp` | `LevelManager` | 多關卡切換 |
| `Mario/InputHandler.cpp` | `InputHandler` | 輸入處理 |
| `Mario/InputDispatcher.cpp` | `InputDispatcher` | 輸入分派 |
| `Mario/EventSystem.cpp` | `EventSystem` | 事件發布/訂閱 |
| `Mario/AudioManager.cpp` | `AudioManager` | 音效播放 |
| `Mario/ServiceLocator.cpp` | `ServiceLocator` | 服務定位器 |
| `Mario/UIManager.cpp` | `UIManager` | HUD 渲染 |
| `Mario/UIOrchestrator.cpp` | `UIOrchestrator` | UI 協調 |
| `Mario/RenderManager.cpp` | `RenderManager` | 渲染管理 |
| `Mario/FloatingText.cpp` | `FloatingText` | 浮動文字 |
| `Mario/DeathScreenManager.cpp` | `DeathScreenManager` | 死亡畫面 |

### test/ (19 個測試檔)

| 檔案路徑 | 測試範圍 |
|----------|---------|
| `ut_player.cpp` | `Player` / `PlayerState` 移動、跳躍、變身、碰撞箱 |
| `ut_entity.cpp` | `Entity` / `EntityModel` 建立、行為、狀態 |
| `ut_block.cpp` | `Block` 屬性、彈跳、破壞 |
| `ut_collider.cpp` | `AABB` 碰撞偵測 |
| `ut_collision_integration.cpp` | 碰撞整合場景 |
| `ut_event_system.cpp` | 事件發布/訂閱 |
| `ut_game_state.cpp` | `GameStateManager` 分數/生命/關卡 |
| `ut_game_state_integration.cpp` | 遊戲狀態整合 |
| `ut_input_handler.cpp` | `InputHandler` 輸入對映 |
| `ut_level.cpp` | `Level` 載入 |
| `ut_level_loading.cpp` | 關卡解析驗證 |
| `ut_esc_menu.cpp` | `ESCMenuState` 選單邏輯 |
| `ut_managers.cpp` | 管理器整合 |
| `ut_moving_platform.cpp` | `MovingPlatform` 振盪物理 |
| `ut_playing_scene_controller.cpp` | 遊玩場景控制 |
| `ut_ending_sequence_controller.cpp` | 結局流程測試 |
| `ut_pipe_warp_controller.cpp` | 水管傳送測試 |
| `ut_sprite_level_validation.cpp` | 精靈路徑與關卡驗證 |
| `ut_misc.cpp` | 雜項邊界測試 |
