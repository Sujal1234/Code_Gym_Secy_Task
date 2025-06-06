#ifndef engine_h
#define engine_h

#include "../include/nlohmann_json.hpp"

#include <iostream>
#include <string>
#include <string_view>
#include <array>
#include <random>
#include <ctime>
#include <vector>
#include <utility>
#include <set>
#include <fstream>

using json = nlohmann::json;

constexpr int GRID_SIZE = 20;
constexpr int MAX_TURNS = 100;
constexpr int INITIAL_HP = 5;
constexpr int BOMB_RANGE = 3; //Including placed cell
constexpr int ATTACK_RANGE = 3; //Including placed cell
constexpr int BOMB_COOLDOWN = 4;
constexpr int ATTACK_COOLDOWN = 4;
constexpr int MIN_CRYSTALS = 10;

struct PlayerMove{
    std::string dir {};
    int bombX {}, bombY {};
    int attackX {}, attackY {};

    PlayerMove() = default;
    PlayerMove(std::string d, int bX, int bY, int aX, int aY)
        : dir(d), bombX(bX), bombY(bY), attackX(aX), attackY(aY) {}
};

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

    bool player1OutputReadError {false};
    bool player2OutputReadError {false};

    int totalCrystals;
    int currentTurn {};
    
    std::string player1LastMove {};
    std::string player2LastMove {};

    bool gameOver {false};
    bool player1Lost {false};
    bool player2Lost {false};

    std::string endReason;

    json logs; //Json object to store logs
    std::string logsFilePath {"logs.json"}; //Path of the file where logs will be written


    // Helper functions
    bool isValidPosition(int x, int y) const;
    bool isEmptyCell(int x, int y) const;
    bool isCrystalCell(int x, int y) const;
    bool isObstacleCell(int x, int y) const;
    int manhattanDistance(int x1, int y1, int x2, int y2) const;

    std::mt19937 rng; //Random number generator

    //Take the input string and retrieve the details of the move.
    //Returns true if the input format is valid, false otherwise.
    // bool parseMove(const std::string_view input, PlayerMove& move) const;
    
    //Move the player in the specified direction.
    //Returns true if the move was successful, false otherwise.
    void initialiseGrid();
    bool movePlayer(int player, std::string_view move);
    void getExplosionArea(int x, int y, std::set<std::pair<int, int>>& explosionArea) const;
    bool parseMove(const std::string_view input, PlayerMove& move) const;

    //Checks win/loss conditions and updates game state accordingly.
    //If game is over, set the end reason and update gameOver flag.
    //Returns true if game is over, false otherwise.
    bool checkGameOver();

    //Adds log of this turn to logs json object.
    //player1Error flag to be set if there was either an error while reading
    //their input, the input format was invalid or the move made was invalid.
    void logTurn(PlayerMove& player1Move, PlayerMove& player2Move);

    //Writes the logs to the appropriate logs file.
    void writeLogs();

    void collectCrystals(int player,
    std::set<std::pair<int, int>>& explosionArea,
    std::set<std::pair<int, int>>& explosionArea2);
    
public:
    //`path` is the path to the file where logs (json) will be written
    //Default value of `path` is "logs.json"
    //Default value of `seed` is static_cast<unsigned>(std::time(nullptr)) (For random seed)
    Engine();
    Engine(unsigned seed);
    Engine(std::string path);
    Engine(std::string path, unsigned seed);
    
    void printGrid() const;
    void printEndReason() const;
    void processTurn(std::string_view player1Input, std::string_view player2Input);

    //Use when the input received from (a) player(s) is invalid.
    //Accordingly set the game state and end reason.
    void outputReadError(bool player1Error, bool player2Error);
    
    //Getter functions
    std::array<std::array<char, GRID_SIZE>, GRID_SIZE> getGrid() const;
    std::string getGridStringPlayersHidden() const;
    std::string getGridString() const;

    int getTotalCrystals() const;
    bool isGameOver() const;
    int getCurrentTurn() const;
    int getAttackCooldown(int player) const;
    int getBombCooldown(int player) const;
    int getCrystals(int player) const;
    std::string getLastMove(int player) const;

    std::string getGameState(int player) const;
};
#endif //engine_h