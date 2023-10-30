#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <vector>

#include "scanner.hpp"

// autogenerated by Bison, don't panic
// if your IDE can't resolve it - call make first
#include "parser.hpp"

namespace yy {

// forward declare our simplistic AST node class so we
// can declare container for it without the header

/**
 * This class is the interface for our scanner/lexer. The end user
 * is expected to use this. It drives scanner/lexer, keeps
 * parsed AST and generally is a good place to store additional
 * context data. Both parser and lexer have access to it via internal 
 * references.
 * 
 * I know that the AST is a bit too strong word for a simple
 * vector with nodes, but this is only an example. Get off me.
 */
class Interpreter
{
public:
    Interpreter() :
        m_scanner(*this),
        m_parser(m_scanner, *this),
        m_location(0)
    {}
    
    /**
     * Run parser. Results are stored inside.
     * \returns 0 on success, 1 on failure
     */
    int parse();
    /**
     * Switch scanner input stream. Default is standard input (std::cin).
     * It will also reset AST.
     */
    void switchInputStream(std::istream *is);
    
    /**
     * This is needed so that Scanner and Parser can call some
     * methods that we want to keep hidden from the end user.
     */
    friend class Parser;
    friend class Scanner;
    
private:
    // Used internally by Scanner YY_USER_ACTION to update location indicator
    void increaseLocation(unsigned int loc);
    
    // Used to get last Scanner location. Used in error messages.
    unsigned int location() const;
    
private:
    Scanner m_scanner;
    Parser m_parser;
    unsigned int m_location;          // Used by scanner
};

}

#endif // INTERPRETER_H
