#include <fstream>
#include "interpreter.hpp"

int main(int argc, char** argv) {
    std::istream* inp = &std::cin;
    if(argc == 2){
        inp = new std::ifstream(argv[1]);
    }
    yy::Interpreter i(inp);
    i.parse();
    if(argc == 2)
        delete inp;
    return 0;
}
