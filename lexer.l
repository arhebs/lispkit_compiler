%{
	#include <iostream>
	#include <cstdlib>
	#include <string>
	#include "scanner.hpp"
	#include "interpreter.hpp"
	#include "parser.hpp"
	#include "location.hh"


    #define YY_POS ::yy::location{m_driver.current_pos()}

    #define OUT (*m_driver.output_stream)


	// Original yyterminate() macro returns int. Since we're using Bison 3 variants
	// as tokens, we must redefine it to change type from `int` to `Parser::semantic_type`
	#define yyterminate() yy::Parser::make_YYEOF(YY_POS);

	// This will track current scanner location.
	// Action is called when length of the token is known.
	#define YY_USER_ACTION m_driver.increaseLocation(yyleng, yylineno);

%}

%option nodefault
%option noyywrap
%option yylineno
%option c++
%option yyclass="Scanner"
%option prefix="yy_"

%%




"("												{ return yy::Parser::make_OP_BR(YY_POS); }
")"												{ return yy::Parser::make_CL_BR(YY_POS); }
[0-9]+											{ return yy::Parser::make_NUM(strtoull(yytext, 0, 10), YY_POS); }
[a-zA-Z][a-zA-Z0-9_]*                       	{ return yy::Parser::make_ID(std::string{yytext}, YY_POS); }
\n												{ m_driver.next_line(); }
" "												{ /* ignore space */ }
<<EOF>>											{ OUT << "EOF" << std::endl; return yyterminate(); }
.												{ OUT << "unknown character" << std::endl; return yyterminate(); }
%%