//
// Created by Rolark on 17.11.23.
//

#ifndef LISPKIT_COMPILER_INTERPRETER_MANAGER_HPP
#define LISPKIT_COMPILER_INTERPRETER_MANAGER_HPP

#include "interpreter.hpp"

namespace yy{

class interpreter_manager {
public:
    Interpreter& operator*(){
        return *_interpreter;
    }

    operator Interpreter&(){
        return *_interpreter;
    }
    operator const Interpreter& () const{
        return *_interpreter;
    }

    interpreter_manager() : _interpreter(new Interpreter){};
    ~interpreter_manager(){
        delete _interpreter;
    }

    void restart();

private:
    Interpreter* _interpreter;
};

}
#endif //LISPKIT_COMPILER_INTERPRETER_MANAGER_HPP
