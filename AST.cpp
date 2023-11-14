//
// Created by Rolark on 14.11.23.
//
#include "AST.hpp"

AST_node::AST_node(std::string val) : value(val)
{}

AST_node::AST_node(AST_node::num_t num) : value(num)
{}

AST_node::AST_node() : value(AST_node_list{})
{}

AST_node&& AST_node::append(AST_node node) {
    std::get<AST_node_list>(value).push_front(node);
    return std::move(*this);
}

void AST_node::check_command_syntax() {
    auto&& list = std::get<AST_node_list>(value);
    if(list.empty())
        return;
    auto&& first = list.front();
    if(!std::holds_alternative<std::string>(first.value))
        return;
    auto&& keyword = std::get<std::string>(first.value);
    auto find = Rules::num_of_arguments.find(keyword);
    if(find != Rules::num_of_arguments.end()){
        //it's the keyword check for arg num
        int arg_count = list.size() - 1;
        auto&& command_trait = find->second;
        if(!command_trait.arg_const_count){
            if (arg_count < command_trait.arguments) {
                throw std::runtime_error{std::format("Not enough arguments in {} statement. {} or more expected, but {} provided", keyword, command_trait.arguments, arg_count)};
            }
        }
        else{
            if (arg_count != command_trait.arguments)
                throw std::runtime_error{std::format("Not enough arguments in {} statement. {} expected, but {} provided", keyword, command_trait.arguments, arg_count)};
        }
        //проверка отдельных ключевых слов
        if(keyword == "LET" || keyword == "LETREC"){
            auto current = list.begin();
            current++; current++;
            for(int arg = 2; current != list.end(); current++, arg++){
                //тут должна быть пара
                auto&& elem = *current;
                if(!std::holds_alternative<AST_node_list>(elem.value))
                    throw std::runtime_error{std::format("Error in {} argument, arg {} should be pair", keyword, arg)};
                else{
                    auto&& elem_list = std::get<AST_node_list>(elem.value);
                    if(elem_list.size() != 2)
                        throw std::runtime_error{std::format("Error in {} argument, arg {} should be pair", keyword, arg)};
                }
            }
        }
    }
}
