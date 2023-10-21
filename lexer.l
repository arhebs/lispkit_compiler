%{
	#include <iostream>
	#include <cstdlib>
	#include <string>
	#include "scanner.hpp"
	#include "interpreter.hpp"
	#include "parser.hpp"
	#include "location.hh"

	using namespace std;

	// Original yyterminate() macro returns int. Since we're using Bison 3 variants
	// as tokens, we must redefine it to change type from `int` to `Parser::semantic_type`
	#define yyterminate() yy::Parser::make_END_OF_FILE(yy::location());

	// This will track current scanner location.
	// Action is called when length of the token is known.
	#define YY_USER_ACTION m_driver.increaseLocation(yyleng);
	
	// !!!WARNING!!!
	// Location API is used, but the location is not initialized, 'cause I'm lazy. When making
	// a token with make_{something} method you can pass detailed token location. Current location
	// is accessible with m_driver.location() method. All puzzle elements are there - just
	// pass location value in every action code block below. I'm going to waste more time writing
	// this excuse than putting this boilerplate below...
	//
	// Location class can be found in location.hh and posistion.hh files. It's just a bit too much
	// boilerplate for this small example. Bummer.
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
[a-zA-Z][a-zA-Z0-9_]*(?:\.[a-zA-Z0-9_]+)?		{ return yy::Parser::make_ID(std::string{yytext}, yy::location()); }
\n												{ /* ignore newlines */ }
" "												{ /* ignore space */ }
.												{ cerr << "unknown character" << endl; }
<<EOF>>											{ return yyterminate(); }
%%