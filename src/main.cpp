#include <iostream>
#include <string>
#include <vector>
#include "Config.hpp"
#include "DependencyFetcher.hpp"
#include "DependencyGraph.hpp"

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");

    std::string config_path = (argc > 1) ? argv[1] : "config.json";

    std::cout << "Запуск приложения (Этап 5: Визуализация)...\n";
    std::cout << "Использование файла конфигурации: " << config_path << "\n\n";

    try {
        AppConfig config = AppConfig::load_from_file(config_path);
        config.print_parameters();

        std::cout << "\nПостройка графа зависимостей для пакета '" << config.package_name << "'...\n";

        DependencyGraph graph;
        graph.build_bfs(config.package_name, config.repository_source, config.repository_mode);

        graph.print_graph();
        graph.print_cycles();

        if (config.ascii_tree_mode) {
            std::cout << "\n=== Дерево зависимостей (ASCII) ===\n";
            graph.print_ascii_tree(config.package_name);
            std::cout << "===================================\n";
        }

        if (config.load_order_mode) {
            graph.print_load_order(config.package_name);
        }

        if (config.mermaid_output_mode) {
            graph.print_mermaid();
            graph.export_mermaid(config.mermaid_output_path);

            std::string html_output_path = config.mermaid_output_path + ".html";
            graph.export_html_visualization(html_output_path);
        }

    }
    catch (const std::exception& e) {
        std::cerr << "[ОШИБКА]: " << e.what() << "\n";
        return 1;
    }

    return 0;
}