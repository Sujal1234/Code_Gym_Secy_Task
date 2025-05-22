#ifndef engine_h
#define engine_h

#include <iostream>
#include <string>
#include <array>
#include <random>
#include <ctime>

constexpr int GRID_SIZE = 20;
constexpr int MAX_TURNS = 250;
constexpr int INITIAL_HP = 5;
constexpr int BOMB_RANGE = 3;
constexpr int ATTACK_RANGE = 3;
constexpr int COOLDOWN_PERIOD = 4;
constexpr int MIN_CRYSTALS = 10;

class Engine{
private:
    // Game state
    std::array<std::array<char, GRID_SIZE>, GRID_SIZE> grid;

    int player1X, player1Y;
    int player2X, player2Y;
    int player1HP {INITIAL_HP}, player2HP {INITIAL_HP};
    int player1Crystals {}, player2Crystals {};
    int player1BombCooldown {}, player1AttackCooldown {};
    int player2BombCooldown {}, player2AttackCooldown {};
    int totalCrystals;
    int currentTurn {};
    bool gameOver {false};
    std::string lastPlayer1Move;
    std::string lastPlayer2Move;

    std::string endReason;

    // Helper functions
    bool isValidPosition(int x, int y) const;
    bool isEmptyCell(int x, int y) const;
    bool isCrystalCell(int x, int y) const;
    bool isObstacleCell(int x, int y) const;
    int manhattanDistance(int x1, int y1, int x2, int y2) const;

    std::mt19937 rng; //Random number generator    
    
public:
    Engine(unsigned seed = static_cast<unsigned>(std::time(nullptr)));

    void initialiseGrid();
    void printGrid() const;

    //Getter functions
    std::array<std::array<char, GRID_SIZE>, GRID_SIZE> getGrid() const;
    int getTotalCrystals();
};
#endif //engine_h