#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <boost/system.hpp>

#include <iostream>
#include <string>
#include <string_view>
#include <sstream>
#include <chrono>

namespace bp = boost::process;
namespace asio = boost::asio;
using namespace std::string_literals;

void buildCpp(std::string code_path, std::string out_name);
std::optional<std::string> readPipeDeadline(bp::async_pipe &readPipe, asio::io_context &ctx,
                      boost::posix_time::seconds deadline);

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
    while(!gameOver){
        //TODO: GAME LOGIC
    }    
    
    bot1.wait();
    bot2.wait();
    return 0;
}

void buildCpp(std::string code_path, std::string out_name){
    std::string cmd {"g++ -std=c++20 -o bin/"s + out_name + " " + code_path};

    int status = system(cmd.c_str());

    if(status == -1){
        perror("system");
        std::exit(2);
    }
    if(WIFEXITED(status)){
        if(WEXITSTATUS(status) != 0){
            std::cerr << "g++ exited with status code: " << WEXITSTATUS(status) << '\n';
            std::exit(3);
        }
    }
}

std::optional<std::string> readPipeDeadline(bp::async_pipe &readPipe, asio::io_context &ctx,
                      boost::posix_time::seconds deadline) {
    std::string input;

    bool timedOut = false;
    asio::deadline_timer timer(ctx);
    timer.expires_from_now(boost::posix_time::seconds(deadline));

    auto on_timeout = [&](boost::system::error_code ec){
        if(ec == asio::error::operation_aborted){
            //Read has occurred first.
            return;
        }
        timedOut = true;
        readPipe.cancel();        
    };
    timer.async_wait(on_timeout);

    auto handle_read = [&](const boost::system::error_code &ec, asio::streambuf &buf,
    std::size_t n_bytes){
        if(ec){
            std::cerr << "Read error: " << ec.message() << std::endl;
            return;
        }
        else{
            //Read executed first.
            std::istream is(&buf);
            std::getline(is, input);
            buf.consume(n_bytes);
        }
    };

    asio::streambuf buffer;
    asio::async_read_until(readPipe, buffer, '\n',
    [&](auto ec, std::size_t n_bytes){
        if(ec == asio::error::operation_aborted && timedOut){
            //Timer has expired and cancelled the read.
            return;
        }
        else{
            handle_read(ec, buffer, n_bytes);
        }
    });
    ctx.run();
    ctx.restart();

    if(input.empty()){
        return std::nullopt;
    }
    else{
        return input;
    }
}