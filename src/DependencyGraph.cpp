#include "DependencyGraph.hpp"
#include <functional>

void DependencyGraph::build_bfs(const std::string& root_package, const std::string& repository_source, const std::string& repository_mode) {
    adj_list_.clear();
    visited_nodes_.clear();
    cycles_.clear();

    std::queue<std::string> q;
    q.push(root_package);
    visited_nodes_.insert(root_package);

    while (!q.empty()) {
        std::string current = q.front();
        q.pop();

        try {
            std::vector<std::string> direct_deps = DependencyFetcher::fetch_dependencies(
                current, repository_source, repository_mode
            );

            adj_list_[current] = direct_deps;

            for (const auto& dep : direct_deps) {
                // обработка циклических зависимостей
                if (visited_nodes_.find(dep) != visited_nodes_.end()) {
                    cycles_.push_back({ current, dep });
                }

                if (visited_nodes_.find(dep) == visited_nodes_.end()) {
                    visited_nodes_.insert(dep);
                    q.push(dep);
                }
            }
        }
        catch (const std::exception& e) {
            adj_list_[current] = {};
            std::cerr << std::format("[ПРЕДУПРЕЖДЕНИЕ]: Не удалось получить зависимости для пакета '{}': {}\n", current, e.what());
        }
    }
}

void DependencyGraph::print_graph() const {
    std::cout << "\n=== Граф зависимостей (Список смежности) ===\n";
    if (adj_list_.empty()) {
        std::cout << "Граф пуст.\n";
        return;
    }
    for (const auto& [pkg, deps] : adj_list_) {
        std::cout << pkg << " -> [ ";
        for (size_t i = 0; i < deps.size(); ++i) {
            std::cout << deps[i] << (i + 1 < deps.size() ? ", " : "");
        }
        std::cout << " ]\n";
    }
    std::cout << "===========================================\n";
}

void DependencyGraph::print_cycles() const {
    std::cout << "\n=== Анализ циклических зависимостей ===\n";
    if (cycles_.empty()) {
        std::cout << "Циклические зависимости не обнаружены.\n";
    }
    else {
        std::cout << "Обнаружены циклические связи (ребра в уже посещенные вершины):\n";
        for (const auto& [from, to] : cycles_) {
            std::cout << "  - Пакет '" << from << "' ссылается на уже обрабатываемый/обработанный пакет '" << to << "'\n";
        }
    }
    std::cout << "=======================================\n";
}

void DependencyGraph::print_ascii_tree(const std::string& root, const std::string& prefix, bool is_last) const {
    std::cout << prefix << (is_last ? "!-- " : "|-- ") << root << "\n";

    auto it = adj_list_.find(root);
    if (it != adj_list_.end()) {
        const auto& deps = it->second;
        for (size_t i = 0; i < deps.size(); ++i) {
            bool last = (i == deps.size() - 1);
            print_ascii_tree_recursive(deps[i], prefix + (is_last ? "    " : "|   "), last, { root });
        }
    }
}

void DependencyGraph::print_ascii_tree_recursive(const std::string& node, const std::string& prefix, bool is_last, std::vector<std::string> ancestors) const {
    std::cout << prefix << (is_last ? "!-- " : "|-- ") << node;

    bool has_cycle = false;
    for (const auto& anc : ancestors) {
        if (anc == node) {
            has_cycle = true;
            break;
        }
    }

    if (has_cycle) {
        std::cout << " (цикл!)\n";
        return;
    }

    std::cout << "\n";

    auto it = adj_list_.find(node);
    if (it != adj_list_.end()) {
        const auto& deps = it->second;
        ancestors.push_back(node);
        for (size_t i = 0; i < deps.size(); ++i) {
            bool last = (i == deps.size() - 1);
            print_ascii_tree_recursive(deps[i], prefix + (is_last ? "    " : "|   "), last, ancestors);
        }
    }
}

void DependencyGraph::print_load_order(const std::string& root) const {
    std::cout << "\n=== Этап 4: Порядок загрузки и установки зависимостей (Topological Order) ===\n";
    std::vector<std::string> order;
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> visiting;

    std::function<void(const std::string&)> dfs = [&](const std::string& node) {
        if (visiting.find(node) != visiting.end()) {
            return; // Предотвращение зацикливания при обходе циклического графа
        }
        if (visited.find(node) != visited.end()) {
            return;
        }
        visiting.insert(node);

        auto it = adj_list_.find(node);
        if (it != adj_list_.end()) {
            for (const auto& dep : it->second) {
                dfs(dep);
            }
        }

        visiting.erase(node);
        visited.insert(node);
        order.push_back(node);
        };

    dfs(root);

    if (order.empty()) {
        std::cout << "Порядок загрузки пуст.\n";
    }
    else {
        for (size_t i = 0; i < order.size(); ++i) {
            std::cout << std::format("{}. {}\n", i + 1, order[i]);
        }
    }
    std::cout << "========================================================================\n";
}