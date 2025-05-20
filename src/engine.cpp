#include <boost/process.hpp>
#include <iostream>
#include <string>
#include <string_view>
#include <fstream>

namespace bp = boost::process;
using namespace std::string_literals;

void build_cpp(std::string code_path, std::string out_name){
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

int main(int argc, char* argv[]){
    //Usage: ./engine bot1.cpp bot2.cpp

    if(argc != 3){
        std::cerr << "Usage: ./engine path_to_bot1.cpp path_to_bot2.cpp\n";
        std::exit(1);
    }

    build_cpp(argv[1], "bot1");
    build_cpp(argv[2], "bot2");
    return 0;
}