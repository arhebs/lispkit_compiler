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
        }},
        /**
         * (LAMBDA (A B) (что то, что использует А Б))
         * должна преобразовать себя в вид ((аргументы)(тело))
         * тоесть просто удалить слово лямбда
         */
        {"LAMBDA", [this](AST_node& node, context_t context) ->AST_node {
            auto&& list = node.to_list();
            list.erase(list.begin());
            return node;
        }},
        /**
         * предназначение: регистрация переменных в контекст
         * принцип работы:
         * (LET (TEST A B) (TEST (LAMBDA (BC) (какие то действия)) (A ()) (B ()))))
         * 1. получить сигнатуру вызываемой функции (test a b)
         * 2. сохранить в контекст значения этих символов (исполнить их)
         * 3. послать сигнатуру функции на исполнение (в execute уже символы разыменуются)
         */
        {"LET", [this](AST_node& node, context_t context) -> AST_node{
            auto&& list = node.to_list();
            auto iterator = list.begin(); ++iterator;
            auto&& function = *iterator;
            if(!function.is_list()){
                throw report_runtime_error("LET", node, "function declaration should be a list");
            }
            auto&& function_list = function.to_list(); ++iterator;
            auto requered_symbols = std::set<std::string>{};
            for(auto&& symbol : function_list){
                if(!symbol.is_string())
                    throw report_runtime_error("LET", node, "character definition must be a string");
                requered_symbols.insert(symbol.to_string());
            }
            auto registrated_symbols = std::set<std::string>{};
            for(; iterator != list.end(); ++iterator) {
                //здесь гарантировано, что будут пары строка-(s-expr)
                auto &&current = *iterator;
                auto &&current_list = current.to_list();
                auto &&symbol_name = current_list.front().to_string();
                auto &&symbol_definition = current_list.back();
                auto symbol_value = this->execute(symbol_definition, context);
                context.insert({symbol_name, symbol_value});
                registrated_symbols.insert(symbol_name);
            }
            auto missing_symbols = std::set<std::string>{};
            std::set_difference(requered_symbols.begin(), requered_symbols.end(),
                                registrated_symbols.begin(), registrated_symbols.end(),
                                std::inserter(missing_symbols, missing_symbols.begin()));
            if(!missing_symbols.empty()){
                throw report_runtime_error("LET", node, "Not all symbols declared");
            }
            auto extra_symbols = std::set<std::string>{};

            std::set_difference(registrated_symbols.begin(), registrated_symbols.end(),
                                requered_symbols.begin(), requered_symbols.end(),
                                std::inserter(extra_symbols, extra_symbols.begin()));
            if(!extra_symbols.empty()){
                report_runtime_warning("LET", node, "extra symbol have been declared");
            }
            return this->execute(function, context);

        }}
    };
}

/** (A B C)
 * на вход подается с-выражение
 * 1. если это переменная (строка), то ищем ее в контексте
 *    нашли - вернули, не нашли - эррор
 * 2. если это число (у конюховой возвращается число), то ошибка
 * 3. если это список, то это вызов функции
 * 3.1 получаем имя вызываемой функции
 * 3.2 если это библиотечная функция (в мапе functions), то вызываем ее
 * 3.3 если нет, то тогда ищем её определение в контексте
 * 3.4 нашли функцию, составлям для нее локальный контекст и исполняем
 */

AST_node Interpreter::execute(AST_node& current, context_t context) {
    if(current.is_num())
        throw report_runtime_error("Execution", current, std::format("using undeclared symbol {}", current.to_num()));
    else if(current.is_string()){
        auto context_find = context.find(current.to_string());
        if(context_find == context.end()){ // не нашли
            throw report_runtime_error("Execution", current, std::format("using undeclared symbol {}", current.to_string()));
        }
        else{
            return (*context_find).second;
        }
    }
    else{ // если список
        auto&& list = current.to_list();
        //первый элемент должен быть названием функции
        if(!list.front().is_string()){
            throw report_runtime_error("Execution", current, "function name must be a string");
        }
        auto&& function_name = list.front().to_string();
        // поиск функции в библиотечных
        auto default_find = functions.find(function_name);
        if(default_find != functions.end()){ //если функция библиотечная
            return default_find->second(current, context);
        }
        //если не нашли, тогда ищем в контексте
        auto context_find = context.find(function_name);
        if(context_find != context.end()){
            auto&& function_value = context_find->second;
            /**
             * функция должна к нам попадать вида
             * (
             *  (аргументы)
             *  (тело)
             * )
             * тогда мы создаем для нее локальный контекст и вызываем тело с ним
             */
            //очевидно, что функция должна быть списком
            if(!function_value.is_list()){
                throw report_runtime_error("Execution", current, "wrong function declaration"); // TODO улучшить тексты ошибок
            }
            auto&& function_value_list = function_value.to_list();
            //должно быть 2 аргумента
            if(function_value_list.size() != 2){
                throw report_runtime_error("Execution", current, "wrong function declaration");
            }
            auto&& function_arguments_decl = function_value_list.front();
            auto&& function_body = function_value_list.back();
            //они должны быть списками
            if(!function_arguments_decl.is_list()){
                throw report_runtime_error("Execute", current, "function argument declaration must be list");
            }
            if(!function_arguments_decl.is_list()){
                throw report_runtime_error("Execute", current, "function body declaration must be list");
            }
            auto&& arguments_list = function_arguments_decl.to_list();
            auto&& body_list = function_body.to_list();

            //проверка на количество аргументов вызываемой функции
            auto&& declarated_arg_count = arguments_list.size();
            auto&& given_arg_count = list.size() - 1;
            if(declarated_arg_count != given_arg_count){
                throw report_runtime_error("Execution", current, "Argument Count Mismatch Error: "
                                                                 "The number of arguments passed must match the number of "
                                                                 "required arguments in the function");

            }
            //создание локального контекста
            context_t local_context;
            auto list_iterator = list.begin(); ++list_iterator; //итератор по значениям аргументов
            for(auto&& argument : arguments_list){
                //аргумент должен быть строкой
                if(!argument.is_string())
                    throw report_runtime_error("Execution", current, "argument must be string");
                auto&& argument_str = argument.to_string();
                auto argument_value = this->execute(*list_iterator, context);
                local_context.insert({argument_str, argument_value});
                ++list_iterator;
            }
            //исполнение тела функции в соответствии с локальным контекстом
            return this->execute(function_body, local_context);
        }
        else{
            throw report_runtime_error("Execute", current, std::format("using undeclared symbol {}", current.to_string()));
        }
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

void Interpreter::report_runtime_warning(std::string command, AST_node &node, std::string error_description) {
    std::stringstream error_str;
    (*output_stream) << "Warning in " << command << " expression '" << node.print_tree(3) << "' - " << error_description << std::endl;
}

bool Interpreter::is_existing_symbol(const std::string &symbol, const context_t& context) {
    if(functions.contains(symbol)){
        return true;
    }
    else if(context.contains(symbol)){
        return true;
    }
    else
        return false;
}

