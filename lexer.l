%{
	#include <iostream>
	#include <cstdlib>
	#include <string>
	#include "scanner.hpp"
	#include "interpreter.hpp"
	#include "parser.hpp"
	#include "location.hh"

	// Original yyterminate() macro returns int. Since we're using Bison 3 variants
	// as tokens, we must redefine it to change type from `int` to `Parser::semantic_type`
	#define yyterminate() yy::Parser::make_YYEOF(yy::location());

	// This will track current scanner location.
	// Action is called when length of the token is known.
	#define YY_USER_ACTION m_driver.increaseLocation(yyleng);

%}

%option nodefault
%option noyywrap
%option c++
%option yyclass="Scanner"
%option prefix="yy_"

%%

"("												{ return yy::Parser::make_OP_BR(yy::location()); }
")"												{ return yy::Parser::make_CL_BR(yy::location()); }
[0-9]+											{ return yy::Parser::make_NUM(strtoull(yytext, 0, 10), yy::location()); }
[a-zA-Z][a-zA-Z0-9_]*                       	{ return yy::Parser::make_ID(std::string{yytext}, yy::location()); }
\n												{ /* ignore newlines */ }
" "												{ /* ignore space */ }
<<EOF>>											{ std::cout<<"EOF"<<std::endl; return yyterminate(); }
.												{ std::cerr << "unknown character" << std::endl; return yyterminate(); }
%%