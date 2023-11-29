//
// Created by Rolark on 14.11.23.
//

#include "main_window.hpp"

MainWindow::MainWindow() :
    paned(Gtk::Orientation::HORIZONTAL),
    execute_button("Запустить"),
    clear_state_button("Сбросить")
{
    set_title("Basic application");
    set_default_size(800, 600);

    grid.set_margin(10);
    grid.set_column_spacing(10);
    grid.set_row_spacing(10);

    //поле ввода кода для трансляции
    code_window.set_child(code_view);
    code_window.set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
    code_window.set_expand();

    //поле вывода результата трансляции (read-only)
    result_view.set_editable(false);
    result_window.set_child(result_view);
    result_window.set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
    result_window.set_expand();

    //вывод дерева трансляции через tree view
    AST_buffer = Gtk::TreeStore::create(AST_columns);
    AST_view.set_model(AST_buffer);
    AST_view.append_column("command", AST_columns.operation);
    AST_view.append_column("arg_count", AST_columns.arg_count);
    AST_window.set_child(AST_view);
    AST_window.set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
    AST_window.set_expand();
    AST_window.set_margin(10);

    // связывание сигналов-слотов
    execute_button.signal_clicked().connect(
            sigc::mem_fun(*this, &MainWindow::on_execute_button_clicked));
    clear_state_button.signal_clicked().connect(
            sigc::mem_fun(*this, &MainWindow::on_clear_state_button_clicked));
    clear_state_button.set_sensitive(false);

    //прикрепление элементов к сетке
    grid.attach(code_window, 0, 0, 2, 1);
    grid.attach(execute_button, 0, 1);
    grid.attach(clear_state_button, 1, 1);
    grid.attach(result_window, 0, 2, 2, 1);
    //grid.attach(AST_window, 2, 0, 1, 3);

    //изначально углы скругленные, хочу квадратные
    Glib::RefPtr<Gtk::CssProvider> refCssProvider;
    refCssProvider  = Gtk::CssProvider::create();
    refCssProvider->load_from_data (".squared {border-radius: 0;}");
    Gtk::StyleContext::add_provider_for_display (
            Gdk::Display::get_default(), refCssProvider, GTK_STYLE_PROVIDER_PRIORITY_USER);
    AST_frame.set_child(AST_window);
    AST_frame.add_css_class("squared");
    AST_frame.set_label("Execution history");

    paned.set_start_child(grid);
    paned.set_end_child(AST_frame);

    set_child(paned);
}

void MainWindow::on_execute_button_clicked() {
    auto text = std::string{code_view.get_buffer()->get_text()};
    interpreter_input = std::istringstream{text};
    std::stringstream result;
    (*interpreter).switch_streams(&interpreter_input, &result);
    (*interpreter).parse();
    if((*interpreter).is_error()){
        result_view.get_buffer()->set_text(result.str());
        interpreter.restart();
    }
    else{
        result << "Generated from AST: " << (*interpreter).get_AST().to_string() << std::endl;
        result_view.get_buffer()->set_text(result.str());
        execute_button.set_sensitive(false);
        clear_state_button.set_sensitive(true);
        fill_AST_buffer();
    }
}

void MainWindow::on_clear_state_button_clicked() {
    interpreter.restart();
    result_view.get_buffer()->set_text("");

    execute_button.set_sensitive(true);
    clear_state_button.set_sensitive(false);
}

void MainWindow::fill_AST_buffer() {
    AST_node& current = (*interpreter).get_AST();
    auto row = *AST_buffer->append();
    if(std::holds_alternative<std::string>(current.value)){
        row[AST_columns.operation] = std::get<std::string>(current.value);
        row[AST_columns.arg_count] = 0;
    }
    else if(std::holds_alternative<AST_node::num_t>(current.value)){
        row[AST_columns.operation] = std::to_string(std::get<AST_node::num_t>(current.value));
        row[AST_columns.arg_count] = 0;
    }
    else{
        auto&& list = std::get<AST_node::AST_node_list>(current.value);
        row[AST_columns.operation] = "()";
        row[AST_columns.arg_count] = list.size();
        for(auto&& node : list){
            AST_traversal(node, row);
        }
    }
}

void MainWindow::AST_traversal(AST_node& current, Gtk::TreeRow& parent_row) {
    auto row = *AST_buffer->append(parent_row.children());
    if(std::holds_alternative<std::string>(current.value)){
        row[AST_columns.operation] = std::get<std::string>(current.value);
        row[AST_columns.arg_count] = 0;
    }
    else if(std::holds_alternative<AST_node::num_t>(current.value)){
        row[AST_columns.operation] = std::to_string(std::get<AST_node::num_t>(current.value));
        row[AST_columns.arg_count] = 0;
    }
    else{
        auto&& list = std::get<AST_node::AST_node_list>(current.value);
        row[AST_columns.operation] = "()";
        row[AST_columns.arg_count] = list.size();
        for(auto&& node : list){
            AST_traversal(node, row);
        }
    }
}
