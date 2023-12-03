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
    std::get<AST_node_list>(value).push_back(std::move(node));
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
                throw std::runtime_error{std::format("Arguments error in {} statement. {} or more expected, but {} provided", keyword, command_trait.arguments, arg_count)};
            }
        }
        else{
            if (arg_count != command_trait.arguments)
                throw std::runtime_error{std::format("Arguments error in {} statement. {} expected, but {} provided", keyword, command_trait.arguments, arg_count)};
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
                    //плюс проверка на то, что первый элемент должен быть строкой
                    if(!elem_list.front().is_string())
                        throw std::runtime_error{std::format("Error in {} argument, arg {}: name of symbol should be a string", keyword, arg)};
                }
            }
        }
    }
}

struct AST_to_string_struct{
    AST_node* node;
    int depth;
    bool is_last = false;
};

std::string AST_node::print_tree(int depth) {
    if(depth == 0)
        return std::string{"..."};
    if(std::holds_alternative<std::string>(this->value))
        return std::get<std::string>(this->value);
    else if(std::holds_alternative<num_t>(this->value))
        return std::to_string(std::get<num_t>(this->value));
    else{ //std::holds_alternative<AST_node_list>(this->value)
        std::stringstream ss;
        auto&& list = std::get<AST_node_list>(this->value);
        ss << '(';
        bool first = true;
        for(auto&& elem : list){
            if(!first)
                ss << ' ';
            ss << elem.print_tree(depth - 1);
            first = false;
        }
        ss << ')';
        return ss.str();
    }
}

AST_node::num_t& AST_node::to_num(){
    return std::get<num_t>(value);
}
bool AST_node::is_num(){
    return std::holds_alternative<num_t>(value);
}

std::string& AST_node::to_string(){
    return std::get<std::string>(value);
}
bool AST_node::is_string(){
    return std::holds_alternative<std::string>(value);
}

AST_node::AST_node_list& AST_node::to_list(){
    return std::get<AST_node_list>(value);
}
bool AST_node::is_list(){
    return std::holds_alternative<AST_node_list>(value);
}

AST_node AST_node::TRUE() {
    return AST_node("TRUE");
}

AST_node AST_node::FALSE() {
    return AST_node("FALSE");
}
