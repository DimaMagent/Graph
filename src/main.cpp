#include <iostream>
#include <string>
#include "Config.hpp"

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");
    std::wcout.imbue(std::locale("Russian"));

    std::string config_path = (argc > 1) ? argv[1] : "config.json";

    std::cout << "Запуск приложения...\n";
    std::cout << "Использование файла конфигурации: " << config_path << "\n\n";

    try {
        AppConfig config = AppConfig::load_from_file(config_path);

        config.print_parameters();

    }
    catch (const std::exception& e) {
        std::cerr << "[ОШИБКА КОНФИГУРАЦИИ]: " << e.what() << "\n";
        return 1;
    }

    return 0;
}