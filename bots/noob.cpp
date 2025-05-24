#include <iostream>
#include <string>
#include <random>
#include <array>
#include <cstddef>

constexpr int GRID_SIZE = 20;

bool isValidPosition(int x, int y){
    return x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE;
}

int main(){
    int x, y, ignore;
    std::cin >> x >> y;
    for (int i = 0; i < 6; i++)
    {
        int ignore;
        std::cin >> ignore;
    }
    std::array<std::array<char, GRID_SIZE>, GRID_SIZE> grid;
    for(int i = 0; i < GRID_SIZE; ++i){
        for(int j = 0; j < GRID_SIZE; ++j){
            std::cin >> grid[i][j];
        }
    }

    std::string dirs[4] = {"UP", "DOWN", "LEFT", "RIGHT"};
    std::random_device rd;
    std::mt19937 gen(rd());
    
    std::uniform_int_distribution<std::size_t> dis(0, 3);

    std::size_t ind = dis(gen);

    std::cerr << "NOOB: " << "MOVE " << dirs[ind] << " BOMB -1 -1 ATTACK -1 -1" << std::endl;
    std::cout << "MOVE " << dirs[ind] << " BOMB -1 -1 ATTACK -1 -1" << std::endl;

    while(true){
        std::string oppDir;
        std::cin >> oppDir >> oppDir; //Ignore the first input ("MOVE")
        
        std::cin >> x >> y;
        for (int i = 0; i < 6; i++)
        {
            std::cin >> ignore;
        }

        for(int i = 0; i < GRID_SIZE; ++i){
            for(int j = 0; j < GRID_SIZE; ++j){
                std::cin >> grid[i][j];
            }
        }
        
        ind = dis(gen);
        
        std::cerr << "NOOB: " << "MOVE " << dirs[ind] << " BOMB -1 -1 ATTACK -1 -1" << std::endl;
        std::cout << "MOVE " << dirs[ind] << " BOMB -1 -1 ATTACK -1 -1" << std::endl;
    }
}