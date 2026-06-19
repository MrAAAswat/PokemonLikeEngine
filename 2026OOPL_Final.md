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

This project is a 2D Pokémon‑style open‑world RPG built on the **PTSD (Practical Tools for Simple Design)** framework provided in the OOPL course. Players can freely explore a multi‑map world, capture various Pokémon, and challenge multiple Bosses in turn‑based battles. The map consists of interconnected towns and routes, each town featuring a BOSS battle (not always a gym), a Pokémon Center (healing), a shop system, and NPCs with potential quests. Routes include forests and ordinary paths, offering diverse terrain and scenery.

The development cycle covered every major game‑development phase: character sprite design, tilemap creation, collision detection, NPC interaction systems, battle mechanics. The project is written primarily in **C++  with supporting CSS, CMake, and JavaScript files and JSON files for databases.

![start](https://raw.githubusercontent.com/UncleAmra/113590030--110590042/main/ScreenShots/CollectStarter.png)

### 組別分工

| Member | Responsibilities |
| --- | --- |
| **Abdulahad Aswat** | Bug fixing for encounter logic; map development and expansion; battle system implementation; inventory UI |
| **G.Amarjargal Eddie** | Overall architecture design; map system improvements |

---

## 遊戲介紹

### 遊戲規則

1. **Exploration & Movement**
Use the arrow keys to move your character in four directions on a grid‑based map. The map includes collision detection – you cannot walk through water tiles, walls, buildings, or other NPCs. You can interact with NPCs by pressing the action key 'Z'. To view the start menu the player can press 'I' when in the overworld, these commands do not work in battle. The start menu allows the player to view their data and access other UI elements such as their inventory, pokemon and to save their game. 

3. **Pokémon Collection**
Encounter wild Pokémon in tall grass and capture them to add to your party.
4. **Battle System**
* Turn‑based combat – player and enemy alternate actions.
* Includes a **move/skill system** with damage calculation formulas.
* Supports **type effectiveness** (super‑effective, not very effective) – each Pokémon has a type and each move has a type.
* After battle, your Pokémon gain **experience points** and can **level up**.


4. **Boss Battles**
Each town has a Gym with a Leader (BOSS). Bosses have specific AI priority logic. Defeating a Gym Leader rewards the player with a **Badge** or a special key item.
5. **Item System**
An in‑game **inventory** holds potions, Poké Balls, key items, and more. Items can be picked up from the map or obtained after battles.

### 遊戲畫面

####  Overworld Exploration & Battle Scene

![Overworld](https://raw.githubusercontent.com/UncleAmra/113590030--110590042/main/ScreenShots/NTUTmap.png)

#### Inventory Menu & Shop Menu

![store](https://raw.githubusercontent.com/UncleAmra/113590030--110590042/main/ScreenShots/Store.png)

![inventory](https://raw.githubusercontent.com/UncleAmra/113590030--110590042/main/ScreenShots/Inventory.png)

#### Gym Leader Battle & Pokémon Party

![battlesystem](https://raw.githubusercontent.com/UncleAmra/113590030--110590042/main/ScreenShots/BattleSystem.png)

![party](https://raw.githubusercontent.com/UncleAmra/113590030--110590042/main/ScreenShots/PokemonTeam.png)


---

## 程式設計

### 程式架構

The project is built on the **PTSD** framework, which provides a lightweight game loop, input handling, rendering, and audio. We extended it with our own game‑specific subsystems.

The overall directory structure is:

```text
├── PTSD/                # Core PTSD framework (provided by course)
├── include/             # Header files (*.hpp)
│   ├── App.hpp          # Main application loop and scene state manager
│   ├── Character.hpp    # Base class for moving entities (Player, NPCs)
│   ├── Player.hpp       # Player logic, input mapping, party, and inventory
│   ├── NPC.hpp          # Non-player characters with dialogue trees
│   ├── Map.hpp          # Map loading, grid parsing, and layer rendering
│   ├── Pokemon.hpp      # Pokémon stats, types, moves, and logic systems
│   ├── BattleManager.hpp# Battle system finite state machine (FSM)
│   ├── *Menu.hpp        # UI windows (InventoryMenu, ShopMenu, StartMenu)
│   └── *Database.hpp    # Singletons for game databases (Item, Move, Trainer)
├── src/                 # Implementation source files (*.cpp)
│   ├── main.cpp         # Game entry point
│   └── [Matching .cpp files for the headers listed above]
├── Resources/           # Game assets and configuration data
│   ├── data/            # JSON data files (items, moves, encounters)
│   ├── maps/            # CSV tile maps and map grid structural files
│   ├── Pokémon/         # Front and back combat sprites for pocket monsters
│   ├── UI/ & Fonts/     # Visual menus, text assets, and HUD components
│   └── [dialogue/, items/, npcs/, player/, tiles/, trainers.JSON]
├── ScreenShots/         # Project report screenshots (ignored by build)
├── CMakeLists.txt       # Main build configuration script
├── files.cmake          # Supplementary build list of source compilation units
└── savegame.json        # Persistent global game save file slot

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
* * `pkmn_animations` – all the data related to the frames of attack animations with positioning and images and timing.
* `encounters.json` – maps locations to encounterable Pokémon with spawn weights.
* `npcs.json` – defines NPC dialogue trees, shop inventories, and action triggers (Heal, Warp, Battle, etc.).
* `props.json` - defines all the properties of props such as textures, visual offsets and if they animate (Always, When Stepped on, never)

![data](https://raw.githubusercontent.com/UncleAmra/113590030--110590042/main/ScreenShots/NPP&PropsDatabase.png)


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

![startTownCSV](https://raw.githubusercontent.com/UncleAmra/113590030--110590042/main/ScreenShots/startTownCSV.png)

![startTown](https://raw.githubusercontent.com/UncleAmra/113590030--110590042/main/ScreenShots/startTownWideShot.png)

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
* **Report spell checking and corrections.

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
