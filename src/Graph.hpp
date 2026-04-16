//
// Created by Noah Schonhorn on 12/2/25.
//

#ifndef RPY_PROJ_ANALYZER_GRAPH_HPP
#define RPY_PROJ_ANALYZER_GRAPH_HPP

#include "Lexer.hpp"
#include "Node.hpp"
#include "Token.hpp"

#include <expected>
#include <filesystem>
#include <iostream>
#include <print>

class Graph {
    std::vector<std::unique_ptr<Node>> nodes;
    std::vector<unsigned> roots;
    std::vector<bool> visited;
    std::vector<std::string> errors;
    std::vector<Node*> nodes_w_expr;
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
    [[nodiscard]] auto add_cond_node(const Tok& t)
        -> std::unique_ptr<Node> {
        ++lexer;

        const auto expr = expr_slice(lexer);
        if (expr && lexer.curr_is<TokColon>()) {
            return std::make_unique<T>(t, *expr);
        }

        std::println(std::cerr, "{}", expr.error());
        return nullptr;
    }

    [[nodiscard]] auto add_show_node(const Tok& t, bool is_scene = false) -> std::unique_ptr<Node>;

    void generate_nodes();

public:
    explicit Graph(const std::filesystem::path &path);

    auto get_nodes() -> std::vector<std::unique_ptr<Node>>&;

    auto get_roots() -> std::vector<unsigned>&;

    void print_as_graph();

    void print_all_nodes() const;

    auto graph_strs() -> std::vector<std::string>;
};


#endif //RPY_PROJ_ANALYZER_GRAPH_HPP
