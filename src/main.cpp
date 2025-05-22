#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <boost/system.hpp>

#include <iostream>
#include <string>
#include <sstream>

#include "../include/util.h"
#include "../include/engine.h"

namespace bp = boost::process;
namespace asio = boost::asio;

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
    
    bool gameOver {false};

    //TODO: GAME SETUP
    while(bot1.running() && bot2.running()){
        std::optional<std::string> bot1Output = readPipeDeadline(
            bot1_out, ctx1, boost::posix_time::seconds(responseTimeLimit)
        );
        std::optional<std::string> bot2Output= readPipeDeadline(
            bot2_out, ctx2, boost::posix_time::seconds(responseTimeLimit)
        );

        if(!bot1Output.has_value() || !bot2Output.has_value()){
            gameOver = true;
            bot1.terminate();
            bot2.terminate();
            std::cerr << "No output received from one of the bots." << std::endl;
            break;
        }
        //Do stuff with input
    }
    
    bot1.wait();
    bot2.wait();
    return 0;
}