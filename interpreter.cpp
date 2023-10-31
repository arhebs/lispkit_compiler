#include "interpreter.hpp"

#include <sstream>

using namespace yy;

int Interpreter::parse() {
    m_location = 0;
    
    return m_parser.parse();
}

void Interpreter::switchInputStream(std::istream* is) {
    m_scanner.switch_streams(is, nullptr);
}

void Interpreter::increaseLocation(unsigned int loc) {
    m_location += loc;
#ifndef NDEBUG
    std::cout << "increaseLocation(): " << loc << ", total = " << m_location << std::endl;
#endif
}

unsigned int Interpreter::location() const {
    return m_location;
}
