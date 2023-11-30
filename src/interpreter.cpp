#include "interpreter.hpp"

#include <sstream>

using namespace yy;

int Interpreter::parse() {
    m_location = 0;
    
    return m_parser.parse();
}

void Interpreter::switch_streams(std::istream* is, std::ostream* os) {
    output_stream = os;
    input_stream = is;
    m_scanner.switch_streams(is, os);
}

void Interpreter::increaseLocation(unsigned int loc, unsigned int lineno) {
    m_location += loc;
    m_column += loc;
    m_lineno = lineno;
#ifdef NDEBUG
    std::cout << "Current position: " << m_location << " symbol, "
        << m_lineno << " line, "
        << m_column << " column" << std::endl;
#endif
}

void Interpreter::next_line() {
    m_column = 0;
}

unsigned int Interpreter::location() const {
    return m_location;
}

yy::position Interpreter::current_pos() {
    return yy::position{&file_name, static_cast<int>(m_lineno), static_cast<int>(m_column)};
}

AST_node &Interpreter::get_AST() {
    return AST;
}

void Interpreter::set_file_name(const std::string &str) {
    file_name = str;
}

bool Interpreter::is_error() {
    return m_error;
}
/**
 *      {"QUOTE", {1, true}},
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
        {"LAMBDA", {2, false}},
        {"LET", {2, false}},
        {"LETREC", {2, false}}
 */

Interpreter::Interpreter(std::istream *inp)  :
        m_scanner(*this),
        m_parser(m_scanner, *this),
        m_location(0),
        m_lineno(0),
        m_column(0),
        m_error(false),
        input_stream(nullptr),
        output_stream(nullptr)
{
    if(inp != nullptr)
        this->switch_streams(inp);


    functions = {
        {"QUOTE", [](AST_node& node, context_t context) -> AST_node{
            //просто возвращает аргумент
            auto&& list = node.to_list();
            return list.back();
        }},
        {"CAR", [this](AST_node& node, context_t context) -> AST_node{
            auto&& list = node.to_list();
            auto&& argument = list.back();
            //вычисляем аргумент
            auto arg_value = this->execute(argument, context);
            //является ли аргумент списком
            if(!arg_value.is_list()){
                //синтаксическая ошибка - аргумент должен быть списком
                throw report_runtime_error("CAR", node, "the argument should be a list, but a non-list value was provided");
            }
            auto&& arg_list = arg_value.to_list();
            if(arg_list.empty()){
                return AST_node{};
            }
            else{
                return *arg_list.begin();
            }
        }},
        {"CDR", [this](AST_node& node, context_t context) -> AST_node{
            auto&& list = node.to_list();
            auto&& argument = list.back();
            auto arg_value = this->execute(argument, context);
            //является ли аргумент списком
            if(!arg_value.is_list()){
                throw report_runtime_error("CDR", node, "the argument should be a list, but a non-list value was provided");
            }
            auto&& arg_list = arg_value.to_list();
            if(arg_list.size() < 2){
                throw report_runtime_error("CDR", node, "argument list must have a minimum length of 2 elements");
            }
            AST_node return_node{};
            return_node.to_list() = AST_node::AST_node_list{++arg_list.begin(), arg_list.end()};
            return return_node;
        }},
        {"CONS", [this](AST_node& node, std::unordered_map<std::string, AST_node> context) -> AST_node{
            auto&& list = node.to_list();
            auto&& new_head = *(++list.begin());
            auto&& new_tail = list.back();

            auto head_value = this->execute(new_head, context);
            auto tail_value = this->execute(new_tail, context);
            //является ли новый хвост списком
            if(!tail_value.is_list()){
                throw report_runtime_error("CONS", node, "the second argument should be a list");
            }

            auto&& tail_list = tail_value.to_list();
            tail_list.push_front(head_value);
            return tail_value;
        }},
        {"ATOM", [this](AST_node& node, context_t context) -> AST_node{
            auto&& list = node.to_list();
            auto&& atom = list.back();
            auto atom_value = this->execute(atom, context);
            if(atom_value.is_list()){
                return AST_node::FALSE();
            }
            else{
                return AST_node::TRUE();
            }
        }},
        {"EQUAL", [this](AST_node& node, context_t context) -> AST_node{
            auto&& list = node.to_list();
            auto iterator = list.begin(); ++iterator;
            auto&& left = *iterator; ++iterator;
            auto&& right = *iterator;
            int non_atom_count = 0;
            auto left_value = this->execute(left, context);
            auto right_value = this->execute(right, context);
            non_atom_count += left_value.is_list();
            non_atom_count += right_value.is_list();
            if(non_atom_count == 2){ // оба элемента списки - ошибка
                throw report_runtime_error("EQUAL", node, "cannot accept two lists simultaneously as arguments");
            }
            if(non_atom_count == 1) { // один элемент список, второй - атом
                return AST_node::FALSE();
            }
            else{
                if(left_value.value.index() != left_value.value.index())
                    return AST_node::FALSE();
                else if (left_value.is_string() && right_value.is_string()){
                    return (left_value.to_string() == right_value.to_string()) ? AST_node::TRUE() : AST_node::FALSE();
                }
                else{
                    return (left_value.to_num() == right_value.to_num()) ? AST_node::TRUE() : AST_node::FALSE();
                }
            }
        }},
        {"ADD", [this](AST_node& node, context_t context) -> AST_node{
            auto&& list = node.to_list();
            auto iterator = list.begin(); ++iterator;
            auto&& left = *iterator; ++iterator;
            auto&& right = *iterator;
            auto left_value = this->execute(left, context);
            auto right_value = this->execute(right, context);
            //оба аргумента должны быть числами
            if(!left_value.is_num() || !right_value.is_num()){
                throw report_runtime_error("ADD", node, "both arguments must be numeric values");
            }
            auto&& left_num = left_value.to_num();
            auto&& right_num = right_value.to_num();

            left_num += right_num;
            return left_value;
        }},
        {"SUB", [this](AST_node& node, context_t context) -> AST_node{
            auto&& list = node.to_list();
            auto iterator = list.begin(); ++iterator;
            auto&& left = *iterator; ++iterator;
            auto&& right = *iterator;
            auto left_value = this->execute(left, context);
            auto right_value = this->execute(right, context);
            //оба аргумента должны быть числами
            if(!left_value.is_num() || !right_value.is_num()){
                throw report_runtime_error("SUB", node, "both arguments must be numeric values");
            }
            auto&& left_num = left_value.to_num();
            auto&& right_num = right_value.to_num();

            left_num -= right_num;
            return left_value;
        }},
        {"MUL", [this](AST_node& node, context_t context) -> AST_node{
            auto&& list = node.to_list();
            auto iterator = list.begin(); ++iterator;
            auto&& left = *iterator; ++iterator;
            auto&& right = *iterator;
            auto left_value = this->execute(left, context);
            auto right_value = this->execute(right, context);
            //оба аргумента должны быть числами
            if(!left_value.is_num() || !right_value.is_num()){
                throw report_runtime_error("MUL", node, "both arguments must be numeric values");
            }
            auto&& left_num = left_value.to_num();
            auto&& right_num = right_value.to_num();

            left_num *= right_num;
            return left_value;
        }},
        {"DIVE", [this](AST_node& node, context_t context) -> AST_node{
            auto&& list = node.to_list();
            auto iterator = list.begin(); ++iterator;
            auto&& left = *iterator; ++iterator;
            auto&& right = *iterator;
            auto left_value = this->execute(left, context);
            auto right_value = this->execute(right, context);
            //оба аргумента должны быть числами
            if(!left_value.is_num() || !right_value.is_num()){
                throw report_runtime_error("MUL", node, "both arguments must be numeric values");
            }
            auto&& left_num = left_value.to_num();
            auto&& right_num = right_value.to_num();

            left_num /= right_num;
            return left_value;
        }},
        {"REM", [this](AST_node& node, context_t context) -> AST_node{
            auto&& list = node.to_list();
            auto iterator = list.begin(); ++iterator;
            auto&& left = *iterator; ++iterator;
            auto&& right = *iterator;
            auto left_value = this->execute(left, context);
            auto right_value = this->execute(right, context);
            //оба аргумента должны быть числами
            if(!left_value.is_num() || !right_value.is_num()){
                throw report_runtime_error("MUL", node, "both arguments must be numeric values");
            }
            auto&& left_num = left_value.to_num();
            auto&& right_num = right_value.to_num();

            left_num %= right_num;
            return left_value;
        }},
        {"LEQ", [this](AST_node& node, context_t context) -> AST_node{
            auto&& list = node.to_list();
            auto iterator = list.begin(); ++iterator;
            auto&& left = *iterator; ++iterator;
            auto&& right = *iterator;
            auto left_value = this->execute(left, context);
            auto right_value = this->execute(right, context);
            //оба аргумента должны быть числами
            if(!left_value.is_num() || !right_value.is_num()){
                throw report_runtime_error("MUL", node, "both arguments must be numeric values");
            }
            auto&& left_num = left_value.to_num();
            auto&& right_num = right_value.to_num();

            return left_num <= right_num ? AST_node::TRUE() : AST_node::FALSE();
        }},
        {"COND", [this](AST_node& node, context_t context) -> AST_node{
            auto&& list = node.to_list();
            auto iterator = list.begin(); ++iterator;
            auto&& condition = *iterator; ++iterator;
            auto&& true_branch = *iterator; ++iterator;
            auto&& false_branch = *iterator;

            auto condition_value = this->execute(condition, context);
            if(!condition_value.is_string()){
                throw report_runtime_error("COND", node, "argument must be boolean value");
            }
            auto&& condition_string = condition_value.to_string();
            if(condition_string != "TRUE" && condition_string != "FALSE"){
                throw report_runtime_error("COND", node, "argument must be boolean value");
            }
            if(condition_string == "TRUE"){
                return this->execute(true_branch, context);
            }
            else{
                return this->execute(false_branch, context);
            }
        }}
    };
}

AST_node Interpreter::execute(AST_node& current, context_t context) {
    if(!std::holds_alternative<AST_node::AST_node_list>(current.value)){
        throw std::runtime_error("Trying to execute atomic value");
    }
    auto&& list = std::get<AST_node::AST_node_list>(current.value);
    if(list.empty())
        throw std::runtime_error("Trying to execute empty list");
    auto&& command = *list.begin();
    if(!std::holds_alternative<std::string>(command.value))
        throw std::runtime_error("Command in not a string");
    auto&& cmd_string = std::get<std::string>(command.value);

    auto find_command = functions.find(cmd_string);
    if(find_command != functions.end()){
        //нашли функцию - выполняем
        return (*find_command).second(current, context);
    }
    else {
        //попытка найти имя в текущем контексте
        auto find_context = context.find(cmd_string);
        if(find_context != context.end()){
            //найдено в контексте
            return find_context->second;
        }
        else
            throw std::runtime_error("Cant resolve symbol");
    }
}

void Interpreter::execute() {
    try{
        auto result = this->execute(AST, {});
        (*output_stream) << result.print_tree() << std::endl;
    }
    catch(const std::runtime_error& err){
        (*output_stream) << "Execution error: " << err.what() << std::endl;
        m_error = true;
    }
}

std::runtime_error
Interpreter::report_runtime_error(std::string command, AST_node &node, std::string error_description) {
    std::stringstream error_str;
    error_str << "Error in " << command << " expression '" << node.print_tree(3) << "' - " << error_description << std::endl;
    return std::runtime_error(error_str.str());
}

