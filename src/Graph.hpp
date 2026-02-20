//
// Created by Noah Schonhorn on 12/2/25.
//

#ifndef RPY_PROJ_ANALYZER_GRAPH_HPP
#define RPY_PROJ_ANALYZER_GRAPH_HPP

#include "Node.hpp"

#include <expected>
#include <format>
#include <iostream>
#include <print>
#include <span>

#define H_A(t, tok) std::holds_alternative<t>(tok)

class Graph {
    std::vector<std::unique_ptr<Node>> nodes;
    std::vector<unsigned> roots;
    std::vector<bool> visited;
    std::vector<std::string> errors;
    std::vector<Node*> nodes_w_expr;

    /*
     * This function and the next are marked as const because the linter kept
     * bugging me to change it, but they are making writes to some pointers.
     * Not sure what best practice is in a case like this.
     */
    void connect_nodes() const;

    void connect_nexts() const;

    auto assign_scores(unsigned idx, double curr_score, OpType op) -> double;

    // template<class T, class... Ts>
    // static constexpr bool is_enum(T in, Ts... enums) {
    //     static_assert((std::is_same_v<T, Ts> && ...), "all args must be the same enum.");
    //     return ((in == enums) || ...);
    // }

    template<class... Ts>
    struct Overload : Ts... {
        using Ts::operator()...;
    };

    template<class... Ts>
    Overload(Ts...) -> Overload<Ts...>;

    template<typename T>
    [[nodiscard]] auto expect_tok(const std::vector<Token>& tokens, unsigned& idx) -> std::expected<T, std::string> {
        auto const& tok = tokens.at(idx);
        if (std::holds_alternative<T>(tok)) {
            return std::get<T>(tokens.at(idx++));
        }

        const std::string actual = std::visit([]<typename U>(U const& t) -> std::string {
            return std::format("{} at {}", tok_name<std::decay_t<U>>(), tok_pos(t));
        }, tok);
        return std::unexpected(std::format("expected {}, got {}", tok_name<T>(), actual));
    }

    template<typename T>
    [[nodiscard]] auto build_expr(const std::vector<Token>& tokens, unsigned& idx) -> std::expected<std::span<const Token>, std::string> {
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
                    [&](const TokNumLit&) -> void {
                        idx++;
                    },
                    [&](const TokBoolLit&) -> void {
                        idx++;
                    },
                    [&](const TokOp&) -> void {
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
    [[nodiscard]] auto add_cond_node(const std::vector<Token>& tokens, unsigned& idx, const Tok& t)
        -> std::unique_ptr<Node> {
        idx++;

        const auto expr = build_expr<TokColon>(tokens, idx);
        if (expr) {
            return std::make_unique<T>(t, *expr);
        }

        std::println(std::cerr, "{}", expr.error());
        return nullptr;
    }

    void generate_nodes(const std::vector<Token>& tokens) {
        unsigned idx = 0;
        while (idx < tokens.size()) {
            const auto& token = tokens.at(idx);
            std::visit(
                Overload{
                    [&](const TokDollarSign& t) {
                        idx++;
                        if (const auto expr = build_expr<TokNewline>(tokens, idx); expr) {
                            nodes.push_back(std::make_unique<NodeExpr>(t, *expr));
                            nodes_w_expr.push_back(nodes.back().get());
                        } else {
                            errors.push_back(expr.error());
                            std::println(std::cerr, "{}", expr.error());
                        }
                    },
                    [&](const TokShow& t) {
                        idx++;
                        std::optional<std::string> attr = std::nullopt;
                        std::optional<std::string> pos = std::nullopt;
                        std::optional<std::string> trans = std::nullopt;

                        if (auto ident = expect_tok<TokIdent>(tokens, idx)) {
                            std::string name;
                            std::optional<unsigned> pos_idx;
                            std::optional<unsigned> transition_idx;

                            name = ident->name;
                            if (auto attr_tok = expect_tok<TokIdent>(tokens, idx)) {
                                attr = attr_tok->name;
                            }

                            while (!H_A(TokNewline, tokens.at(idx))) {
                                if (const auto pos_tok = expect_tok<TokPos>(tokens, idx)) {
                                    if (!pos_idx) {
                                        pos_idx = idx;
                                        if (auto pos_desc = expect_tok<TokIdent>(tokens, idx)) {
                                            pos = pos_desc->name;
                                        } else {
                                            std::println(std::cerr, "{}", pos_desc.error());
                                            return;
                                        }
                                    } else {
                                        std::println(
                                            std::cerr, "error: duplicate position in Show statement on line {}",
                                            t.line);
                                    }
                                }

                                if (const auto trans_tok = expect_tok<TokTransition>(tokens, idx)) {
                                    if (!transition_idx) {
                                        transition_idx = idx;
                                        if (auto trans_desc = expect_tok<TokIdent>(tokens, idx)) {
                                            trans = trans_desc->name;
                                        } else {
                                            std::println(std::cerr, "{}", trans_desc.error());
                                            return;
                                        }
                                    } else {
                                        std::println(
                                            std::cerr,
                                            "error: duplicate transition in Show statement on line {}",
                                            t.line);
                                    }
                                }
                            }

                            if ((pos_idx && transition_idx) && *pos_idx >= *transition_idx) {
                                std::println(
                                    std::cerr,
                                    "error: position and transition for Show statement out of order on line {}",
                                    t.line);
                                return;
                            }

                            nodes.push_back(std::make_unique<NodeShow>(*ident, name, attr, pos, trans));
                        } else {
                            errors.push_back(ident.error());
                            std::println(std::cerr, "{}", ident.error());
                        }
                    },
                    [&](const TokHide& t) {
                        idx++;
                        if (auto name = expect_tok<TokIdent>(tokens, idx)) {
                            if (auto trans = expect_tok<TokTransition>(tokens, idx)) {
                                if (auto trans_desc = expect_tok<TokIdent>(tokens, idx)) {
                                    nodes.push_back(std::make_unique<NodeHide>(t, name->name, trans_desc->name));
                                } else {
                                    errors.push_back(trans_desc.error());
                                    std::println(std::cerr, "{}", trans_desc.error());
                                }
                            } else {
                                nodes.push_back(std::make_unique<NodeHide>(t, name->name, std::nullopt));
                            }
                        } else {
                            errors.push_back(name.error());
                            std::println(std::cerr, "{}", name.error());
                        }
                    },
                    [&](const TokMenu& t) {
                        idx++;
                        std::optional<std::string> set = std::nullopt;
                        std::optional<std::string> text;
                        if (auto colon = expect_tok<TokColon>(tokens, idx); !colon) {
                            errors.push_back(colon.error());
                            std::println(std::cerr, "{}", colon.error());
                            return;
                        }

                        while (H_A(TokNewline, tokens.at(idx)) || H_A(TokTab, tokens.at(idx))) {
                            idx++;
                        }

                        if (const auto set_tok = expect_tok<TokSet>(tokens, idx)) {
                            if (auto ident = expect_tok<TokIdent>(tokens, idx);
                                ident && set_tok->indent == t.indent + 1) {
                                set = ident->name;
                                while (!H_A(TokNewline, tokens.at(idx)) && !H_A(TokTab, tokens.at(idx))) {
                                    idx++;
                                }
                            } else {
                                errors.push_back(ident.error());
                                std::println(std::cerr, "{}", ident.error());
                                return;
                            }
                        }

                        std::unique_ptr<NodeChoice> choice = nullptr;

                        if (const auto str_tok = expect_tok<TokStrLit>(tokens, idx)) {
                            if (const auto newline = expect_tok<TokNewline>(tokens, idx);
                                newline && str_tok->indent == t.indent + 1) {
                                text = str_tok->text;
                            } else if (const auto colon = expect_tok<TokColon>(tokens, idx)) {
                                choice = std::make_unique<NodeChoice>(*colon, str_tok->text);
                            } else {
                                errors.push_back(newline.error());
                                std::println(std::cerr, "{}", newline.error());
                                return;
                            }
                        }

                        nodes.push_back(std::make_unique<NodeMenu>(t, text, set));
                        if (choice != nullptr) {
                            nodes.push_back(std::move(choice));
                        }
                    },
                    [&](const TokLabel& t) {
                        idx++;
                        auto ident = expect_tok<TokIdent>(tokens, idx);
                        if (!ident) {
                            errors.push_back(ident.error());
                            std::println(std::cerr, "{}", ident.error());
                            return;
                        }
                        auto colon = expect_tok<TokColon>(tokens, idx);
                        if (!colon) {
                            errors.push_back(colon.error());
                            std::println(std::cerr, "{}", colon.error());
                            return;
                        }
                        nodes.push_back(std::make_unique<NodeLabel>(t, ident->name));
                    },
                    [&](const TokScene& t) {
                        idx++;
                        if (const auto ident = expect_tok<TokIdent>(tokens, idx)) {
                            nodes.push_back(std::make_unique<NodeScene>(t, ident->name));
                        } else {
                            errors.push_back(ident.error());
                            std::println(std::cerr, "{}", ident.error());
                        }
                    },
                    [&](const TokIdent& t) {
                        idx++;
                        if (const auto str_lit = expect_tok<TokStrLit>(tokens, idx)) {
                            nodes.push_back(std::make_unique<NodeDialogue>(t, t.name, str_lit->text));
                        } else {
                            errors.push_back(str_lit.error());
                            std::println(std::cerr, "{}", str_lit.error());
                        }
                    },
                    [&](const TokStrLit& t) {
                        idx++;
                        if (const auto colon = expect_tok<TokColon>(tokens, idx)) {
                            nodes.push_back(std::make_unique<NodeChoice>(t, t.text));
                        } else if (const auto newline = expect_tok<TokNewline>(tokens, idx)) {
                            nodes.push_back(std::make_unique<NodeDialogue>(t, t.text));
                        } else {
                            errors.push_back(newline.error());
                            std::println(std::cerr, "{}", newline.error());
                        }
                    },
                    [&](const TokDefault &t) {
                        idx++;
                        if (const auto expr = build_expr<TokNewline>(tokens, idx); expr) {
                            unsigned e_idx = 0;
                            auto new_expr = fold_into_expr(*expr, e_idx);
                            if (is_valid_assign(new_expr.get())) {
                                nodes.push_back(std::make_unique<NodeExpr>(t, *expr, std::move(new_expr), false));
                                nodes_w_expr.push_back(nodes.back().get());
                            } else {
                                const std::string err = std::format("invalid Default declaration at {}", tok_pos(t));
                                errors.push_back(err);
                            }
                        } else {
                            errors.push_back(expr.error());
                            std::println(std::cerr, "{}", expr.error());
                        }
                    },
                    [&](const TokDefine &t) {
                        idx++;
                        if (const auto expr = build_expr<TokNewline>(tokens, idx); expr) {
                            unsigned e_idx = 0;
                            auto new_expr = fold_into_expr(*expr, e_idx);
                            if (is_valid_assign(new_expr.get())) {
                                nodes.push_back(std::make_unique<NodeExpr>(t, *expr, std::move(new_expr), true));
                                nodes_w_expr.push_back(nodes.back().get());
                            } else {
                                const std::string err = std::format("invalid Default declaration at {}", tok_pos(t));
                                errors.push_back(err);
                            }
                        } else {
                            errors.push_back(expr.error());
                            std::println(std::cerr, "{}", expr.error());
                        }
                    },
                    [&](const TokPlay& t) {
                        idx++;
                        AudioChannel channel;
                        if (const auto music = expect_tok<TokMusic>(tokens, idx)) {
                            channel = AudioChannel::Music;
                        } else if (const auto sfx = expect_tok<TokSfx>(tokens, idx)) {
                            channel = AudioChannel::Sfx;
                        } else {
                            errors.push_back(music.error());
                            std::println(std::cerr, "{}", music.error());
                        }

                        if (const auto path = expect_tok<TokStrLit>(tokens, idx)) {
                            nodes.push_back(std::make_unique<NodePlay>(t, channel, path->text));
                        } else {
                            errors.push_back(path.error());
                            std::println(std::cerr, "{}", path.error());
                        }
                    },
                    [&](const TokIf& t) {
                        if (auto if_node = add_cond_node<NodeIf>(tokens, idx, t)) {
                            nodes.push_back(std::move(if_node));
                            nodes_w_expr.push_back(nodes.back().get());
                        }
                    },
                    [&](const TokElif& t) {
                        if (auto elif_node = add_cond_node<NodeElif>(tokens, idx, t)) {
                            nodes.push_back(std::move(elif_node));
                            nodes_w_expr.push_back(nodes.back().get());
                        }
                    },
                    [&](const TokElse& t) {
                        idx++;
                        if (const auto colon = expect_tok<TokColon>(tokens, idx)) {
                            nodes.push_back(std::make_unique<NodeElse>(t));
                        } else {
                            errors.push_back(colon.error());
                            std::println(std::cerr, "{}", colon.error());
                        }
                    },
                    [&](const TokWhile& t) {
                        if (auto while_node = add_cond_node<NodeWhile>(tokens, idx, t)) {
                            nodes.push_back(std::move(while_node));
                            nodes_w_expr.push_back(nodes.back().get());
                        }
                    },
                    [&](const TokReturn& t) {
                        idx++;
                        if (const auto expr = build_expr<TokNewline>(tokens, idx)) {
                            if (expr->empty()) {
                                nodes.push_back(std::make_unique<NodeReturn>(t));
                            } else {
                                nodes.push_back(std::make_unique<NodeReturn>(t, *expr));
                            }
                            nodes_w_expr.push_back(nodes.back().get());
                        } else {
                            errors.push_back(expr.error());
                            std::println(std::cerr, "{}", expr.error());
                        }
                    },
                    [&](const TokCall& t) {
                        idx++;
                        if (const auto ident = expect_tok<TokIdent>(tokens, idx)) {
                            nodes.push_back(std::make_unique<NodeCall>(t, ident->name));
                        } else {
                            errors.push_back(ident.error());
                            std::println(std::cerr, "{}", ident.error());
                        }
                    },
                    [&](const TokJump& t) {
                        idx++;
                        if (const auto ident = expect_tok<TokIdent>(tokens, idx)) {
                            nodes.push_back(std::make_unique<NodeCall>(t, ident->name));
                        } else {
                            errors.push_back(ident.error());
                            std::println(std::cerr, "{}", ident.error());
                        }
                    },
                    [&](const TokCharacter &t) {
                        idx++;
                    },
                    [&](const TokImage &t) {
                        idx++;
                        const auto name = expect_tok<TokIdent>(tokens, idx);
                        if (!name) {
                            errors.push_back(name.error());
                            std::println(std::cerr, "{}", name.error());
                            return;
                        }
                        const auto attr = expect_tok<TokIdent>(tokens, idx);
                        if (!attr) {
                            errors.push_back(attr.error());
                            std::println(std::cerr, "{}", attr.error());
                            return;
                        }
                        const auto file_path = expect_tok<TokStrLit>(tokens, idx);
                        if (!file_path) {
                            errors.push_back(file_path.error());
                            std::println(std::cerr, "{}", file_path.error());
                            return;
                        }
                        nodes.push_back(std::make_unique<NodeImage>(t, name->name, attr->name, file_path->text));
                    },
                    [&](const TokNewline&) {
                    },
                    [&](const TokTab&) {
                    },
                    [&]<typename U>(U&& other) {
                        using To = std::decay_t<U>;
                        static_assert(std::is_base_of_v<Tok, To>, "expected derived from base Tok");
                        std::string msg = std::format("unexpected token {} at {}", tok_name<To>(), tok_pos(other));
                        errors.push_back(msg);
                        std::println(std::cerr, "{}", msg);
                    },
                }, token);
            idx++;
        }

        std::println("--------------------");
        if (errors.empty()) {
            std::println("parsing script OK!");
            connect_nodes();
            connect_nexts();
        } else {
            std::println(std::cerr, "Parsing script encountered {} error(s):", errors.size());
            for (const auto& error : errors) {
                std::println(std::cerr, "\t{}", error);
            }
        }
        std::println("--------------------");
    }

public:
    explicit Graph(const std::vector<Token>& tokens);

    auto get_nodes() -> std::vector<std::unique_ptr<Node>>&;

    auto get_roots() -> std::vector<unsigned>&;

    void print_as_graph();

    void print_all_nodes() const;

    auto graph_strs() -> std::vector<std::string>;
};


#endif //RPY_PROJ_ANALYZER_GRAPH_HPP
