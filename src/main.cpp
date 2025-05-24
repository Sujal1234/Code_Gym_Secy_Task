#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <boost/system.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <array>
#include <optional>

#include "../include/util.h"
#include "../include/engine.h"

namespace bp = boost::process;
namespace asio = boost::asio;

void abortProgram(Engine& engine, bp::child& bot1, bp::child& bot2);
void handleTurn(Engine& engine, bp::child& bot1, bp::child& bot2,
    bp::async_pipe& bot1_out, bp::async_pipe& bot2_out);

int main(int argc, char* argv[]){
    //Usage: ./engine bot1.cpp bot2.cpp

    if(argc != 3){
        std::cerr << "Usage: ./engine path_to_bot1.cpp path_to_bot2.cpp\n";
        std::exit(1);
    }

    buildCpp(argv[1], "bot1");
    buildCpp(argv[2], "bot2");

    asio::io_context ctx1;
    asio::io_context ctx2;
    
    bp::async_pipe bot1_out{ctx1}, bot2_out{ctx2};
    bp::opstream bot1_in, bot2_in;

    
    bp::child bot1("bin/bot1", bp::std_out > bot1_out, bp::std_in < bot1_in, ctx1);
    bp::child bot2("bin/bot2", bp::std_out > bot2_out, bp::std_in < bot2_in, ctx2);

    Engine engine(0);

    //First turn - send just the game state and grid to both bots    
    bot1_in << engine.getGameState(0) << std::endl;
    bot2_in << engine.getGameState(1) << std::endl;

    std::array<std::array<char, GRID_SIZE>, GRID_SIZE> grid = engine.getGrid();
    for(int i = 0; i < GRID_SIZE; ++i){
        for(int j = 0; j < GRID_SIZE; ++j){
            bot1_in << grid[i][j];
            bot2_in << grid[i][j];
        }
        bot1_in << std::endl;
        bot2_in << std::endl;
    }

    engine.printGrid(); //For debugging

    handleTurn(engine, bot1, bot2, bot1_out, bot2_out);
    
    while(bot1.running() && bot2.running()){
        engine.printGrid(); //For debugging

        //Send the last move made by the opponent
        bot1_in << "MOVE " << engine.getLastMove(1) << std::endl;
        bot2_in << "MOVE " << engine.getLastMove(0) << std::endl;

        bot1_in << engine.getGameState(0) << std::endl;
        bot2_in << engine.getGameState(1) << std::endl;

        //Send the updated grid to both bots
        grid = engine.getGrid();
        for(int i = 0; i < GRID_SIZE; ++i){
            for(int j = 0; j < GRID_SIZE; ++j){
                bot1_in << grid[i][j];
                bot2_in << grid[i][j];
            }
            bot1_in << std::endl;
            bot2_in << std::endl;
        }

        handleTurn(engine, bot1, bot2, bot1_out, bot2_out);        
    }
    
    bot1.wait();
    bot2.wait();
    return 0;
}

void abortProgram(Engine& engine, bp::child& bot1, bp::child& bot2){
    engine.printEndReason();
    bot1.terminate();
    bot2.terminate();
    bot1.wait();
    bot2.wait();
    std::exit(0);
}

void handleTurn(Engine& engine, bp::child& bot1, bp::child& bot2,
    bp::async_pipe& bot1_out, bp::async_pipe& bot2_out){

    auto& ctx1 = (asio::io_context&) bot1_out.get_executor().context();
    auto& ctx2 = (asio::io_context&) bot2_out.get_executor().context();

    //Asynchronously read input from the bots but give them limited time to respond
    std::optional<std::string> bot1Output = readPipeDeadline(
        bot1_out, ctx1, boost::posix_time::seconds(responseTimeLimit)
    );
    std::optional<std::string> bot2Output= readPipeDeadline(
        bot2_out, ctx2, boost::posix_time::seconds(responseTimeLimit)
    );
    bool bot1ReadError = !bot1Output.has_value();
    bool bot2ReadError = !bot2Output.has_value();

    if(bot1ReadError || bot2ReadError){
        std::cerr << "Error reading input after "
        << engine.getCurrentTurn() << " turn" << std::endl;

        engine.outputReadError(bot1ReadError, bot2ReadError);
        abortProgram(engine, bot1, bot2);
    }

    engine.processTurn(bot1Output.value(), bot2Output.value());
    if(engine.isGameOver()){
        abortProgram(engine, bot1, bot2);
    }
}