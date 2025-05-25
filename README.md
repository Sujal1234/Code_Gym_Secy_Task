# Code_Gym_Secy_Task
A game I created, to be played between bots. Details of the game can be found in Game_Description.md

## Build
This is a cross-platform project that works on Windows, Linux and Mac OS (But tested only on Ubuntu).  
The installation steps will be detailed mainly for Ubuntu but the same tools can be installed easily on the other operating systems.

### Prerequisites
For compilation we need: `gcc`, `g++` (These are also necessary while running the final executable), `make` and the [Boost libraries](https://www.boost.org/).

```bash
# Ubuntu/Debian
sudo apt install gcc g++ make

# Fedora
sudo dnf install gcc-c++ make

# Arch
sudo pacman -S gcc g++ make
```

To install the Boost libraries the instructions on the [getting started](https://www.boost.org/doc/user-guide/getting-started.html) page of Boost's websites can be followed.  
For linux:
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential python3 libbz2-dev libz-dev libicu-dev

# Fedora
sudo dnf update
sudo dnf install gcc-c++ python3 bzip2-devel zlib-devel libicu-devel

#Arch
sudo pacman -Syu
sudo pacman -S base-devel python3 bzip2 zlib icu
```

### Compiling
On linux simply run:
```bash
make
```
in the main project directory. This will create an executable named `engine`.  
For other operating systems do the equivalent compilation of all the `.cpp` files in the `src` directory. With `g++` this would look like:
```bash
g++ src/*.cpp -o engine
```

## Usage
Only bots written in C++ (Upto C++20) are supported. To get two bots to play against each other, first ensure that a "bin" directory exists. Then run (for Linux):
```bash
./engine bot1.cpp bot2.cpp logs_file.json(optional)
```
The third argument is the path to the file where the logs will be written and it defaults to `logs.json` if not mentioned.

Running the engine will play the two bots against each other and create a game log in the specified file in JSON format.  
Details of the game logs format are given further ahead.  
The engine also prints the grid before, with the positions of the players indicated.

## Game log format
The game log is in JSON format. The attributes are as follows:

* `"grid"`: A string representing the initial grid, with '.' representing an empty cell, '#' an obstacle, 'C' a crystal, '1' being player 1 and '2' being player 2.

* It has keys of the form `"Turn <turn number>"` with each describing another object.

The format of the object for each turn is as follows:

* `"Game status"`: The value is "Game Over" if the game ended after that turn or "Ongoing" otherwise.

* If the game ended after that turn then it also has the keys `"End reason"` and `"Winner"`. These are self explanatory.

* `"Player 1"` and `"Player 2"`: Objects with the moves made by each player and other information about the player.

The object for each player is in the following format:

* `"MOVE"`: The movement direction of the player ("UP", "DOWN", "LEFT" or "RIGHT").

* `"ATTACK"`: An array with the attack coordinates sent by the player.

* `"BOMB"`: An array with the bomb coordinates sent by the player.

* `"Position"`: An array with the coordinates of the player.

* `"Attack cooldown"`, `"Bomb cooldown"`, `"Crystals"`, `"HP"` are self explanatory.

* If there was an error in reading the player's output (possibly time limit exceeded) then the `"MOVE"`, `"ATTACK"` and `"BOMB"` properties are set to "ERROR".

## Brief Code Summary
The engine first compiles the two bot scripts and stores the executables in "bin" directory.

Then it launches the two executables as child processes. To handle processes, I have used the [Boost.Process](https://www.boost.org/library/latest/process/) library.

Input is read asynchronously from the processes through pipes. For asynchronous programming the [Boost.Asio](https://www.boost.org/library/latest/asio/) library has been used. I chose to read input asynchronously as this allows me to put a time limit on the time taken to receive input.  
The way this works is to create an asynchronous timer and an asynchronous read function. Whichever finishes first will cancel the other function. (See function `readPipeDeadline` in src/util.cpp).

The `Engine` class handles the input parsing, move validation, game logic, game state updation, move logging, etc.

To make the logs in JSON I have used the popular library [nlohmann/json](https://github.com/nlohmann/json) as "include/nlohmann_json.hpp" which I have used to make a json object and pretty-print it to the logs file.

## Remarks
1. Unfortunately I have not made any decently smart bots due to time constraints.

2. Due to the nature of the program, which involves running external C++ code, there are several security vulnerabilities. I have not addressed most of these.