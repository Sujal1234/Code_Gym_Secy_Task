#include "../include/engine.h"
#include <iostream>
#include <random>
#include <string>
#include <string_view>
#include <sstream>
#include <array>
#include <utility>
#include <set>
#include <cassert>

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

    //Randomly place obstacles in 0% - 10% of the grid
    int obstacleCount = static_cast<int>(GRID_SIZE * GRID_SIZE * obstacleMultiplier);

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

//Returns true if the input format is valid, false otherwise.
bool Engine::parseMove(const std::string_view input, PlayerMove& move) const {
        std::stringstream ss(input.data());

        std::string moveStr, attackStr, bombStr;
        
        if(!(ss >> moveStr) || moveStr !=  "MOVE"){
            return false;
        }

        if(!(ss >> move.dir) || 
        (move.dir != "UP" && move.dir != "DOWN" &&
         move.dir != "LEFT" && move.dir != "RIGHT")){
            return false;
        }

        if(!(ss >> bombStr) || bombStr != "BOMB"){
            return false;
        }
        if(!(ss >> move.bombX) || !(ss >> move.bombY)){
            return false;
        }
        if(!isValidPosition(move.bombX, move.bombY)){
            if(!(move.bombX == -1 && move.bombY == -1)){ //Bomb not used
                return false;
            }
        }

        if(!(ss >> attackStr) || attackStr != "ATTACK"){
            return false;
        }
        if(!(ss >> move.attackX) || !(ss >> move.attackY)){
            return false;
        }
        if(!isValidPosition(move.attackX, move.attackY)){
            if(!(move.attackX == -1 && move.attackY == -1)){ //Attack not used
                return false;
            }
        }

        std::string remaining;
        if(ss >> remaining){
            return false; //Extra input
        }
        return true;
}

//Move the player in the specified direction
//Returns true if the move is valid, false otherwise.
bool Engine::movePlayer(int player, std::string_view move) {
    //Player 1 is 0, Player 2 is 1
    int& playerX = (player == 0) ? player1X : player2X;
    int& playerY = (player == 0) ? player1Y : player2Y;

    int newX = playerX, newY = playerY;

    if (move == "UP") newY--;
    else if (move == "DOWN") newY++;
    else if (move == "LEFT") newX--;
    else if (move == "RIGHT") newX++;

    if (!isValidPosition(newX, newY) || !isEmptyCell(newX, newY)) {
        return false; // Invalid move
    }
    playerX = newX;
    playerY = newY;
    return true;
}

void Engine::getExplosionArea(int x, int y, std::set<std::pair<int, int>>& explosionArea) const{
    explosionArea.emplace(x, y); //Add the cell where the bomb is placed

    int dx[4] = {1, -1, 0, 0};
    int dy[4] = {0, 0, 1, -1};

    for (int dir = 0; dir < 4; dir++)
    {
        for (int dist = 1; dist <= BOMB_RANGE-1; dist++)
        {
            int newX = x + dx[dir] * dist;
            int newY = y + dy[dir] * dist;

            if (!isValidPosition(newX, newY) || isObstacleCell(newX, newY)) {
                break; //Stop if out of bounds or obstacle in this direction
            }
            explosionArea.emplace(newX, newY);
        }   
    }
}

void Engine::processTurn(std::string_view player1Input, std::string_view player2Input){
   PlayerMove player1Move, player2Move;

   if(!parseMove(player1Input, player1Move)){
       player1Lost = true;;
   }
   if(!parseMove(player2Input, player2Move)){
       player2Lost = true;
   }

   bool player1Bombed {true}, player2Bombed {true};
   if(!player1Lost && player1Move.bombX == -1 && player1Move.bombY == -1){
       player1Bombed = false;
   }
   if(!player2Lost && player2Move.bombX == -1 && player2Move.bombY == -1){
       player2Bombed = false;
   }

   
    if(!player1Lost && player1Bombed){
        //Check if BOMB is placed on a non-empty cell
         if(!isEmptyCell(player1Move.bombX, player1Move.bombY)){
              player1Lost = true;
         }
         //Check if BOMB is before cooldown is over
         if(player1BombCooldown > 0){
              player1Lost = true;
         }
    }
    if(!player2Lost && player2Bombed){
         if(!isEmptyCell(player2Move.bombX, player2Move.bombY)){
              player2Lost = true;
         }
         if(player2BombCooldown > 0){
              player2Lost = true;
         }
    }

   //Check if BOMB is placed within range
   if(!player1Lost && (player1Move.bombX != -1 && player1Move.bombY != -1)){
       if(manhattanDistance(player1X, player1Y,
         player1Move.bombX, player1Move.bombY) > BOMB_RANGE){
           player1Lost = true;
       }
   }
   if(!player2Lost && (player2Move.bombX != -1 && player2Move.bombY != -1)){
       if(manhattanDistance(player2X, player2Y,
         player2Move.bombX, player2Move.bombY) > BOMB_RANGE){
           player2Lost = true;
       }
   }

   bool player1Attacked {true}, player2Attacked {true};
   if(!player1Lost && player1Move.attackX == -1 && player1Move.attackY == -1){
       player1Attacked = false;
   }
   if(!player2Lost && player2Move.attackX == -1 && player2Move.attackY == -1){
       player2Attacked = false;
   }

   //Check if ATTACK is placed within range
   if(!player1Lost && player1Attacked){
       if(manhattanDistance(player1X, player1Y,
         player1Move.attackX, player1Move.attackY) > ATTACK_RANGE){
           player1Lost = true;
       }
   }
    if(!player2Lost && player2Attacked){
         if(manhattanDistance(player2X, player2Y,
            player2Move.attackX, player2Move.attackY) > ATTACK_RANGE){
              player2Lost = true;
         }
    }

    int player1OldX {player1X}, player1OldY {player1Y};
    int player2OldX {player2X}, player2OldY {player2Y};

    if(!player1Lost && !movePlayer(0, player1Move.dir)){
        player1Lost = true;
    }
    if(!player2Lost && !movePlayer(1, player2Move.dir)){
        player2Lost = true;
    }

    //All moves have been verified for validity
    if(player1Lost || player2Lost){
        gameOver = true;
        if(player1Lost && player2Lost){
            endReason = "Tie: Both players sent an invalid move";
        }
        else if(player1Lost){
            endReason = "Player 2 wins as Player 1 sent an invalid move";
        }
        else{
            endReason = "Player 1 wins as Player 2 sent an invalid move";
        }
        return;
    }

    //Both players have made valid moves and moved successfully

    //Store the last moves so it can be sent in the next turn
    player1LastMove = player1Move.dir;
    player2LastMove = player2Move.dir;

    //Update cooldowns
    player1AttackCooldown = (player1Attacked) ? ATTACK_COOLDOWN :
                            std::max(0, player1AttackCooldown - 1);
    player2AttackCooldown = (player2Attacked) ? ATTACK_COOLDOWN :
                            std::max(0, player2AttackCooldown - 1);
    player1BombCooldown = (player1Bombed) ? BOMB_COOLDOWN :
                          std::max(0, player1BombCooldown - 1);
    player2BombCooldown = (player2Bombed) ? BOMB_COOLDOWN :
                          std::max(0, player2BombCooldown - 1);

    //Calculate cells affected by the bombs of both players
    std::set<std::pair<int, int>> explosionArea1;
    std::set<std::pair<int, int>> explosionArea2;

    if(player1Bombed){
        getExplosionArea(player1Move.bombX, player1Move.bombY, explosionArea1);
    }
    if(player2Bombed){
        getExplosionArea(player2Move.bombX, player2Move.bombY, explosionArea2);
    }

    collectCrystals(0, explosionArea1, explosionArea2);
    collectCrystals(1, explosionArea2, explosionArea1);

    std::set<std::pair<int, int>> attackArea1;
    std::set<std::pair<int, int>> attackArea2;

    if(player1Attacked){
        //Attack area is the same as explosion area
        getExplosionArea(player1Move.attackX, player1Move.attackY, attackArea1);
    }
    if(player2Attacked){
        //Attack area is the same as explosion area
        getExplosionArea(player2Move.attackX, player2Move.attackY, attackArea2);
    }

    if(attackArea1.count({player2X, player2Y})){
        player2HP--;
    }
    if(attackArea2.count({player1X, player1Y})){
        player1HP--;
    }

    //Crystals have been collected and players have attacked
    //Now we need to check if game is over
    currentTurn++;
    if(checkGameOver()){
        return;
    }
}

//To be used when both players have provided correct input and already moved
bool Engine::checkGameOver(){
    if(gameOver) return true;

    //Check if any player has lost all HP
    if(player1HP <= 0){
        assert(player1HP == 0);
        gameOver = true;
        player1Lost = true;
    }
    else if(player2HP <= 0){
        assert(player1HP == 0);
        gameOver = true;
        player2Lost = true;
    }

    if(player1Lost && player2Lost){
        gameOver = true;
        if(player1Crystals > player2Crystals){
            endReason = "Player 1 wins as both players have died and Player 1 has more crystals";
        }
        else if(player2Crystals > player1Crystals){
            endReason = "Player 2 wins as both players have died and Player 2 has more crystals";
        }
        else{
            endReason = "Tie: Both players lost all HP and have the same number of crystals";
        }
    }
    else if(player1Lost){
        gameOver = true;
        endReason = "Player 2 wins as Player 1 lost all HP";
    }
    else if(player2Lost){
        gameOver = true;
        endReason = "Player 1 wins as Player 2 lost all HP";
    }
    if(gameOver) return true;

    //Check if no crystals are left
    if(totalCrystals <= 0){
        assert(totalCrystals == 0);
        gameOver = true;
        if(player1Crystals > player2Crystals){
            player2Lost = true;
            endReason = "Player 1 wins as all crystals have been collected and Player 1 has more crystals";
        }
        else if(player2Crystals > player1Crystals){
            player1Lost = true;
            endReason = "Player 2 wins as all crystals have been collected and Player 2 has more crystals";
        }
        else{
            //Crystals are equal so check HP
            if(player1HP > player2HP){
                player2Lost = true;
                endReason = "Player 1 wins as all crystals have been collected and Player 1 has more HP";
            }
            else if(player2HP > player1HP){
                player1Lost = true;
                endReason = "Player 2 wins as all crystals have been collected and Player 2 has more HP";
            }
            else{
                player1Lost = true;
                player2Lost = true;
                endReason = "Tie: All crystals have been collected and both players have the same HP";
            }
        }
        return true;
    }

    //Check if max moves have been played
    if(currentTurn >= MAX_TURNS){
        gameOver = true;
        if(player1Crystals > player2Crystals){
            player2Lost = true;
            endReason = std::string("Player 1 wins as ") +
                        std::to_string(MAX_TURNS) +
                        std::string(" moves have been played and Player 1 has more crystals");
        }
        else if(player2Crystals > player1Crystals){
            player1Lost = true;
            endReason = std::string("Player 1 wins as ") +
                        std::to_string(MAX_TURNS) +
                        std::string(" moves have been played and Player 2 has more crystals");        }
        else{
            //Crystals are equal so check HP
            if(player1HP > player2HP){
                player2Lost = true;
                endReason = std::string("Player 1 wins as ") +
                            std::to_string(MAX_TURNS) +
                            std::string(" moves have been played, both players have the same crystals and Player 1 has more HP");
            }
            else if(player2HP > player1HP){
                player1Lost = true;
                endReason = std::string("Player 1 wins as ") +
                            std::to_string(MAX_TURNS) +
                            std::string(" moves have been played, both players have the same crystals and Player 2 has more HP");
            }
            else{
                player1Lost = true;
                player2Lost = true;
                endReason = std::string("Tie: ") +
                            std::to_string(MAX_TURNS) +
                            std::string(" moves have been played and both players have the same HP");
            }
        }
        return true;
    }

    return false; //Game is still ongoing
}

void Engine::collectCrystals(int player,
    std::set<std::pair<int, int>>& explosionArea,
    std::set<std::pair<int, int>>& explosionArea2){

    int& playerCrystals = (player == 0) ? player1Crystals : player2Crystals;

    for(const auto& [x, y] : explosionArea){
        if(isCrystalCell(x, y)){
            if(explosionArea2.count({x, y})){
                //Both players bombed the same crystal
                grid[y][x] = '.'; //Remove crystal from grid
                totalCrystals--;
            }
            else{
                //Player collects the crystal
                playerCrystals++;
                grid[y][x] = '.'; //Remove crystal from grid
            }
        }
    }
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

void Engine::printEndReason() const {
    if(gameOver){
        std::cout << endReason << '\n';
    }
    else{
        std::cout << "Game is still ongoing.\n";
    }
}

//Getter functions
std::array<std::array<char, GRID_SIZE>, GRID_SIZE> Engine::getGrid() const{
    return grid;
}

int Engine::getTotalCrystals() const{
    return totalCrystals;
}

bool Engine::isGameOver() const{
    return gameOver;
}

int Engine::getCurrentTurn() const{
    return currentTurn;
}

int Engine::getAttackCooldown(int player) const{
    if(player == 0){
        return player1AttackCooldown;
    }
    else{
        return player2AttackCooldown;
    }
}
int Engine::getBombCooldown(int player) const{
    if(player == 0){
        return player1BombCooldown;
    }
    else{
        return player2BombCooldown;
    }
}

int Engine::getCrystals(int player) const{
    if(player == 0){
        return player1Crystals;
    }
    else{
        return player2Crystals;
    }
}