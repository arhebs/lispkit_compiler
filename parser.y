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
    #include "AST.hpp"
    #include "location.hh"

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

    #define YY_POS ::yy::location{driver.current_pos()}

    #define OUT (*driver.output_stream)

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

//%token END_OF_FILE нам не нужно переизобретать EOF, он сам генерируется бизоном (см. parser.hpp "make_YYEOF")
%token <std::string> ID
%token <int64_t> NUM
%token OP_BR "("
%token CL_BR ")"

%type <AST_node> s_expr
%type <AST_node> atom
%type <AST_node> s_expr_seq

%start start

%%

start : s_expr {
    OUT << "Success" << std::endl;
    driver.AST = $1;
}

s_expr :
    atom
    {
        $$ = $1;
    }
    | OP_BR s_expr_seq CL_BR
    {
        //check for num of arguments
        AST_node& current = $2;
        if(driver.check_number_of_arguments){
            try{
                current.check_command_syntax();
            }
            catch(const std::runtime_error& err){
                error(YY_POS, err.what());
                YYABORT;
            }
        }
        $$ = $2;
    };

s_expr_seq :
    /*empty*/
    {
        $$ = AST_node{}; // create list
    }
    | s_expr_seq s_expr
    {
        $$ = $1.append($2);
    }
    ;

atom: ID
    {
        $$ = AST_node{$1};
    }
    | NUM
    {
        $$ = AST_node{$1};
    };
    
%%

// Bison expects us to provide implementation - otherwise linker complains
void yy::Parser::error(const location &loc, const std::string &message) {
    OUT << (loc.begin.filename != nullptr ? *loc.begin.filename : "input") << ':' << loc.begin.line << ':' << loc.begin.column
        << ": error: " << message << std::endl;
    driver.m_error = true;
}

