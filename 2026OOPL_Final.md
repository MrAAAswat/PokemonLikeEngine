# 2026 OOPL Final Report

## 組別資訊

* **組別**：Group 6
* **組員**：
* Abdulahad Aswat (MrAAAswat)
* G.Amarjargal Eddie (UncleAmra)


* **復刻遊戲**：Pokémon – 2D Open‑World RPG

---

## 專案簡介

### 遊戲簡介

This project is a 2D Pokémon‑style open‑world RPG built on the **PTSD (Practical Tools for Simple Design)** framework provided in the OOPL course. Players can freely explore a multi‑region world, capture various Pokémon, and challenge multiple Gym Leaders in turn‑based battles. The map consists of interconnected towns and routes, each town featuring a Gym (BOSS battle), a Pokémon Center (healing), a shop system, and NPCs with potential quests. Routes include forests, caves, and ordinary paths, offering diverse terrain and scenery.

The development cycle spanned 17 weeks, covering every major game‑development phase: character sprite design, tilemap creation, collision detection, NPC interaction systems, battle mechanics, and audio integration. The project is written primarily in **C++ (78.6%)**, with supporting CSS, CMake, and JavaScript files.

### 組別分工

| Member | Responsibilities |
| --- | --- |
| **Abdulahad Aswat** | Bug fixing for encounter logic; map development and expansion; battle system implementation; inventory UI |
| **G.Amarjargal Eddie** | Overall architecture design; map system improvements |

---

## 遊戲介紹

### 遊戲規則

1. **Exploration & Movement**
Use the arrow keys to move your character in four directions on a grid‑based map. The map includes collision detection – you cannot walk through water tiles, walls, buildings, or other NPCs. You can interact with NPCs by pressing the action key.
2. **Pokémon Collection**
Encounter wild Pokémon in tall grass or caves and capture them to add to your party.
3. **Battle System**
* Turn‑based combat – player and enemy alternate actions.
* Includes a **move/skill system** with damage calculation formulas.
* Supports **type effectiveness** (super‑effective, not very effective) – each Pokémon has a type and each move has a type.
* After battle, your Pokémon gain **experience points** and can **level up**.


4. **Gym Leader Battles**
Each town has a Gym with a Leader (BOSS). Bosses have specific AI priority logic. Defeating a Gym Leader rewards the player with a **Badge** or a special key item.
5. **Item System**
An in‑game **inventory** holds potions, Poké Balls, key items, and more. Items can be picked up from the map or obtained after battles.

### 遊戲畫面

#### 🗺️ Overworld Exploration & Battle Scene

#### 🎒 Inventory Menu & Shop Menu

#### 🏆 Gym Leader Battle & Pokémon Party

---

## 程式設計

### 程式架構

The project is built on the **PTSD** framework, which provides a lightweight game loop, input handling, rendering, and audio. We extended it with our own game‑specific subsystems.

The overall directory structure is:

```text
├── PTSD/                # Core PTSD framework (provided by course)
├── src/                 # Game source code
│   ├── App.cpp/hpp      # Main application: initialises game, runs loop, handles scenes
│   ├── Character.cpp/hpp# Base class for all moving entities (player, NPCs)
│   ├── Player.cpp/hpp   # Player-specific logic (input, party, inventory)
│   ├── NPC.cpp/hpp      # Non-player characters with dialogue and AI behaviours
│   ├── Map.cpp/hpp      # Map loading, rendering, collision, and prop interaction
│   ├── Pokemon/         # Pokémon data, battle, and party management
│   ├── Battle/          # Battle state machine, AI, and UI
│   ├── UI/              # Menus (inventory, shop, start screen)
│   └── Data/            # JSON database classes (PokemonDatabase, ItemDatabase, etc.)
├── assets/              # Sprites, tilesets, audio, and fonts
├── data/                # JSON configuration files (items, pokemon, moves, encounters, npcs)
└── ScreenShots/         # Report screenshots (not part of the build)

```

**Core class relationships**:

* `App` owns the `GameState`, which contains the current `Player`, `MapManager`, `BattleManager`, and `Inventory`.
* The `MapManager` loads CSV tile data and manages collision via a 2D grid of `Tile` objects (each with walkable flag and prop ID).
* `BattleManager` is a state machine that takes control of the game loop during battles; it uses `Pokemon` instances and a `Trainer` (enemy) with AI logic.
* All JSON data is loaded into singleton `Database` classes (e.g., `PokemonDatabase::getInstance()`) for global access.

We chose this architecture to **separate concerns**: rendering, game logic, and data are decoupled, making it easier to debug and extend.

### 程式技術

#### 1. Data‑Driven Architecture

We decoupled game content from hardcoded logic by using **JSON** as our primary data format. This allows rapid iteration without recompilation.

* **JSON Data Stores**:
* `items.json` – item metadata, categories, buy/sell prices, effects.
* `pokemon.json` / `moves.json` – base stats, types, growth rates, move power/accuracy.
* `encounters.json` – maps locations to encounterable Pokémon with spawn weights.
* `npcs.json` – defines NPC dialogue trees, shop inventories, and action triggers (Heal, Warp, Battle, etc.).



* **Parsing**: We used the **nlohmann/json** library. Each data type is managed by a singleton `Database` class (e.g., `PokemonDatabase`, `ItemDatabase`), providing global access with minimal overhead.

#### 2. Persistent Save System

The save system captures the full “world state”, not just player stats.

* **Serialization**: The `SaveSystem` serializes the current `GameState` into JSON, including:
* Player data (coordinates, facing direction, party, inventory).
* Map context (current map path, collected item IDs).
* Progression flags (e.g., `gym_leader_defeated`, `received_starter`).


* **Robustness**: On loading, `App::InitGameLoad` verifies asset paths and re‑syncs the `MapManager` and `Player` states, ensuring the player does not spawn inside collision tiles.

#### 3. Map System and Collision Detection

We use a **multi‑layered tilemap** approach with CSV layers:

* **Ground Layer** – base terrain (grass, water, floor).
* **Prop / Interactive Layer** – collision data, NPC placements, and lootable items.

Each tile has properties (walkable, animated, interaction ID). Collision is grid‑based: before moving, we check the next tile’s `isWalkable` flag. For complex interactions (e.g., entering a cave), a custom **Prop ID** triggers the appropriate logic.

#### 4. Battle System State Machine

The battle sequence is a **finite state machine (FSM)** separate from the overworld loop.

* **Turn Resolution**: States: `PLAYER_INPUT` → `ANIMATION` → `ENEMY_AI` → `RESOLUTION`.
* **AI Logic**: Uses a priority‑based decision tree evaluating player health and boss move pool.
* **Data‑Driven Encounters**: `HandleOverworldEncounters` performs a weighted roll using `encounters.json` to determine wild Pokémon species and level per route.

#### 5. Audio

Background music (BGM) for towns, routes, and battles, plus sound effects (SFX) – integrated via PTSD’s audio module.

#### 6. Build & Compilation

Uses **CMake** for cross‑platform building. Developed and tested in **Debug mode** (Release mode paths not fully configured).

### 使用到 AI/AI Agent 的部分

We used AI tools (Claude and Gemini) to assist with:

* **Research**: studying the architecture and mechanics of classic Pokémon games to inform our map and battle system designs.
* **Debugging**: identifying and fixing subtle bugs in collision detection and state transitions.
* **Code generation** (sparingly): generating boilerplate or utility functions when we clearly understood the required structure and the PTSD framework’s constraints. All AI‑generated code was reviewed, tested, and integrated with full awareness of its functionality.

---

## 結語

### 問題與解決方法

| Issue | Solution |
| --- | --- |
| Player getting stuck on map edges due to collision inaccuracies | Implemented precise grid‑based collision and restricted movement to valid tiles |
| Abrupt transitions between overworld and battle scenes | Designed a dedicated `BattleScene` state with smooth fade‑in/out |
| NPC dialogue rendering glitches (text overflow, missing characters) | Utilized PTSD’s built‑in text renderer with proper line‑wrapping and dialog boxes |
| Build failures in Release mode due to incorrect asset paths | Standardized on Debug mode for all development and testing |
| Encounter logic occasionally triggering incorrectly | Abdulahad fixed the encounter‑rate algorithm and debugged edge cases |
| Managing large JSON data and parsing overhead | Used nlohmann/json with singleton Database classes for efficient in‑memory access |
| Save file corruption when loading with different asset versions | Added version checks and asset‑path verification in `InitGameLoad` |

### 自評

| 項次 | 項目 | 完成 |
| --- | --- | --- |
| 1 | 這是範例 | V |
| 2 | 完成專案權限改為 public | V |
| 3 | 具有 debug mode 的功能 | V |
| 4 | 解決專案上所有 Memory Leak 的問題 |  |
| 5 | 報告中沒有任何錯字，以及沒有任何一項遺漏 | V |
| 6 | 報告至少保持基本的美感，人類可讀 | V |

### 心得

This OOPL final project gave us the opportunity to build a complete 2D RPG from the ground up. Over the 17‑week development cycle, we truly internalised the value of **object‑oriented design** – from the `Pokemon` base class with inheritance and polymorphism, to the clean separation of concerns among the battle, map, and inventory modules. Every subsystem had well‑defined responsibilities, which made integration and debugging far more manageable.

We also gained practical experience in **game development workflows**: requirements gathering, asset preparation, core‑mechanic implementation, and final system integration. The battle system, in particular, challenged us to carefully design turn‑based logic, damage formulas, type charts, and status effects – all while keeping the code extensible for future additions.

Equally important was **team collaboration** and **version control**. Using Git, we learned to divide tasks, resolve merge conflicts, and keep the main branch stable. We are proud of what we accomplished and believe the project demonstrates a solid grasp of both C++ and OOP principles.

### 貢獻比例

| Member | Contribution |
| --- | --- |
| Abdulahad Aswat | 60% |
| G.Amarjargal Eddie | 40% |
