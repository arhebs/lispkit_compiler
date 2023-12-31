//
// Created by Rolark on 14.11.23.
//

#ifndef LISPKIT_COMPILER_MAIN_WINDOW_HPP
#define LISPKIT_COMPILER_MAIN_WINDOW_HPP

#include <sstream>
#include <string>
#include <stack>
#include <memory>

#include <gtkmm.h>

#include "interpreter.hpp"

class MainWindow : public Gtk::Window {
private:
    class ast_model_columns : public Gtk::TreeModel::ColumnRecord {
    public:
        ast_model_columns(){
            add(operation); add(arg_count);
        }
        Gtk::TreeModelColumn<std::string> operation;
        Gtk::TreeModelColumn<unsigned long long> arg_count;
    };
public:
    explicit MainWindow();
    ~MainWindow() override = default;
    void on_compile_button_clicked();
protected:
    void on_execute_button_clicked();

    void on_execute_secd_button_clicked();


    //gui components
    Gtk::Grid grid;

    Gtk::Paned paned;

    Gtk::ScrolledWindow code_window;
    Gtk::TextView code_view;

    Gtk::ScrolledWindow result_window;
    Gtk::TextView result_view;

    Gtk::Frame AST_frame;
    Gtk::ScrolledWindow AST_window;
    Gtk::TreeView AST_view;
    Glib::RefPtr<Gtk::TreeStore> AST_buffer;
    ast_model_columns AST_columns;

    Gtk::Button execute_button;
    Gtk::Button execute_secd_button;
    Gtk::Button compile_button;

private:
    void fill_AST_buffer();
    void AST_traversal(AST_node& current, Gtk::TreeRow& parent_row);

    std::unique_ptr<yy::Interpreter> interpreter;
    std::istringstream interpreter_input;
};


#endif //LISPKIT_COMPILER_MAIN_WINDOW_HPP
