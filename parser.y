%skeleton "lalr1.cc"
%require "3.0"
%defines
%define api.parser.class { Parser }

%define api.token.constructor
%define api.value.type variant
%define parse.assert
%define api.namespace { yy }
%code requires
{
    #include <iostream>
    #include <string>
    #include <vector>
    #include <stdint.h>
    /* место подключения будущего АСД*/

    namespace yy {
        class Scanner;
        class Interpreter;
    }
}

%code top
{
    #include <iostream>
    #include "scanner.hpp"
    #include "parser.hpp"
    #include "interpreter.hpp"
    #include "location.hh"

    // yylex() arguments are defined in parser.y
    static yy::Parser::symbol_type yylex(yy::Scanner &scanner, yy::Interpreter &driver) {
        return scanner.get_next_token();
    }
}

%lex-param { yy::Scanner &scanner }
%lex-param { yy::Interpreter &driver }
%parse-param { yy::Scanner &scanner }
%parse-param { yy::Interpreter &driver }

%locations
%define parse.trace
%define parse.error verbose

%define api.token.prefix {TOKEN_}

%token END_OF_FILE 
%token <std::string> ID
%token <uint64_t> NUM
%token OP_BR
%token CL_BR


%start s_expr

%%

s_expr : atom
    | OP_BR s_expr_seq CL_BR;

s_expr_seq : 
    /*empty*/
    | s_expr s_expr_seq;

atom: ID | NUM;
    
%%

// Bison expects us to provide implementation - otherwise linker complains
void yy::Parser::error(const location &loc, const std::string &message) {
    std::cout << "Error: " << message << std::endl
        << "Error location: " << driver.location() << std::endl;
}
