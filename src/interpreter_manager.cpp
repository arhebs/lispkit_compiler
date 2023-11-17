//
// Created by Rolark on 17.11.23.
//

#include "interpreter_manager.hpp"

void yy::interpreter_manager::restart() {
    delete _interpreter;
    _interpreter = new Interpreter;
}
