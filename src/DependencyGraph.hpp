#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stdexcept>
#include <format>
#include <iostream>
#include "DependencyFetcher.hpp"

class DependencyGraph {
private:
    std::unordered_map<std::string, std::vector<std::string>> adj_list_;
    std::unordered_set<std::string> visited_nodes_;
    std::vector<std::pair<std::string, std::string>> cycles_;

public:
    /**
     * Получение графа зависимостей алгоритмом BFS без рекурсии.
     */
    void build_bfs(const std::string& root_package, const std::string& repository_source, const std::string& repository_mode);

    const std::unordered_map<std::string, std::vector<std::string>>& get_adjacency_list() const {
        return adj_list_;
    }

    const std::vector<std::pair<std::string, std::string>>& get_cycles() const {
        return cycles_;
    }

    void print_graph() const;

    void print_cycles() const;

    void print_ascii_tree(const std::string& root, const std::string& prefix = "", bool is_last = true) const;

private:
    void print_ascii_tree_recursive(const std::string& node, const std::string& prefix, bool is_last, std::vector<std::string> ancestors) const;
};