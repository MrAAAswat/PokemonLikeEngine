# Pokémon – 2D Open-World RPG

A 2D Pokémon-style open-world RPG built on the **PTSD (Practical Tools for Simple Design)** framework from the OOPL course. Explore a multi-map world, capture Pokémon, and battle Gym Leaders in turn-based combat.

**Members:** Abdulahad Aswat ([MrAAAswat](https://github.com/MrAAAswat)), G.Amarjargal Eddie ([UncleAmra](https://github.com/UncleAmra))

![start](https://raw.githubusercontent.com/UncleAmra/113590030--110590042/main/ScreenShots/CollectStarter.png)

---

## About

Players can freely explore a multi-map world made of interconnected towns and routes. Each town features a BOSS battle (not always a gym), a Pokémon Center for healing, a shop system, and NPCs with dialogue and quests. Routes vary in terrain — forests, ordinary paths, and more — for varied scenery.

The project is written primarily in **C++**, using CMake for builds and JSON for game data (items, moves, encounters, NPCs, props).

![Overworld](https://raw.githubusercontent.com/UncleAmra/113590030--110590042/main/ScreenShots/NTUTmap.png)

## Features

- **Exploration & Movement** — grid-based 4-directional movement with tile collision (walls, water, buildings, NPCs)
- **NPC Interaction** — talk to NPCs with `Z`; open the start menu with `I` (overworld only)
- **Pokémon Collection** — encounter wild Pokémon in tall grass and capture them into your party
- **Turn-Based Battles** — move/skill system with damage formulas and type effectiveness (super-effective / not very effective)
- **Leveling** — Pokémon gain EXP and level up after battle
- **Gym Battles** — each town's Gym Leader has priority-based AI; defeating them rewards a Badge or key item
- **Inventory & Shop** — earn money from battles, buy items from shops/NPCs, and manage potions, Poké Balls, and key items in a categorized inventory
- **Persistent Saves** — full world state is saved, including player position, party, inventory, and progression flags

![battlesystem](https://raw.githubusercontent.com/UncleAmra/113590030--110590042/main/ScreenShots/BattleSystem.png)

![party](https://raw.githubusercontent.com/UncleAmra/113590030--110590042/main/ScreenShots/PokemonTeam.png)

Players earn money through battles, which can be spent at shops or NPC vendors on items to help them on their journey. Items can also be picked up directly from the overworld or received as battle/interaction rewards.

![store](https://raw.githubusercontent.com/UncleAmra/113590030--110590042/main/ScreenShots/Store.png)

Items persist across save files and are viewable in the inventory menu, sorted into three categories: General, Poké Balls, and Key Items.

![inventory](https://raw.githubusercontent.com/UncleAmra/113590030--110590042/main/ScreenShots/Inventory.png)

## Controls

| Key | Action |
| --- | --- |
| Arrow Keys | Move |
| `Z` | Interact with NPCs / confirm |
| `I` | Open start menu (overworld only) |

## Project Structure

```text
├── PTSD/                # Core PTSD framework (provided by course)
├── include/              # Header files (*.hpp)
│   ├── App.hpp           # Main application loop and scene state manager
│   ├── Character.hpp     # Base class for moving entities (Player, NPCs)
│   ├── Player.hpp        # Player logic, input mapping, party, and inventory
│   ├── NPC.hpp            # Non-player characters with dialogue trees
│   ├── Map.hpp            # Map loading, grid parsing, and layer rendering
│   ├── Pokemon.hpp        # Pokémon stats, types, moves, and logic systems
│   ├── BattleManager.hpp  # Battle system finite state machine (FSM)
│   ├── *Menu.hpp          # UI windows (InventoryMenu, ShopMenu, StartMenu)
│   └── *Database.hpp      # Singletons for game databases (Item, Move, Trainer)
├── src/                  # Implementation source files (*.cpp)
│   ├── main.cpp           # Game entry point
│   └── [Matching .cpp files for the headers above]
├── Resources/            # Game assets and configuration data
│   ├── data/               # JSON data files (items, moves, encounters)
│   ├── maps/                # CSV tile maps and map grid structural files
│   ├── Pokémon/             # Front and back combat sprites
│   ├── UI/ & Fonts/         # Menus, text assets, and HUD components
│   └── [dialogue/, items/, npcs/, player/, tiles/, trainers.JSON]
├── ScreenShots/          # Project screenshots
├── CMakeLists.txt        # Main build configuration
├── files.cmake           # Source compilation unit list
└── savegame.json         # Persistent global game save file slot
```

`App` owns the `GameState`, which holds the current `Player`, `MapManager`, `BattleManager`, and `Inventory`. `MapManager` loads CSV tile data and manages collision via a grid of `Tile` objects. `BattleManager` is a state machine that drives combat using `Pokemon` and `Trainer` (enemy AI). All JSON data is loaded into singleton `Database` classes (e.g. `PokemonDatabase::getInstance()`) for global access.

## Map System

Maps use a **multi-layered tilemap** approach authored as CSV files:

- **Ground Layer** — base terrain (grass, water, floor)
- **Prop / Interactive Layer** — collision data, NPC placements, and lootable items

Each tile carries properties (walkable, animated, interaction ID). Before moving, the game checks the next tile's `isWalkable` flag; a **Prop ID** system handles more complex interactions, like entering a cave or triggering a warp.

![startTownCSV](https://raw.githubusercontent.com/UncleAmra/113590030--110590042/main/ScreenShots/startTownCSV.png)

![startTown](https://raw.githubusercontent.com/UncleAmra/113590030--110590042/main/ScreenShots/startTownWideShot.png)

NPCs, props, and their behavior (dialogue trees, shop inventories, animation rules) are configured through data files rather than hardcoded, making it easy to expand towns without touching game logic.

![data](https://raw.githubusercontent.com/UncleAmra/113590030--110590042/main/ScreenShots/NPP&PropsDatabase.png)

## Getting Started

This project is built directly on top of the PTSD framework (included as regular tracked files, no submodule required).

1. Clone the repository:

   ```bash
   git clone https://github.com/MrAAAswat/PokemonLikeEngine.git
   ```

2. Build the project:

   > [!WARNING]
   > Build in `Debug` mode — `Release` paths are not fully configured.

   ```sh
   cmake -DCMAKE_BUILD_TYPE=Debug -B build # -G Ninja
   cmake --build build
   ```

For more on the underlying framework, see the [PTSD README](https://github.com/ntut-open-source-club/practical-tools-for-simple-design).

## Built With

- **PTSD** — rendering, input, and audio framework ([practical-tools-for-simple-design](https://github.com/ntut-open-source-club/practical-tools-for-simple-design))
- **CMake** — build system
- **nlohmann/json** — JSON parsing for game data

## Credits

Developed as the final project for OOPL 2026 at NTUT.

| Member | Contribution |
| --- | --- |
| Abdulahad Aswat | 60% |
| G.Amarjargal Eddie | 40% |
