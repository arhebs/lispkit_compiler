#include "interpreter.hpp"

#include <sstream>

using namespace yy;

Interpreter::Interpreter() :
    // m_commands(),
    m_scanner(*this),
    m_parser(m_scanner, *this),
    m_location(0)
{

}

int Interpreter::parse() {
    m_location = 0;
    return m_parser.parse();
}

void Interpreter::clear() {

}

std::string Interpreter::str() const {
    return "";
}

void Interpreter::switchInputStream(std::istream *is) {
    m_scanner.switch_streams(is, NULL);
    // m_commands.clear();    
}

// void Interpreter::addCommand(const Command &cmd)
// {
//     m_commands.push_back(cmd);
// }

void Interpreter::increaseLocation(unsigned int loc) {
    m_location += loc;
    std::cout << "increaseLocation(): " << loc << ", total = " << m_location << std::endl;
}

unsigned int Interpreter::location() const {
    return m_location;
}
