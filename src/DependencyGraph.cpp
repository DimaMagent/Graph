#include "DependencyGraph.hpp"
#include <functional>
#include <fstream>
#include <cctype>

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
            return;
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

std::string DependencyGraph::generate_mermaid() const {
    std::string result = "graph TD;\n";

    // Функция для создания идентификатора узла в Mermaid
    auto sanitize_id = [](const std::string& name) {
        std::string id = "pkg_";
        for (unsigned char c : name) {
            if (std::isalnum(c)) {
                id += c;
            }
            else {
                id += '_';
            }
        }
        return id;
        };

    // Собираем все уникальные узлы
    std::unordered_set<std::string> all_nodes;
    for (const auto& [pkg, deps] : adj_list_) {
        all_nodes.insert(pkg);
        for (const auto& dep : deps) {
            all_nodes.insert(dep);
        }
    }

    // Объявляем узлы с их оригинальными именами в виде меток
    for (const auto& node : all_nodes) {
        std::string safe_id = sanitize_id(node);
        result += std::format("    {}[\"{}\"];\n", safe_id, node);
    }

    // Строим ориентированные связи между узлами
    for (const auto& [pkg, deps] : adj_list_) {
        std::string safe_pkg = sanitize_id(pkg);
        for (const auto& dep : deps) {
            std::string safe_dep = sanitize_id(dep);
            result += std::format("    {} --> {};\n", safe_pkg, safe_dep);
        }
    }

    result += "    classDef default fill:#f9f,stroke:#333,stroke-width:2px;\n";
    return result;
}

void DependencyGraph::print_mermaid() const {
    std::cout << "\n=== Этап 5: Текстовое представление графа (Mermaid) ===\n";
    std::cout << "```mermaid\n";
    std::cout << generate_mermaid();
    std::cout << "```\n";
    std::cout << "====================================================\n";
}

void DependencyGraph::export_mermaid(const std::string& filepath) const {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error(std::format("Не удалось открыть файл для записи Mermaid: '{}'", filepath));
    }
    file << generate_mermaid();
    std::cout << "[+] Mermaid граф успешно сохранен в файл: " << filepath << "\n";
}

void DependencyGraph::export_html_visualization(const std::string& html_filepath) const {
    std::ofstream file(html_filepath);
    if (!file.is_open()) {
        throw std::runtime_error(std::format("Не удалось открыть HTML файл для записи: '{}'", html_filepath));
    }

    file << "<!DOCTYPE html>\n";
    file << "<html lang=\"en\">\n";
    file << "<head>\n";
    file << "    <meta charset=\"UTF-8\">\n";
    file << "    <title>Dependency Graph Visualization (Mermaid)</title>\n";
    file << "    <script type=\"module\">\n";
    file << "        import mermaid from 'https://cdn.jsdelivr.net/npm/mermaid@10/dist/mermaid.esm.min.mjs';\n";
    file << "        mermaid.initialize({ startOnLoad: true });\n";
    file << "    </script>\n";
    file << "    <style>\n";
    file << "        body { font-family: Arial, sans-serif; margin: 40px; background-color: #f4f4f9; }\n";
    file << "        .container { background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }\n";
    file << "        h2 { color: #333; }\n";
    file << "        .mermaid { display: flex; justify-content: center; margin-top: 20px; }\n";
    file << "    </style>\n";
    file << "</head>\n";
    file << "<body>\n";
    file << "    <div class=\"container\">\n";
    file << "        <h2>Package Dependency Graph</h2>\n";
    file << "        <p>Generated automatically by dependency visualizer (Stage 5).</p>\n";
    file << "        <div class=\"mermaid\">\n";
    file << generate_mermaid();
    file << "        </div>\n";
    file << "    </div>\n";
    file << "</body>\n";
    file << "</html>\n";

    std::cout << "[+] HTML-страница визуализации сохранена: " << html_filepath << "\n";

    // Автоматическое открытие HTML в браузере
#if defined(_WIN32)
    std::string cmd = "start " + html_filepath;
    system(cmd.c_str());
#elif defined(__APPLE__)
    std::string cmd = "open " + html_filepath;
    system(cmd.c_str());
#else
    std::string cmd = "xdg-open " + html_filepath;
    system(cmd.c_str());
#endif
}