# Pokémon in C/C++
This project was completed for COM S 3270 at Iowa State University. Project initially pulled from the IAState pyrite server's pokedex database, but now has an Assets folder containing various CSV files that get parsed before running the game. The original Makefile was also reconfigured to account for either a Unix or Windows system.

## How To Play
To start, run `make clean` in order to clean up the workspace, then run `make` to generate the necessary object files. In the repository folder, after a call to `make`, you can then run the game with the `./pokemon327` command. It will take a moment to start up, as it is parsing through several large CSV files.
<img width="1139" height="661" alt="image" src="https://github.com/user-attachments/assets/5b546fe9-f5d1-4d69-922c-f54dfab093b2" />

A "game loop" has been implemented, where the PC has the top priority to move--this means the PC will always take its turn first, while NPCs will move after the PC has taken a turn. The PC is denoted by the `@` symbol. The currently generated map cell that the PC is in will be displayed in the bottom left in the format `#(E/W)x#(N/S)`.
### Controls
The number keys 1-9 allow you to move around the map. Each terrain cell surrounding the player is numbered 1-9, indicating the corresponding cell the player will move to upon pressing a number key:

![Drawing 2](https://github.com/user-attachments/assets/4657af37-1369-4534-b9e7-c11ca47a84de)

Your PC can move onto any space that is not a boulder (`0`), tree (`4`), mountain (`%`), forest (`^`), or water (`~`) cell. Press `Q`, not `q`, to quit out of the game. To view your current party, press `x`. Exiting your party window is accepted by pressing the `ESC` key. To open the PokéMart and Pokémon Center menus, the PC needs to be directly on a PokeMart (`M`) or Pokemon Center (`C`) cell and press `<`. When interacting with a menu of any sort (i.e., PokéMart, Pokémon Center, Battle Menu, etc.), you can scroll using the arrow keys, select something with the `ENTER` key, and escape using the `ESC` key. Trainer Battles, for debugging purposes, can be escaped from by pressing the `ESC` key.

### Start of Game
Initially, your PC will be positioned at the map origin 0Ex0N along a random path space (denoted by a `#`). The PC will be provided with 3 random Pokémon (which can be Legendary or Mythical Pokémon) that all start at level 1, 5 potions, 10 Poké Balls, and 5 revives.

## Features Implemented
The following features were implemented in this project:
- Dynamically and Statically allocated data structures
- Pointer initialization, pointer dereferencing, and address referencing
- Terrain types
- Djikstra's Algorithm for NPCs to find the shortest possible path to get to the PC
- Various NPC AI
- Priority Queue for NPC movement following each turn in the game loop
- CSV parsing
- Classes in C++ for storing various datasets:
  - Pokemon Types
  - Pokemon Moves
  - Pokemon MoveSets
  - Pokemon
  - Items
- ArrayLists in C++ that store Classes
- `ncurses` library to produce a constantly updated and colorful map within the same terminal window

## NPCs
### NPC Types
The following NPCs were implemented in this project:
- Hikers
- Rivals
- Swimmers
- Other (Trainer)

Each of these NPCs interacts with the surrounding terrain respective to their NPC type. For each map, up to 10 NPCs are created and placed randomly on each map. All NPCs have 3 Pokémon in their party.

### NPC Behaviors
NPC behavior in this is akin to behaviors found in roguelike games. The following behaviors can be found in NPCs:
- Sentry
- Wanderer
- Pacer
- Hunter

These behaviors are designated to each NPC type as "move behaviors," which determine the cell they will move to following the PC's next turn.

## Pokémon Encounters
Wild Pokémon encounters can occur when the PC treads on any long grass (`:`) cells. Right before the battle initiates, a `!` will appear above the PC to indicate a wild Pokémon has been encountered, and a Wild Pokémon Battle will then ensue. A trainer has the opportunity to catch a wild Pokémon with a Poké Ball, and messages in the status window above the Battle UI will indicate whether or not a wild Pokémon was captured or if it escaped the Poké Ball.
### Level Scaling
Wild Pokémon are leveled in accordance to how far the map cell that the PC is currently in is from the origin.
### Shiny Pokémon
A shiny encounter has been implemented, where there is a 1/8192 chance for any Pokémon to be shiny. This has been implemented for NPC Pokémon and for the PCs Pokémon as well.

## Battle Mechanics
Battle mechanics implemented in this project were made to reflect the Gen 1 battle mechanics:
- Critical hit and critical hit chance
- Experience gain
- Type matchups
- Money gained after trainer battles
- Status conditions
- Wild and Trainer battles
- Battle turns relative to Pokémon speed
<img width="1139" height="639" alt="image" src="https://github.com/user-attachments/assets/55a3924c-aeb2-4f5d-a576-e191c94ac0d5" />



