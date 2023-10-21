#include <iostream>

#include "interpreter.hpp"

int main() {
    std::cout << "Hello, World!" << std::endl;
    yy::Interpreter i;
    i.parse();
    return 0;
}
