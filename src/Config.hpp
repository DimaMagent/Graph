#pragma once

#include <string>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <format>
#include <nlohmann/json.hpp>

struct AppConfig {
    std::string package_name;
    std::string repository_source;
    std::string repository_mode;
    bool ascii_tree_mode{ false };
    bool load_order_mode{ false };
    bool mermaid_output_mode{ true };
    std::string mermaid_output_path{ "graph.mmd" }; // Путь для сохранения .mmd файла

    /// Загружает и валидирует конфигурацию из JSON-файла
    static AppConfig load_from_file(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error(std::format("Не удалось открыть конфигурационный файл: '{}'", filepath));
        }

        nlohmann::json j;
        try {
            file >> j;
        }
        catch (const nlohmann::json::parse_error& e) {
            throw std::runtime_error(std::format("Ошибка синтаксиса JSON в файле '{}': {}", filepath, e.what()));
        }

        AppConfig config;

        // Валидация параметра package_name
        if (!j.contains("package_name") || !j["package_name"].is_string()) {
            throw std::runtime_error("Параметр 'package_name' отсутствует или имеет неверный тип (ожидается строка).");
        }
        config.package_name = j["package_name"].get<std::string>();
        if (config.package_name.empty()) {
            throw std::runtime_error("Параметр 'package_name' не может быть пустым.");
        }

        // Валидация параметра repository_source
        if (!j.contains("repository_source") || !j["repository_source"].is_string()) {
            throw std::runtime_error("Параметр 'repository_source' отсутствует или имеет неверный тип (ожидается строка).");
        }
        config.repository_source = j["repository_source"].get<std::string>();
        if (config.repository_source.empty()) {
            throw std::runtime_error("Параметр 'repository_source' не может быть пустым.");
        }

        // Валидация параметра repository_mode
        if (!j.contains("repository_mode") || !j["repository_mode"].is_string()) {
            throw std::runtime_error("Параметр 'repository_mode' отсутствует или имеет неверный тип (ожидается строка).");
        }
        config.repository_mode = j["repository_mode"].get<std::string>();
        if (config.repository_mode != "test_file" && config.repository_mode != "remote_url") {
            throw std::runtime_error(std::format(
                "Недопустимое значение 'repository_mode': '{}'. Ожидается 'test_file' или 'remote_url'.",
                config.repository_mode
            ));
        }

        // Валидация параметра ascii_tree_mode
        if (j.contains("ascii_tree_mode")) {
            if (!j["ascii_tree_mode"].is_boolean()) {
                throw std::runtime_error("Параметр 'ascii_tree_mode' имеет неверный тип (ожидается boolean).");
            }
            config.ascii_tree_mode = j["ascii_tree_mode"].get<bool>();
        }

        // Валидация параметра load_order_mode
        if (j.contains("load_order_mode")) {
            if (!j["load_order_mode"].is_boolean()) {
                throw std::runtime_error("Параметр 'load_order_mode' имеет неверный тип (ожидается boolean).");
            }
            config.load_order_mode = j["load_order_mode"].get<bool>();
        }

        // Валидация параметров Mermaid
        if (j.contains("mermaid_output_mode")) {
            if (!j["mermaid_output_mode"].is_boolean()) {
                throw std::runtime_error("Параметр 'mermaid_output_mode' имеет неверный тип (ожидается boolean).");
            }
            config.mermaid_output_mode = j["mermaid_output_mode"].get<bool>();
        }

        if (j.contains("mermaid_output_path") && j["mermaid_output_path"].is_string()) {
            config.mermaid_output_path = j["mermaid_output_path"].get<std::string>();
        }

        return config;
    }

    void print_parameters() const {
        std::cout << "=== Конфигурация инструмента визуализации графа зависимостей ===\n";
        std::cout << std::format("{:<25} : {}\n", "package_name", package_name);
        std::cout << std::format("{:<25} : {}\n", "repository_source", repository_source);
        std::cout << std::format("{:<25} : {}\n", "repository_mode", repository_mode);
        std::cout << std::format("{:<25} : {}\n", "ascii_tree_mode", ascii_tree_mode ? "true" : "false");
        std::cout << std::format("{:<25} : {}\n", "load_order_mode", load_order_mode ? "true" : "false");
        std::cout << std::format("{:<25} : {}\n", "mermaid_output_mode", mermaid_output_mode ? "true" : "false");
        std::cout << std::format("{:<25} : {}\n", "mermaid_output_path", mermaid_output_path);
        std::cout << "===============================================================\n";
    }
};