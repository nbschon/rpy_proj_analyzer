//
// Created by Noah Schonhorn on 12/2/25.
//

#ifndef RPY_PROJ_ANALYZER_GRAPH_HPP
#define RPY_PROJ_ANALYZER_GRAPH_HPP

#include "Lexer.hpp"
#include "Node.hpp"
#include "Token.hpp"

#include <concepts>
#include <expected>
#include <filesystem>
#include <functional>
#include <iostream>
#include <print>

template<typename F>
concept NodeVisitor = std::invocable<F, const Node&>;

class Graph {
    std::vector<std::unique_ptr<Node>> nodes;
    std::vector<unsigned> roots;
    std::vector<bool> visited;
    std::vector<std::string> errors;
    std::vector<Node*> nodes_w_expr;
    std::vector<Node*> nodes_w_atl;
    std::vector<Token> tokens;
    Lexer lexer;

    unsigned idx = 0;

    /*
     * This function and the next are marked as const because the linter kept
     * bugging me to change it, but they are making writes to some pointers.
     * Not sure what best practice is in a case like this.
     */
    void connect_ancestors() const;
    void connect_nexts() const;

    auto assign_scores(unsigned idx, double curr_score, OpType op) -> double;

    template<class... Ts>
    struct Overload : Ts... {
        using Ts::operator()...;
    };

    template<class... Ts>
    Overload(Ts...) -> Overload<Ts...>;

    template<typename T>
    [[nodiscard]] auto add_cond_node(const Tok& tok) -> std::unique_ptr<Node> {
        ++lexer;

        const auto expr = expr_slice(lexer);
        if (expr && lexer.curr_is<TokColon>()) {
            return std::make_unique<T>(tok, *expr);
        }

        std::println(std::cerr, "{}", expr.error());
        return nullptr;
    }

    [[nodiscard]] auto add_show_node(const Tok& tok, bool& has_atl, bool is_scene = false) -> std::unique_ptr<NodeShow>;

    void generate_nodes();

    enum class TrvOrd : std::uint8_t {
        Pre,
        Post,
    };

    template<TrvOrd T, NodeVisitor F>
    void dfs(F&& fn) {
        visited = std::vector<bool>(nodes.size());

        std::function<void(unsigned node)> dfs_traverse;
        dfs_traverse = [&](const unsigned node_idx) -> void {
            if (node_idx >= nodes.size()) {
                return;
            }
            if (nodes.at(node_idx) == nullptr) {
                return;
            }
            if (visited.at(node_idx)) {
                return;
            }

            visited.at(node_idx) = true;
            const Node &node = *nodes.at(node_idx);

            if constexpr (T == TrvOrd::Pre) {
                fn(node);
            }

            if (node.has_children()) {
                if (const auto *parent = dynamic_cast<const NodeParent*>(&node); parent->first_child) {
                    dfs_traverse(*parent->first_child);
                }
            }
            if (node.next) {
                dfs_traverse(*node.next);
            }

            if constexpr (T == TrvOrd::Post) {
                fn(node);
            }
        };

        for (const auto &r: roots) {
            dfs_traverse(r);
        }
    }

    int find_highest_wc_path() const;

public:
    explicit Graph(const std::filesystem::path &path);

    auto get_nodes() -> std::vector<std::unique_ptr<Node>>&;

    auto get_roots() -> std::vector<unsigned>&;

    void print_all_nodes() const;
};


#endif //RPY_PROJ_ANALYZER_GRAPH_HPP
