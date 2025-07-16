# Tic Tac Toe Game in C++

This is my first project, and I am currently working on gaining experience.  
A simple Tic Tac Toe game implemented in C++ featuring both two-player mode and single-player mode against an AI opponent. This project serves as practice for algorithms, game logic, and console-based user interface implementation.

## Installation & Usage

1. Clone the repository:  
   `git clone https://github.com/DrGhost019/TicTacToe.git`

2. Navigate to the project directory:  
   `cd TicTacToe`

3. Compile the program using g++:  
   `g++ TicTacToe.cpp -o TicTacToe`

4. Run the executable:  
   `TicTacToe.exe` (on Windows)

## Features

- Two-player mode allowing two human players to compete locally  
- Single-player mode against a basic AI opponent with simple decision logic  
- Console-based user interface with clear prompts and board display  
- Robust input validation to prevent invalid moves  
- Game logic that correctly detects wins, draws, and invalid states  

## How to Play

1. Upon running the program, select the game mode:  
   - Enter '1' for two-player mode  
   - Enter '2' for single-player mode against AI

2. Players take turns placing their marks ('X' or 'O') on a 3x3 grid by entering the number corresponding to the desired cell (1-9).

3. The board cells are numbered as follows:

4. The first player to align three of their marks horizontally, vertically, or diagonally wins the game.

5. If all cells are filled without a winner, the game ends in a draw.

6. Invalid inputs or attempts to place a mark in an occupied cell will prompt an error message and ask the player to try again.

## Technologies

- C++ (Standard Library)  
- Console I/O  
- Compatible with Windows OS  

## Author

Mohammad Moein Kashefi Nejad - [moeinkashefinejad@gmail.com]  
GitHub: [https://github.com/DrGhost019](https://github.com/DrGhost019)

## License

This project is licensed under the MIT License - see the [LICENSE](./LICENSE) file for details.
