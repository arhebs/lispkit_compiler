//
// Created by Rolark on 14.11.23.
//

#ifndef LISPKIT_COMPILER_AST_HPP
#define LISPKIT_COMPILER_AST_HPP

#include <string>
#include <variant>
#include <list>
#include <unordered_map>
#include <stdexcept>
#include <format>

#include "location.hh"

struct AST_node{
    using AST_node_list = std::list<AST_node>;
    using num_t = unsigned long long;
    std::variant<std::string, num_t, AST_node_list> value;

    //value should be std::list
    AST_node&& append(AST_node node);

    void check_command_syntax();

    explicit AST_node(std::string val);
    explicit AST_node(num_t num);
    explicit AST_node(); // list
};

namespace Rules{
    struct command_trait{
        int arguments;
        bool arg_const_count;

    };
    const std::unordered_map<std::string, command_trait> num_of_arguments{
        {"QUOTE", {1, true}},
        {"CAR", {1, true}},
        {"CDR", {1, true}},
        {"CONS", {2, true}},
        {"ATOM", {1, true}},
        {"EQUAL", {2, true}},
        {"ADD", {2, true}},
        {"SUB", {2, true}},
        {"MUL", {2, true}},
        {"DIVE", {2, true}},
        {"REM", {2, true}},
        {"LEQ", {2, true}},
        {"COND", {3, true}},
        {"LAMBDA", {2, true}},
        {"LET", {2, false}},
        {"LETREC", {2, false}}
    };
}



#endif //LISPKIT_COMPILER_AST_HPP
