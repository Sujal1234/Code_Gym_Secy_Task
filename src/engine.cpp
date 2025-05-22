#include "../include/engine.h"
#include <iostream>
#include <random>
#include <string>
#include <array>

Engine::Engine(unsigned seed)
{
    rng.seed(seed);
    initialiseGrid();
}

bool Engine::isValidPosition(int x, int y) const {
    return x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE;
}

bool Engine::isEmptyCell(int x, int y) const {
    return isValidPosition(x, y) && grid[y][x] == '.';
}

bool Engine::isCrystalCell(int x, int y) const {
    return isValidPosition(x, y) && grid[y][x] == 'C';
}

bool Engine::isObstacleCell(int x, int y) const {
    return isValidPosition(x, y) && grid[y][x] == '#';
}

int Engine::manhattanDistance(int x1, int y1, int x2, int y2) const {
    return std::abs(x1 - x2) + std::abs(y1 - y2);
}

void Engine::initialiseGrid(){
    for (int i = 0; i < GRID_SIZE; i++)
    {
        grid[i].fill('.');
    }

    std::uniform_real_distribution<float> disMult(0, 0.1);
    float obstacleMultiplier = disMult(rng);

    std::cerr << "Obstacle Multiplier: " << obstacleMultiplier << std::endl;

    //Randomly place obstacles in 0% - 10% of the grid
    int obstacleCount = static_cast<int>(GRID_SIZE * GRID_SIZE * obstacleMultiplier);

    std::cerr << "Obstacle Count: " << obstacleCount << std::endl;

    std::uniform_int_distribution<int> disGrid(0, GRID_SIZE - 1);
    int x, y;

    for (int i = 0; i < obstacleCount; i++)
    {
        do{
            x = disGrid(rng);
            y = disGrid(rng);
        } while (!isEmptyCell(x, y));

        grid[y][x] = '#';
    }

    std::uniform_int_distribution<int> disCrystal(0, 9);
    totalCrystals = MIN_CRYSTALS  + disCrystal(rng);
    if(totalCrystals % 2 == 0){
        totalCrystals++; //Ensure odd number of crystals
    }

    //Randomly place crystals in the grid
    for (int i = 0; i < totalCrystals; i++)
    {
        do{
            x = disGrid(rng);
            y = disGrid(rng);
        } while (!isEmptyCell(x, y));

        grid[y][x] = 'C';
    }

    //Place players in opposite halves (left/right) of the grid
    std::uniform_int_distribution<int> disGridHalf(0, GRID_SIZE / 2 - 1);
    do {
        player1X = disGridHalf(rng);
        player1Y = disGridHalf(rng);
    } while (!isEmptyCell(player1X, player1Y));
    
    do {
        player2X = (GRID_SIZE / 2) + disGridHalf(rng);
        player2Y = (GRID_SIZE / 2) + disGridHalf(rng);
    } while (!isEmptyCell(player2X, player2Y));
}

void Engine::printGrid() const {
    for (int y = 0; y < GRID_SIZE; y++)
    {
        for (int x = 0; x < GRID_SIZE; x++)
        {
            if (x == player1X && y == player1Y)
                std::cout << '1';
            else if (x == player2X && y == player2Y)
                std::cout << '2';
            else
                std::cout << grid[y][x];
        }
        std::cout << '\n';
    }
}

//Getter functions
std::array<std::array<char, GRID_SIZE>, GRID_SIZE> Engine::getGrid() const{
    return grid;
}

int Engine::getTotalCrystals(){
    return totalCrystals;
}
