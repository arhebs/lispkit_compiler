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

void Interpreter::increaseLocation(unsigned int loc, unsigned int lineno) {
    m_location += loc;
    m_column += loc;
    m_lineno = lineno;
#ifndef NDEBUG
    std::cout << "Current position: " << m_location << " symbol, "
        << m_lineno << " line, "
        << m_column << " column" << std::endl;
#endif
}

void Interpreter::next_line() {
    m_column = 0;
}

unsigned int Interpreter::location() const {
    return m_location;
}


