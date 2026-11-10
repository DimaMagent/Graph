#include <iostream>
#include <string>
#include <vector>
#include "Config.hpp"
#include "DependencyFetcher.hpp"

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");

    std::string config_path = (argc > 1) ? argv[1] : "config.json";

    std::cout << "Запуск приложения...\n";
    std::cout << "Использование файла конфигурации: " << config_path << "\n\n";

    try {
        AppConfig config = AppConfig::load_from_file(config_path);
        config.print_parameters();

        std::cout << "\nПолучение информации о прямых зависимостях для пакета '" << config.package_name << "'...\n";

        std::vector<std::string> dependencies = DependencyFetcher::fetch_dependencies(
            config.package_name,
            config.repository_source,
            config.repository_mode
        );

        std::cout << "\n=== Прямые зависимости пакета '" << config.package_name << "' (" << dependencies.size() << ") ===\n";
        if (dependencies.empty()) {
            std::cout << "Прямые зависимости не найдены.\n";
        }
        else {
            for (size_t i = 0; i < dependencies.size(); ++i) {
                std::cout << i + 1 << ". " << dependencies[i] << "\n";
            }
        }
        std::cout << "===============================================================\n";

    }
    catch (const std::exception& e) {
        std::cerr << "[ОШИБКА]: " << e.what() << "\n";
        return 1;
    }

    return 0;
}