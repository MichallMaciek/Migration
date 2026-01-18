# Migration
# Migration Game - Java & C++ Hybrid Project

## 🎮 Game Description
**Migration** is a strategy board game implemented using a hybrid architecture: the User Interface is built with **Java (Swing)**, while the core game logic and Artificial Intelligence are powered by **C++** for high performance.

### The Rules
* **The Board:** An 8x8 grid (customizable).
* **The Goal:** Move your pieces to the opposite side of the board or block the opponent so they cannot move.
* **Movement:**
    * **Player 1 (Cyan/User):** Can only move **UP**.
    * **Player 2 (Red/Bot):** Can only move **RIGHT**.
* **Winning:** The game ends when a player cannot make a valid move. The last player to move typically wins (or the opponent loses due to being blocked).

---

## ⚙️ Technical Architecture
This project demonstrates advanced programming concepts required for the course:
1.  **JNI (Java Native Interface):** connects the Java GUI with the C++ backend.
2.  **Alpha-Beta Pruning:** A high-performance algorithm used by the Bot to look ahead up to 8 moves deep.
3.  **Cross-Language Logic:** The C++ backend handles the board state, move validation, and AI calculations, while Java handles rendering and file I/O.

---

## 🛠️ Installation & How to Run

### Prerequisites
* **Java JDK:** Version 8 or higher.
* **G++ Compiler:** Must support C++17.
* **Terminal/Command Line.**

### 🐧 Linux & 🍏 macOS
The project includes a `run.sh` script that automatically compiles and runs the game.

1.  Open your terminal.
2.  Navigate to the project folder.
3.  Grant execution permission to the script:
    ```bash
    chmod +x run.sh
    ```
4.  Run the game:
    ```bash
    ./run.sh
    ```

### 🪟 Windows
The `run.sh` script is designed for Unix systems. For Windows, you must compile manually using **MinGW (G++)** and **Java**.

1.  **Open Command Prompt (cmd)** or PowerShell in the project folder.
2.  **Compile C++ Library:**
    (Ensure `JAVA_HOME` is set. If not, replace `%JAVA_HOME%` with the path to your JDK, e.g., `C:\Program Files\Java\jdk-17`).
    ```cmd
    g++ -shared -I"%JAVA_HOME%\include" -I"%JAVA_HOME%\include\win32" Game.cpp -o build\migration.dll
    ```
3.  **Compile Java:**
    ```cmd
    javac -d build Migration.java
    ```
4.  **Run:**
    ```cmd
    cd build
    java -Djava.library.path=. Migration
    ```

---

## 🧠 AI & Game Logic

### Difficulty Levels
The game features a configurable AI with three difficulty tiers based on search depth:
* **Easy (Depth 1):** The bot reacts only to the immediate board state.
* **Medium (Depth 5):** The bot thinks 5 moves ahead, using basic strategy.
* **Hard (Depth 8):** The bot thinks 8 moves ahead, making it extremely difficult to beat without perfect play.

### How the Bot Works (Minimax with Alpha-Beta)
1.  **State Evaluation:** The bot assigns a "score" to every possible future board.
    * It gains points for advancing pieces to the **Right** (its goal).
    * It loses points if the Player advances **Up**.
    * It gains bonus points for **blocking** the Player's path.
2.  **Recursive Search:** It simulates moves for itself (Max) and the Player (Min) recursively.
3.  **Pruning:** If the AI finds a move that is clearly worse than a previous option, it stops analyzing that branch immediately (Alpha-Beta Pruning). This allows it to search Deep 8 efficiently.

### File Structure
* `Migration.java`: Handles the Window, Animations, Mouse Inputs, and Game History saving.
* `Game.cpp`: The "Brain". Contains the Board representation, Move Generation, and AI algorithms.
* `Game.hpp`: Header file defining the C++ structures.
* `run.sh`: Automated build script for Unix systems.