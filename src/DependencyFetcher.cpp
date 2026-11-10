#include "DependencyFetcher.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <format>
#include <algorithm>
#include <cctype>
#include <array>
#include <cstdio>

std::string DependencyFetcher::fetch_from_url(const std::string& url) {
    std::string command;
#if defined(_WIN32)
    command = std::format("curl -s -L \"{}\" 2>NUL", url);
#else
    command = std::format("curl -s -L \"{}\"", url);
#endif

    std::array<char, 128> buffer;
    std::string result;

#if defined(_WIN32)
    FILE* pipe = _popen(command.c_str(), "r");
#else
    FILE* pipe = popen(command.c_str(), "r");
#endif

    if (!pipe) {
        throw std::runtime_error(std::format("Не удалось запустить процесс для загрузки данных по URL: '{}'", url));
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result.append(buffer.data());
    }

#if defined(_WIN32)
    int return_code = _pclose(pipe);
#else
    int return_code = pclose(pipe);
#endif

    if (return_code != 0 || result.empty()) {
#if defined(_WIN32)
        std::string ps_command = std::format("powershell -Command \"(Invoke-WebRequest -Uri '{}' -UseBasicParsing).Content\"", url);
        FILE* ps_pipe = _popen(ps_command.c_str(), "r");
        if (ps_pipe) {
            result.clear();
            while (fgets(buffer.data(), buffer.size(), ps_pipe) != nullptr) {
                result.append(buffer.data());
            }
            _pclose(ps_pipe);
        }
#endif
        if (result.empty()) {
            throw std::runtime_error(std::format("Не удалось получить данные по URL: '{}'. Проверьте интернет-соединение.", url));
        }
    }

    return result;
}

std::string DependencyFetcher::fetch_from_file(const std::string& filepath) {
    std::ifstream file(filepath);
    std::string active_path = filepath;

    if (!file.is_open()) {
    active_path = "tests/" + filepath;
    file.open(active_path);
}

if (!file.is_open()) {
    throw std::runtime_error(std::format("Не удалось открыть тестовый файл ни по пути '{}', ни по пути 'tests/{}'", filepath, filepath));
}

std::stringstream buffer;
buffer << file.rdbuf();
return buffer.str();
}

std::string DependencyFetcher::extract_package_name(const std::string& requirement_str, bool preserve_case) {
    std::string name;
    for (char c : requirement_str) {
        if (c == ' ' || c == ';' || c == '(' || c == '[' || c == '<' || c == '>' || c == '=' || c == '~' || c == '!') {
            break;
        }
        name.push_back(c);
    }
    if (!preserve_case) {
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c); });
    }
    return name;
}

std::vector<std::string> DependencyFetcher::parse_json_dependencies(const nlohmann::json& j, const std::string& package_name, bool preserve_case) {
    std::vector<std::string> dependencies;

    // Формат файла тестового репозитория (словарь со списком зависимостей для каждого пакета)
    if (j.contains(package_name) && j[package_name].is_array()) {
        for (const auto& dep : j[package_name]) {
            if (dep.is_string()) {
                std::string pkg_name = extract_package_name(dep.get<std::string>(), preserve_case);
                if (!pkg_name.empty()) {
                    dependencies.push_back(pkg_name);
                }
            }
        }
    }
    // Стандартный формат PyPI JSON API
    else if (j.contains("info") && j["info"].contains("requires_dist") && !j["info"]["requires_dist"].is_null()) {
        if (j["info"]["requires_dist"].is_array()) {
            for (const auto& req : j["info"]["requires_dist"]) {
                if (req.is_string()) {
                    std::string pkg_name = extract_package_name(req.get<std::string>(), preserve_case);
                    if (!pkg_name.empty()) {
                        dependencies.push_back(pkg_name);
                    }
                }
            }
        }
    }
    else if (j.contains("dependencies") && j["dependencies"].is_array()) {
        for (const auto& dep : j["dependencies"]) {
            if (dep.is_string()) {
                std::string pkg_name = extract_package_name(dep.get<std::string>(), preserve_case);
                if (!pkg_name.empty()) {
                    dependencies.push_back(pkg_name);
                }
            }
        }
    }

    std::sort(dependencies.begin(), dependencies.end());
    dependencies.erase(std::unique(dependencies.begin(), dependencies.end()), dependencies.end());

    return dependencies;
}

std::vector<std::string> DependencyFetcher::fetch_dependencies(
    const std::string& package_name,
    const std::string& repository_source,
    const std::string& repository_mode
) {
    std::string raw_json_data;
    bool preserve_case = (repository_mode == "test_file");

    if (repository_mode == "test_file") {
        raw_json_data = fetch_from_file(repository_source);
    }
    else if (repository_mode == "remote_url") {
        std::string target_url = repository_source;
        if (target_url.back() != '/') {
            target_url += '/';
        }
        if (target_url.find("pypi.org") != std::string::npos && target_url.find("/json") == std::string::npos) {
            target_url += package_name + "/json";
        }
        raw_json_data = fetch_from_url(target_url);
    }
    else {
        throw std::runtime_error(std::format("Недопустимый режим репозитория: '{}'", repository_mode));
    }

    nlohmann::json j;
    try {
        j = nlohmann::json::parse(raw_json_data);
    }
    catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error(std::format("Ошибка разбора JSON для пакета '{}': {}", package_name, e.what()));
    }

    return parse_json_dependencies(j, package_name, preserve_case);
}