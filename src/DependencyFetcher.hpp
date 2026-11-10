#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class DependencyFetcher {
public:
    static std::vector<std::string> fetch_dependencies(
        const std::string& package_name,
        const std::string& repository_source,
        const std::string& repository_mode
    );

private:
    static std::vector<std::string> parse_json_dependencies(const nlohmann::json& j, const std::string& package_name, bool preserve_case);
    static std::string extract_package_name(const std::string& requirement_str, bool preserve_case);
    static std::string fetch_from_url(const std::string& url);
    static std::string fetch_from_file(const std::string& filepath);
};