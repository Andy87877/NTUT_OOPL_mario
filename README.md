# NTUT OOPL Mario Project

遊戲名稱：Super Mario 

組員：

- 113820033 謝奕宏

# Game Introduction

本專案旨在復刻經典 2D 橫捲軸動作遊戲《Super Mario Bros.》。

- 類型：2D 橫捲軸動作遊戲
- 操作：控制主角 `Mario` 跑、跳、踩敵、收集金幣與道具
- 目標：抵達關卡旗杆完成關卡
- 參考畫面：[遊戲畫面連結](https://www.youtube.com/watch?v=rLl9XBg7wSs)

實作功能：
1. **角色移動**：實作 Mario 的左右移動與跳躍功能，包含基本的物理模擬（重力、跳躍拋物線）。
2. **敵人互動**：實作基本敵人（如 Goomba）的巡邏行為與被踩踏的死亡機制。
3. **道具系統**：實作蘑菇與金幣的收集機制，蘑菇可使 Mario 變大提升能力。
4. **關卡設計**：實作多組關卡配置，包含平台、坑洞與敵人分布，並設計終點旗杆。
5. **UI 顯示**：實作基本的 HUD 顯示金幣數量、生命值與分數，提供玩家即時資訊。

# Development timeline

> timeline進度為參考用，實際上會提早完成

- Week 2：環境建置與核心類別與收集素材
    - [x] 建立專案目錄與設定基礎框架
    - [x] 實作基礎物件類別與資料型別
    - [x] 收集並整理遊戲素材（圖像、音效、關卡設計）

- Week 3：場景與基礎渲染
    - [x] 實作場景狀態機與切換管理
    - [x] 建立與渲染基礎遊戲畫面

> demo mario to teacher and TA

- Week 4：角色移動與輸入
    - [ ] 實作主角類別與按鍵輸入映射
    - [ ] 處理角色左右移動與狀態管理

- Week 5：重力與跳躍系統
    - [ ] 實作物理引擎與重力加速度
    - [ ] 處理跳躍拋物線與地面偵測

- Week 6：地圖系統建置
    - [ ] 實作 Tile 類別與工廠模式
    - [ ] 建立關卡載入器與地圖渲染

- Week 7：碰撞偵測與攝影機
    - [ ] 實作 AABB 碰撞偵測與回應機制
    - [ ] 實作攝影機跟隨與邊界限制

- Week 8：期中展示與整理
    - [ ] 確保場景移動與碰撞功能正常
    - [ ] 整合 HUD 顯示系統並準備展示

- Week 9：基礎敵人實作 (Goomba)
    - [ ] 實作敵人基底類別與巡邏 AI
    - [ ] 處理踩踏判定與受傷機制

- Week 10：進階敵人實作 (Koopa)
    - [ ] 實作 Koopa 巡邏與躲藏邏輯
    - [ ] 處理龜殼推行與碰撞其他敵人

- Week 11：道具收集系統
    - [ ] 實作道具類別 (蘑菇、星星) 與效果
    - [ ] 實作金幣收集與計算機制

- Week 12：狀態機與過關機制
    - [ ] 實作角色變身狀態機 (小/大/無敵)
    - [ ] 處理關卡終點旗杆接觸與切換

- Week 13：核心體驗打磨
    - [ ] 實作受傷無敵與生命值結算
    - [ ] 優化奔跑、慣性及進階跳躍手感

- Week 14：關卡擴充設計
    - [ ] 重新設計並擴充多組關卡配置
    - [ ] 加入問號方塊、水管與各式地形變化

- Week 15：視覺與動畫優化
    - [ ] 實作角色與敵人各狀態動畫切換
    - [ ] 加入分數浮動與物件跳動視覺效果

- Week 16：進階功能與全面測試
    - [ ] 實作進階道具 (火焰花、1UP)
    - [ ] 進行全關卡遊玩測試與邊界修復

- Week 17：期末發表與繳交
    - [ ] 整合背景音樂與各式特效音
    - [ ] 拍攝遊玩影片、準備報告並提交

## 還原關卡(不會1:1完整復刻，會盡力復刻關卡內容與細節)

1-1 ![1-1](Resources/map_reference/1-1.png)
1-2 ![1-2](Resources/map_reference/1-2.png)
8-4 ![8-4](Resources/map_reference/8-4.png)

## My OOPL_mario proposal
https://github.com/Okapi-Oriented-Programming/2026-OOPL/blob/main/Proposal/113820033/Proposal.md

<!-- # 程式物件架構 (重構中 先註解掉)
```
SuperMarioBros
├── App                        # 主迴圈，協調所有子系統
│
├── Controller 層
│   ├── SceneManager           # 場景狀態機（Title/Playing/GameOver…）
│   └── InputHandler           # 鍵盤輸入封裝
│
├── View 層（繼承 Util::GameObject）
│   ├── Player                 # 玩家精靈渲染與動畫
│   ├── Entity                 # 敵人 / 道具 / 投射物 / 特效
│   ├── Block                  # 方塊精靈渲染與彈跳動畫
│   │   └── MovingPlatform     # 移動平台（繼承 Block）
│   ├── Camera                 # 畫面捲動
│   ├── RenderManager          # 統一管理可渲染物件
│   ├── UIManager              # HUD（分數 / 金幣 / 生命 / 時間）
│   ├── FloatingText           # 分數浮動數字
│   ├── DeathScreenManager     # 死亡畫面
│   └── ESCMenuState           # 暫停選單
│
├── Model 層（純邏輯，可單元測試）
│   ├── PlayerState            # 玩家速度、變身狀態、碰撞箱
│   ├── EntityState            # 實體方向、死亡狀態
│   ├── BlockState             # 方塊撞擊與內容物釋放
│   ├── GameStateManager       # 分數 / 生命 / 金幣 / 計時 / 關卡進度
│   ├── PhysicsEngine          # 重力、跳躍拋物線
│   ├── CollisionManager       # 玩家↔方塊、玩家↔實體碰撞偵測
│   ├── Level                  # 從 CSV 解析關卡方塊與實體
│   └── LevelManager           # 多關卡與地下室切換
│
└── 基礎設施層
    ├── EntityFactory          # 工廠模式，建立敵人/道具/特效
    ├── EventSystem            # 發布/訂閱（18 種事件）
    ├── AudioManager           # 音樂與音效播放
    └── SpritePathResolver     # 精靈圖路徑解析
``` -->

## 參考資料：
- [PTSD Template](https://github.com/ntut-open-source-club/practical-tools-for-simple-design)
- [Google Test Documentation](https://google.github.io/googletest/)

- [Super Mario Bros. C# Remake](https://github.com/Jack-Development/SuperMarioBros-CSharp-Remake)
- [Super Mario Bros. Maps](https://www.mariowiki.com/Category:Super_Mario_Bros._maps)
- [Super Mario Bros. Tileset](https://www.spriters-resource.com/wii_u/supermariomaker/asset/69702/)
- [Super Mario Bros. Items](https://www.spriters-resource.com/wii_u/supermariomaker/asset/69701/)

~~~
~~~

# PTSD Template

This is a [PTSD](https://github.com/ntut-open-source-club/practical-tools-for-simple-design) framework template for students taking OOPL2024s.

## Quick Start

1. Use this template to create a new repository
   ![github screenshot](https://github.com/ntut-rick/ptsd-template/assets/126899559/ef62242f-03ed-481d-b858-12b730c09beb)

2. Clone your repository

   ```bash
   git clone YOUR_GIT_URL --recursive
   ```

3. Build your project

  > [!WARNING]
  > Please build your project in `Debug` because our `Release` path is broken D:
   
   ```sh
   cmake -DCMAKE_BUILD_TYPE=Debug -B build # -G Ninja
   ```
   better read [PTSD README](https://github.com/ntut-open-source-club/practical-tools-for-simple-design)