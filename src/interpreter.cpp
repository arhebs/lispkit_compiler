#include "interpreter.hpp"

#include <sstream>

using namespace yy;

int Interpreter::parse() {
    m_location = 0;
    
    return m_parser.parse();
}

void Interpreter::switch_streams(std::istream* is, std::ostream* os) {
    output_stream = os;
    input_stream = is;
    m_scanner.switch_streams(is, os);
}

void Interpreter::increaseLocation(unsigned int loc, unsigned int lineno) {
    m_location += loc;
    m_column += loc;
    m_lineno = lineno;
#ifdef NDEBUG
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

yy::position Interpreter::current_pos() {
    return yy::position{&file_name, static_cast<int>(m_lineno), static_cast<int>(m_column)};
}

AST_node &Interpreter::get_AST() {
    return AST;
}

void Interpreter::set_file_name(const std::string &str) {
    file_name = str;
}

bool Interpreter::is_error() {
    return m_error;
}


