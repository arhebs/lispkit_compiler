#include <fstream>
#include <argparse/argparse.hpp>


#include "interpreter.hpp"
#include "main_window.hpp"
#include "AST.hpp"

int main(int argc, char** argv) {
    argparse::ArgumentParser program{"lispkit_compiler", "0.1"};
    program.add_argument("file")
        .help("файл для компиляции")
        .default_value("")
        .nargs(1);
    program.add_argument("--gui")
        .help("Запускает программу в режиме графического (почти) интерфейса")
        .default_value(false)
        .implicit_value(true);
    try{
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err){
        std::cerr << err.what() << std::endl;
        std::cerr << program << std::endl;
        std::exit(1);
    }

    if(program.get<bool>("--gui")){
        auto app = Gtk::Application::create("org.arhebs.lispkit_compiler");

        return app->make_window_and_run<MainWindow>(1, argv);
    }
    else{
        auto&& input_file = program.get<std::string>("file");
        std::ifstream file_input{input_file};
        yy::Interpreter i(&file_input);
        i.parse();
        std::cout << i.get_AST().to_string() << std::endl;
        return 0;
    }
}
