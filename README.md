# Digger

This project is a C++ remake of **Digger**, built on top of my engine.
The main focus was applying several engine design patterns discussed in class and using them in this game.


## GitHub Link
https://github.com/Karan-Kalra1/Programming4


## Design Choices

### Command Pattern for Input

Input is handled using the **Command pattern**.

Examples of commands are:

* `GridMoveCommand`
* `ShootFireballCommand`
* `MenuNavigateCommand`
* `MenuConfirmCommand`
* `HighScoreLetterCommand`


Player 1 supports both keyboard and controller input.
Player 2 uses controller input only.
Menus and high score entry can be controlled with both keyboard and controller.

---

### Event Bus

The engine uses an **event bus** for event-based communication between systems.
Instead of tightly coupling objects together, gameplay systems can send events and interested listeners can react to them.

This is useful for things like score changes, player events, UI updates, and other gameplay notifications.
It keeps systems more independent because the sender does not need to know exactly who receives the event.

---

### Service Locator for Audio

Audio is accessed through a **Service Locator**.
Gameplay code does not directly depend on a concrete sound system. Instead, it asks the service locator for the active sound system.

This project also contains a **logging sound system**, which wraps the real sound system and logs sound calls. 

---

### Threaded Audio System

The audio system uses a separate worker thread for processing sound requests.
Gameplay code does not play sounds directly on the main game thread. Instead, it sends sound commands such as play, stop, stop all, or play looping to the audio system.

The audio system stores these requests in a queue.
The audio thread then processes the queue and calls the underlying SDL audio functions.

A mutex and condition variable are used to safely communicate between the main thread and the audio thread.
When a new sound request is pushed into the queue, the audio thread is notified and wakes up to process it.

---

### State Pattern

The **State pattern** is used for enemy behavior.

Enemies can switch between different states:

* `NobbinState`
* `HobbinState`

Nobbins move through existing tunnels, while Hobbins can dig through normal dirt.
Keeping this behavior in separate state classes avoids one large enemy update function full of conditionals.

---

### Level Loading from Text Files

Levels are loaded from external text files.
Each character in the file represents a tile or object type.

For example:

* dirt tiles
* solid boundary tiles
* empty tunnels
* player spawn positions
* enemy spawner
* diamond positions
* money bag positions


---

### Digging System

Digging is implemented as a tile-based system with smaller visual dirt pieces inside each tile.

The level stores dirt logically as grid tiles. 

Diggable dirt tiles are also split into smaller visual pieces.
When the player moves through dirt, the game checks the player's world position and removes dirt pieces within a digging radius.

This makes digging look more gradual instead of instantly removing an entire tile.

Once enough pieces of a dirt tile have been removed, the tile is marked as open.
At that point, enemies can move through it as a tunnel.

### Game Manager as a Facade

Originally, most gameplay logic was inside `GameManagerComponent`.
As the project grew, this became hard to maintain, so the logic was split into smaller systems.

The current structure uses:

* `LevelSystem`
* `PlayerManager`
* `EnemyManager`
* `MoneyBagManager`
* `FireballManager`
* `HudController`
* `MenuController`
* `HighScoreScreenController`
* `DeathSequenceController`

`GameManagerComponent` now mainly acts as a facade.
Other gameplay objects still call the game manager, but the actual work is delegated to the correct systems.

This keeps the public interface simple while making the internal code easier to organize.

---

### Scene and Object Lifetime

Gameplay objects are owned by the scene and removed through the scene when they are no longer needed.

For level transitions, the game keeps track of spawned level objects so they can all be cleared before loading the next level.
Old enemies are also marked for removal before being destroyed, which prevents them from updating for one extra frame and affecting the newly loaded level.

---

## Controls

### Player 1

* Keyboard: WASD to move
* Keyboard: Space to shoot
* Controller 0: D-pad to move
* Controller 0: A to shoot

### Player 2

* Controller 1: D-pad to move
* Controller 1: A to shoot

### Menu and High Score Input

* Keyboard: Arrow keys and Enter
* Controller: D-pad and A
