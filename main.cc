#include <fstream>
#include <argparse/argparse.hpp>


#include "interpreter.hpp"
#include "main_window.hpp"
#include "AST.hpp"

int main(int argc, char** argv) {
    argparse::ArgumentParser program{"lispkit_compiler", "0.1"};
    program.add_argument("--gui")
        .help("Запускает программу в режиме графического (почти) интерфейса")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("file")
        .help("файл для компиляции")
        .required()
        .nargs(1);
    try{
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err){
        std::cerr << err.what() << std::endl;
        std::cerr << program << std::endl;
        std::exit(1);
    }

    if(program.get<bool>("--gui")){
//        MainWindow w;
//        w.loop();
    }
    else{
        yy::Interpreter i(program.get<std::string>("file"));
        i.parse();
        std::cout << i.get_AST().to_string() << std::endl;
    }
    return 0;
}
