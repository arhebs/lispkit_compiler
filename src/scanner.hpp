#ifndef SCANNER_H
#define SCANNER_H


/**
 * Generated Flex class name is yyFlexLexer by default. If we want to use more flex-generated
 * classes we should name them differently. See scanner.l prefix option.
 *
 * Unfortunately the implementation relies on this trick with redefining class name
 * with a preprocessor macro. See GNU Flex manual, "Generating C++ Scanners" section
 */
#if ! defined(yyFlexLexerOnce)
#undef yyFlexLexer
#define yyFlexLexer yy_FlexLexer // the trick with prefix; no namespace here :(
#include <FlexLexer.h>
#endif

// Scanner method signature is defined by this macro. Original yylex() returns int.
// Since Bison 3 uses symbol_type, we must change returned type. We also rename it
// to something sane, since you cannot overload return type.
#undef YY_DECL
#define YY_DECL yy::Parser::symbol_type yy::Scanner::get_next_token()

#include "parser.hpp" // this is needed for symbol_type

namespace yy {

// Forward declare interpreter to avoid include. Header is added inimplementation file.
class Interpreter; 
    
class Scanner final : public yyFlexLexer {
public:
    Scanner(Interpreter& driver) : m_driver(driver) {}
	virtual ~Scanner() {}
	virtual yy::Parser::symbol_type get_next_token();
        
private:
    Interpreter& m_driver;
};

}

#endif