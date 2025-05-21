#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/system.hpp>

#include <iostream>
#include <string>
#include <optional>
#include <sstream>

#include "../include/util.h"

namespace bp = boost::process;
namespace asio = boost::asio;
using namespace std::string_literals;

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