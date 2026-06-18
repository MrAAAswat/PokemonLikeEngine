```markdown
# 2026 OOPL Final Report

---

## 組別資訊 / Group Information

**組別 / Group**: Group 6

**組員 / Members**:
- Abdulahad Aswat (MrAAAswat)
- G.Amarjargal Eddie (UncleAmra)

**復刻遊戲 / Recreated Game**: Pokémon – 2D Open‑World RPG

---

## 專案簡介 / Project Introduction

### 遊戲簡介 / Game Introduction

This project is a 2D Pokémon‑style open‑world RPG built on the **PTSD (Practical Tools for Simple Design)** framework provided in the OOPL course. Players can freely explore a multi‑region world, capture various Pokémon, and challenge multiple Gym Leaders in turn‑based battles. The map consists of interconnected towns and routes, each town featuring a BOSS battle, a Pokémon Center (healing), Shop system, other NPCs with potential quests. Routes include forests, caves, and ordinary paths, offering diverse terrain and scenery.

The development cycle spanned 17 weeks, covering every major game‑development phase: character sprite design, tilemap creation, collision detection, NPC interaction systems, battle mechanics, and audio integration. The project is written primarily in **C++ (78.6%)**, with supporting CSS, CMake, and JavaScript files.

### 組別分工 / Work Distribution

| Member | Responsibilities |
|--------|-------------------|
| Abdulahad Aswat | Bug fixing for encounter logic; map development and expansion as well as battle system, inventory UI |
| G.Amarjargal Eddie | Overall architecture design; map improvements|

---

## 遊戲介紹 / Game Introduction

### 遊戲規則 / Game Rules

1. **Exploration & Movement**  
   Use the arrow keys to move your character in four directions on a grid‑based map. The map includes collision detection – you cannot walk through water tiles, walls, or other obstacles such as buildings or other npcs. You can interact with other 

2. **Pokémon Collection**  
   Encounter wild Pokémon in tall grass or caves and capture them to add to your party.

3. **Battle System**  
   - Turn‑based combat – player and enemy alternate actions.  
   - Includes a **move/skill system** with damage calculation formulas.  
   - Supports type effectiveness (super‑effective, not very effective) as each pokemon has a type as well as each pokemon attack move.  
   - After battle, your Pokémon gain **experience points** and can **level up**.
<img width="1599" height="894" alt="Screenshot 2026-05-16 212207" src="https://github.com/user-attachments/assets/061d854c-0fb2-41bf-b318-123c10e69995" />

4. **Gym Leader Battles**  
   Each town has a Gym with a Leader (BOSS). Bosses have specific AI priority logic. Defeating a Gym Leader rewards the player with a **Badge** or a special key item.

5. **Item System**  
   An in‑game **inventory** holds potions, Poké Balls, key items, and more. Items can be picked up from the map or obtained after battles.

### 遊戲畫面 / Game Screenshots

> *(Please insert screenshots here – recommended: overworld exploration, battle UI, inventory/menu screens.)*

---

## 程式設計 / Program Design

### 程式架構 / Program Architecture

The project is built on the **PTSD** framework, a game‑development template provided for OOPL students. The overall structure is:

```
├── PTSD/                    # Core PTSD framework
├── src/                     # Game source code
│   ├── Trainer/             # Player trainer class (movement, NPC interactions)
│   ├── Pokemon/             # Pokémon base class (HP, Attack, Defense, Types)
│   ├── Map/                 # Map loading, tilemap rendering, collision
│   ├── Battle/              # Turn‑based battle logic and AI
│   ├── UI/                  # User interface (battle menus, HP bars, dialogs)
│   └── Inventory/           # Inventory management and item usage
├── assets/                  # Sprites, tilesets, Pokémon images, audio
└── CMakeLists.txt           # CMake build configuration
```

**Key Class Design**:

| Class | Purpose |
|-------|---------|
| `Trainer` | Controls player movement and interaction with NPCs / objects |
| `Pokemon` | Base class with HP, stats, type, moves, and status effects |
| `BattleManager` | Orchestrates turn flow – player action → enemy AI response |
| `MapManager` | Loads tilemaps, renders backgrounds, and handles collision checks |
| `DialogSystem` | Renders text dialogs using PTSD’s text rendering engine |
| `Inventory` | Manages item storage, usage, and UI display |

### 程式技術 / Technical Implementation

1. **Game Engine & Framework**  
   Built on the PTSD framework (C++), which provides rendering, input handling, and scene management.

2. **Rendering**  
   - Tilemap‑based background rendering (or image‑based for certain scenes).  
   - Sprite animation for the player character (4‑directional movement frames).

3. **Map System and Collision Detection**  
   Grid‑based collision system that restricts movement to walkable tiles. We took inspiration from the original map system in the first Pokemon games that used       tile mapping where each grid had an ID that represented a texture for the tile. Our tiles had a few properties such as texture, is walkable and if their           animating or not. In addition to this we had 2 layers of csv files to manage the ground textures as well as one abovee for npcs, items and building structures.
<img width="882" height="456" alt="image" src="https://github.com/user-attachments/assets/f70d3864-7f27-40f4-803c-cb1372e1c537" />
<img width="636" height="640" alt="image" src="https://github.com/user-attachments/assets/f3bbcd1b-61c7-4dc3-a541-f91f49f52298" />


4. **Battle System**  
   - Implemented as a **state machine** (overworld ↔ battle scene).  
   - Turn logic with damage formulas incorporating Attack, Defense, and type modifiers.  
   - Status effects and type chart are fully functional.

5. **Audio**  
   Background music (BGM) for towns, routes, and battles, plus sound effects (SFX) for collisions and attacks – integrated via PTSD’s audio module.

6. **Build & Compilation**  
   Uses **CMake** for cross‑platform building. The project is developed and tested in **Debug mode** (Release mode paths are not fully configured).

### 使用到 AI/AI Agent 的部分 / AI / AI Agent Usage

> *We used AI tools to assist us in our research on the older Pokemon games and how those engines worked in helping us create our map system. We used tools such as Claude and Gemini to help us debug and sometimes generate code too. However, whenever code was generated it was done with the intention of knowing what structure we needed to implement and understanding of the PTSD framework.*

---

## 結語 / Conclusion

### 問題與解決方法 / Problems and Solutions

| Issue | Solution |
|-------|----------|
| Player getting stuck on map edges due to collision inaccuracies | Implemented precise grid‑based collision and restricted movement to valid tiles |
| Abrupt transitions between overworld and battle scenes | Designed a dedicated `BattleScene` state with smooth fade‑in/out |
| NPC dialogue rendering glitches (text overflow, missing characters) | Utilized PTSD’s built‑in text renderer with proper line‑wrapping and dialog boxes |
| Build failures in Release mode due to incorrect asset paths | Standardized on Debug mode for all development and testing |
| Encounter logic occasionally triggering incorrectly | Abdulahad fixed the encounter‑rate algorithm and debugged edge cases |

### 自評 / Self‑Assessment

| Item | Description | Completed |
|------|-------------|-----------|
| 1    | This is a template row (example) | V |
| 2    | Repository visibility set to **public** | [ ] |
| 3    | **Debug mode** functionality is present and working | V |
| 4    | All memory leaks in the project have been resolved | [ ] |
| 5    | Report contains no typos and no missing sections | V |
| 6    | Report is well‑formatted and human‑readable | V |

### 心得 / Reflection

This OOPL final project gave us the opportunity to build a complete 2D RPG from the ground up. Over the 17‑week development cycle, we truly internalised the value of **object‑oriented design** – from the `Pokemon` base class with inheritance and polymorphism, to the clean separation of concerns among the battle, map, and inventory modules. Every subsystem had well‑defined responsibilities, which made integration and debugging far more manageable.

We also gained practical experience in **game development workflows**: requirements gathering, asset preparation, core‑mechanic implementation, and final system integration. The battle system, in particular, challenged us to carefully design turn‑based logic, damage formulas, type charts, and status effects – all while keeping the code extensible for future additions.

Equally important was **team collaboration** and **version control**. Using Git, we learned to divide tasks, resolve merge conflicts, and keep the main branch stable. We are proud of what we accomplished and believe the project demonstrates a solid grasp of both C++ and OOP principles.

### 貢獻比例 / Contribution Ratio

| Member | Contribution |
|--------|--------------|
| Abdulahad Aswat | 60% |
| G.Amarjargal Eddie | 40% |
```
