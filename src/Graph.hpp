//
// Created by Noah Schonhorn on 12/2/25.
//

#ifndef RPY_PROJ_ANALYZER_GRAPH_HPP
#define RPY_PROJ_ANALYZER_GRAPH_HPP

#include "Node.hpp"
#include "Token.hpp"

#include <expected>
#include <format>
#include <iostream>
#include <print>
#include <span>
#include <type_traits>

#define H_A(t, tok) std::holds_alternative<t>(tok)

template<class T, class... Us>
struct variant_has;

template<class T, class... Us>
struct variant_has<T, std::variant<Us...>>
    : std::bool_constant<(std::is_same_v<T, Us> || ...)> {};

template <class T>
concept InTokens = variant_has<T, std::remove_cvref_t<Token>>::value;

class Graph {
    std::vector<std::unique_ptr<Node>> nodes;
    std::vector<unsigned> roots;
    std::vector<bool> visited;
    std::vector<std::string> errors;
    std::vector<Node*> nodes_w_expr;

    unsigned idx = 0;

    /*
     * This function and the next are marked as const because the linter kept
     * bugging me to change it, but they are making writes to some pointers.
     * Not sure what best practice is in a case like this.
     */
    void connect_nodes() const;
    void connect_nexts() const;

    auto assign_scores(unsigned idx, double curr_score, OpType op) -> double;

    template<class... Ts>
    struct Overload : Ts... {
        using Ts::operator()...;
    };

    template<class... Ts>
    Overload(Ts...) -> Overload<Ts...>;

    template<typename T>
    requires InTokens<T>
    [[nodiscard]] auto expect(const std::vector<Token>& tokens) -> std::expected<T, std::string> {
        auto const& tok = tokens.at(idx);
        if (std::holds_alternative<T>(tok)) {
            return std::get<T>(tokens.at(idx++));
        }

        const std::string actual = std::visit([]<typename U>(U const& t) -> std::string {
            return std::format("{} at {}", tok_name<std::decay_t<U>>(), tok_pos(t));
        }, tok);
        return std::unexpected(std::format("expected {}, got {}", tok_name<T>(), actual));
    }

    // the template argument is the token by which the expression slice is delimited.
    // for most things, it's a newline, but for things that need a new indentation,
    // it's a colon instead.
    template<typename T>
    requires InTokens<T>
    [[nodiscard]] auto expr_slice(const std::vector<Token>& tokens) -> std::expected<std::span<const Token>, std::string> {
        std::string error_msg;

        const std::size_t start_idx = idx;

        while (idx < tokens.size() && !std::holds_alternative<T>(tokens.at(idx)) && error_msg.empty()) {
            const auto& token = tokens.at(idx);
            std::visit(
                Overload {
                    [&](const TokIdent&) -> void {
                        idx++;
                    },
                    [&](const TokStrLit&) -> void {
                        idx++;
                    },
                    [&](const TokIntLit&) -> void {
                        idx++;
                    },
                    [&](const TokFloatLit&) -> void {
                        idx++;
                    },
                    [&](const TokBoolLit&) -> void {
                        idx++;
                    },
                    [&](const TokOp&) -> void {
                        idx++;
                    },
                    [&](const TokLParen&) -> void {
                        idx++;
                    },
                    [&](const TokRParen&) -> void {
                        idx++;
                    },
                    [&](const TokComma&) -> void {
                        idx++;
                    },
                    [&]<typename U>(U&& other) -> void {
                        using V = std::decay_t<U>;
                        static_assert(std::is_base_of_v<Tok, V>, "expected derived from base Tok");
                        const auto base_tok = static_cast<const Tok &>(other);
                        error_msg = std::format("expected viable Expr token, got {} at {}", tok_name<V>(),
                                                tok_pos(base_tok));
                    },
                }, token);
        }

        if (!error_msg.empty()) {
            errors.push_back(error_msg);
            return std::unexpected(error_msg);
        }

        const std::span all(tokens.data(), tokens.size());
        return all.subspan(start_idx, idx - start_idx);
    }

    template<typename T>
    [[nodiscard]] auto add_cond_node(const std::vector<Token>& tokens, const Tok& t)
        -> std::unique_ptr<Node> {
        idx++;

        const auto expr = expr_slice<TokColon>(tokens);
        if (expr) {
            return std::make_unique<T>(t, *expr);
        }

        std::println(std::cerr, "{}", expr.error());
        return nullptr;
    }

    void generate_nodes(const std::vector<Token>& tokens);

public:
    explicit Graph(const std::vector<Token>& tokens);

    auto get_nodes() -> std::vector<std::unique_ptr<Node>>&;

    auto get_roots() -> std::vector<unsigned>&;

    void print_as_graph();

    void print_all_nodes() const;

    auto graph_strs() -> std::vector<std::string>;
};


#endif //RPY_PROJ_ANALYZER_GRAPH_HPP
