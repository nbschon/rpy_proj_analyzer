//
// Created by Noah Schonhorn on 12/2/25.
//

#include "Graph.hpp"

#include <algorithm>
#include <format>
#include <functional>
#include <unordered_map>

template<class T, class... Ts>
constexpr bool is_val = (std::is_same_v<T, Ts> || ...);

void Graph::connect_nodes() const {
    std::vector<unsigned> parents; // treat like a stack
    std::unordered_map<unsigned, unsigned> last_child;

    for (unsigned i = 0; i < nodes.size(); ++i) {
        Node* curr = nodes.at(i).get();
        const unsigned indent = curr->indent;

        while (!parents.empty()) {
            if (Node* top = nodes.at(parents.back()).get(); top->indent < indent) {
                break;
            } else {
                auto* parent = static_cast<NodeParent*>(top);
                parent->after_block = i;
                parents.pop_back();
            }
        }

        if (!parents.empty()) { // if we have nodes
            if (Node* top = nodes.at(parents.back()).get()) { // if the top of the stack is valid
                if (top->has_children() && indent == top->indent + 1) { // if the top is a parent and
                                                                        // current indent is one after the top
                    curr->parent = parents.back(); // set the current node's parent to the active parent
                    if (auto* parent = static_cast<NodeParent*>(top); !parent->first_child) {
                        // if the parent does not have a child yet, set the child
                        parent->first_child = i;
                    } else if (parent->first_child) {
                        const unsigned prev = last_child.at(parents.back());
                        nodes.at(prev)->next = i;
                        curr->prev = prev;
                    }

                    last_child[parents.back()] = i;
                }
            }
        }

        if (curr->has_children()) {
            parents.push_back(i);
        }
    }

    while (!parents.empty()) {
        Node* top = nodes.at(parents.back()).get();
        auto* parent = static_cast<NodeParent*>(top);
        parent->after_block = nodes.size();
        parents.pop_back();
    }
}

void Graph::connect_nexts() const {
    std::unordered_map<unsigned, unsigned> last_at_indent;

    for (unsigned i = 0; i < nodes.size(); ++i) {
        const Node* curr = nodes.at(i).get();
        const auto indent = curr->indent;

        if (last_at_indent.contains(indent)) {
            const auto &last = last_at_indent.at(indent);
            bool connect = true;
            for (int j = last; j < i; ++j) {
                if (nodes.at(j)->indent < indent) {
                    connect = false;
                }
            }
            if (connect) {
                nodes.at(last_at_indent[indent])->next = i;
            }
        }

        last_at_indent[indent] = i;
    }
}

auto Graph::assign_scores(const unsigned idx, double curr_score, const OpType op) -> double {
    if (const auto *assign = dynamic_cast<NodeExpr *>(&*nodes.at(idx))) {
        // if (assign->op == op && H_A(double, assign->val)) {
        //     const double val = std::get<double>(assign->val);
        //     if (op == OpType::PlusEq) {
        //         curr_score += val;
        //     } else if (op == OpType::MinusEq) {
        //         curr_score -= val;
        //     }
        // }
    }

    double best = curr_score;

    // Each child starts from *this* node's current score
    // for (const auto &out_idx : nodes.at(idx)->next) {
    //     best = std::max(best, assign_scores(out_idx, curr_score, op));
    // }

    nodes.at(idx)->score_potential = static_cast<int>(best);
    return best;
}

auto Graph::add_show_node(const std::vector<Token>& tokens, const Tok& t, bool is_scene) -> std::unique_ptr<Node> {
    std::string name;
    std::vector<std::string> attrs;
    ShowProps props{};

    if (auto char_name = expect<TokIdent>(tokens)) {
        name = char_name->name;
    } else {
        errors.push_back(std::move(char_name.error()));
        std::println(std::cerr, "{}", errors.back());
        return nullptr;
    }

    while (std::holds_alternative<TokIdent>(tokens.at(idx))) {
        auto attr = expect<TokIdent>(tokens);
        // because we're already inside the loop, we already know this is valid
        attrs.push_back(attr->name);
    }

    if (const auto as_tok = expect<TokAs>(tokens)) {
        if (auto as_ident = expect<TokIdent>(tokens)) {
            props.as = as_ident->name;
        } else {
            errors.push_back(std::move(as_ident.error()));
            std::println(std::cerr, "{}", errors.back());
            return nullptr;
        }
    }

    if (const auto at_tok = expect<TokAt>(tokens)) {
        if (auto at_pos = expect<TokIdent>(tokens)) {
            props.at = at_pos->name;
        } else {
            errors.push_back(std::move(at_pos.error()));
            std::println(std::cerr, "{}", errors.back());
            return nullptr;
        }
    }

    if (const auto behind_tok = expect<TokBehind>(tokens)) {
        if (auto behind_list = expect<TokIdent>(tokens)) {
            props.behind = behind_list->name;
        } else {
            errors.push_back(std::move(behind_list.error()));
            std::println(std::cerr, "{}", errors.back());
            return nullptr;
        }
    }

    if (const auto onlayer_tok = expect<TokOnlayer>(tokens)) {
        if (auto layer = expect<TokIdent>(tokens)) {
            props.onlayer = layer->name;
        } else {
            errors.push_back(std::move(layer.error()));
            std::println(std::cerr, "{}", errors.back());
            return nullptr;
        }
    }

    if (const auto zorder_tok = expect<TokZOrder>(tokens)) {
        if (auto zorder = expect<TokIntLit>(tokens)) {
            props.zorder = zorder->value;
        } else {
            errors.push_back(std::move(zorder.error()));
            std::println(std::cerr, "{}", errors.back());
            return nullptr;
        }
    }

    if (const auto with_tok = expect<TokWith>(tokens)) {
        if (auto trans = expect<TokIdent>(tokens)) {
            props.trans = trans->name;
        } else {
            errors.push_back(std::move(trans.error()));
            std::println(std::cerr, "{}", errors.back());
            return nullptr;
        }
    }

    return std::make_unique<NodeShow>(t, name, attrs, props, is_scene);
}

void Graph::generate_nodes(const std::vector<Token>& tokens) {
    idx = 0;
    while (idx < tokens.size()) {
        const auto& token = tokens.at(idx);
        std::visit(
            Overload{
                [&](const TokDollarSign& t) {
                    idx++;
                    if (auto expr = expr_slice<TokNewline>(tokens); expr) {
                        nodes.push_back(std::make_unique<NodeExpr>(t, *expr));
                        nodes_w_expr.push_back(nodes.back().get());
                    } else {
                        errors.push_back(std::move(expr.error()));
                        std::println(std::cerr, "{}", errors.back());
                    }
                },
                [&](const TokShow& t) {
                    idx++;
                    if (auto show_node = add_show_node(tokens, t)) {
                        nodes.push_back(std::move(show_node));
                    }
                },
                [&](const TokScene& t) {
                    idx++;
                    if (auto scene_node = add_show_node(tokens, t, true)) {
                        nodes.push_back(std::move(scene_node));
                    }
                },
                [&](const TokHide& t) {
                    idx++;
                    std::string name;
                    HideProps props{};
                    if (auto char_name = expect<TokIdent>(tokens)) {
                        name = char_name->name;
                    } else {
                        errors.push_back(std::move(char_name.error()));
                        std::println(std::cerr, "{}", errors.back());
                        return;
                    }

                    if (const auto onlayer_tok = expect<TokOnlayer>(tokens)) {
                        if (auto layer = expect<TokIdent>(tokens)) {
                            props.onlayer = layer->name;
                        } else {
                            errors.push_back(std::move(layer.error()));
                            std::println(std::cerr, "{}", errors.back()) ;
                            return;
                        }
                    }

                    if (const auto with = expect<TokWith>(tokens)) {
                        if (auto trans = expect<TokIdent>(tokens)) {
                            props.trans = trans->name;
                        } else {
                            errors.push_back(std::move(trans.error()));
                            std::println(std::cerr, "{}", errors.back());
                            return;
                        }
                    }

                    nodes.emplace_back(std::make_unique<NodeHide>(t, name, props));
                },
                [&](const TokMenu& t) {
                    idx++;
                    std::optional<std::string> set = std::nullopt;
                    std::optional<std::string> text;
                    if (auto colon = expect<TokColon>(tokens); !colon) {
                        errors.push_back(std::move(colon.error()));
                        std::println(std::cerr, "{}", errors.back());
                        return;
                    }

                    while (H_A(TokNewline, tokens.at(idx)) || H_A(TokTab, tokens.at(idx))) {
                        idx++;
                    }

                    if (const auto set_tok = expect<TokSet>(tokens)) {
                        if (auto ident = expect<TokIdent>(tokens);
                            ident && set_tok->indent == t.indent + 1) {
                            set = ident->name;
                            while (!H_A(TokNewline, tokens.at(idx)) && !H_A(TokTab, tokens.at(idx))) {
                                idx++;
                            }
                        } else {
                            errors.push_back(std::move(ident.error()));
                            std::println(std::cerr, "{}", errors.back());
                            return;
                        }
                    }

                    std::unique_ptr<NodeChoice> choice = nullptr;

                    // special case: if the menu has a second line, it's either
                    // a say statement or a choice.
                    if (const auto str_tok = expect<TokStrLit>(tokens)) {
                        if (auto newline = expect<TokNewline>(tokens);
                            newline && str_tok->indent == t.indent + 1) {
                            text = str_tok->text;
                        } else if (const auto colon = expect<TokColon>(tokens)) {
                            choice = std::make_unique<NodeChoice>(*colon, str_tok->text);
                        } else if (const auto if_tok = expect<TokIf>(tokens)) {
                            if (auto slice = expr_slice<TokColon>(tokens)) {
                                choice = std::make_unique<NodeChoice>(*if_tok, str_tok->text, *slice);
                            } else {
                                errors.push_back(std::move(slice.error()));
                                std::println(std::cerr, "{}", errors.back());
                                return;
                            }
                        } else {
                            errors.push_back(std::move(newline.error()));
                            std::println(std::cerr, "{}", errors.back());
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
                    auto ident = expect<TokIdent>(tokens);
                    if (!ident) {
                        errors.push_back(std::move(ident.error()));
                        std::println(std::cerr, "{}", errors.back());
                        return;
                    }

                    if (auto colon = expect<TokColon>(tokens); !colon) {
                        errors.push_back(std::move(colon.error()));
                        std::println(std::cerr, "{}", errors.back());
                        return;
                    }
                    nodes.push_back(std::make_unique<NodeLabel>(t, ident->name));
                },
                [&](const TokIdent &t) {
                    idx++;
                    if (auto str_lit = expect<TokStrLit>(tokens)) {
                        nodes.push_back(std::make_unique<NodeDialogue>(t, t.name, str_lit->text));
                    } else {
                        errors.push_back(std::move(str_lit.error()));
                        std::println(std::cerr, "{}", errors.back());
                    }
                },
                [&](const TokStrLit &t) {
                    idx++;
                    if (const auto colon = expect<TokColon>(tokens)) {
                        nodes.push_back(std::make_unique<NodeChoice>(t, t.text));
                    } else if (const auto if_tok = expect<TokIf>(tokens)) {
                        if (auto slice = expr_slice<TokColon>(tokens)) {
                            nodes.push_back(std::make_unique<NodeChoice>(*if_tok, t.text, *slice));
                        } else {
                            errors.push_back(std::move(slice.error()));
                            std::println(std::cerr, "{}", errors.back());
                        }
                    }
                    else if (auto newline = expect<TokNewline>(tokens)) {
                        nodes.push_back(std::make_unique<NodeDialogue>(t, t.text));
                    } else {
                        errors.push_back(std::move(newline.error()));
                        std::println(std::cerr, "{}", errors.back());
                    }
                },
                [&](const TokDefault &t) {
                    idx++;
                    if (auto expr = expr_slice<TokNewline>(tokens)) {
                        unsigned e_idx = 0;
                        auto new_expr = fold_into_expr(*expr, e_idx);
                        if (is_valid_assign(new_expr.get())) {
                            nodes.push_back(std::make_unique<NodeExpr>(t, *expr, std::move(new_expr), false));
                            nodes_w_expr.push_back(nodes.back().get());
                        } else {
                            errors.emplace_back(std::format("invalid Default declaration at {}", tok_pos(t)));
                        }
                    } else {
                        errors.push_back(std::move(expr.error()));
                        std::println(std::cerr, "{}", errors.back());
                    }
                },
                [&](const TokDefine &t) {
                    idx++;
                    if (auto expr = expr_slice<TokNewline>(tokens)) {
                        unsigned e_idx = 0;
                        auto new_expr = fold_into_expr(*expr, e_idx);
                        if (is_valid_assign(new_expr.get())) {
                            nodes.push_back(std::make_unique<NodeExpr>(t, *expr, std::move(new_expr), true));
                            nodes_w_expr.push_back(nodes.back().get());
                        } else {
                            errors.emplace_back(std::format("invalid Define declaration at {}", tok_pos(t)));
                        }
                    } else {
                        errors.push_back(std::move(expr.error()));
                        std::println(std::cerr, "{}", errors.back());
                    }
                },
                [&](const TokPlay& t) {
                    idx++;
                    AudioChannel channel;
                    if (auto music = expect<TokMusic>(tokens)) {
                        channel = AudioChannel::Music;
                    } else if (const auto sfx = expect<TokSfx>(tokens)) {
                        channel = AudioChannel::Sfx;
                    } else {
                        errors.push_back(std::move(music.error()));
                        std::println(std::cerr, "{}", errors.back());
                    }

                    if (auto path = expect<TokStrLit>(tokens)) {
                        nodes.push_back(std::make_unique<NodePlay>(t, channel, path->text));
                    } else {
                        errors.push_back(std::move(path.error()));
                        std::println(std::cerr, "{}", errors.back());
                    }
                },
                [&](const TokIf& t) {
                    if (auto if_node = add_cond_node<NodeIf>(tokens, t)) {
                        nodes.push_back(std::move(if_node));
                        nodes_w_expr.push_back(nodes.back().get());
                    }
                },
                [&](const TokElif& t) {
                    if (auto elif_node = add_cond_node<NodeElif>(tokens, t)) {
                        nodes.push_back(std::move(elif_node));
                        nodes_w_expr.push_back(nodes.back().get());
                    }
                },
                [&](const TokElse& t) {
                    idx++;
                    if (auto colon = expect<TokColon>(tokens)) {
                        nodes.push_back(std::make_unique<NodeElse>(t));
                    } else {
                        errors.push_back(std::move(colon.error()));
                        std::println(std::cerr, "{}", errors.back());
                    }
                },
                [&](const TokWhile& t) {
                    if (auto while_node = add_cond_node<NodeWhile>(tokens, t)) {
                        nodes.push_back(std::move(while_node));
                        nodes_w_expr.push_back(nodes.back().get());
                    }
                },
                [&](const TokReturn& t) {
                    idx++;
                    if (auto expr = expr_slice<TokNewline>(tokens)) {
                        if (expr->empty()) {
                            nodes.push_back(std::make_unique<NodeReturn>(t));
                        } else {
                            nodes.push_back(std::make_unique<NodeReturn>(t, *expr));
                        }
                        nodes_w_expr.push_back(nodes.back().get());
                    } else {
                        errors.push_back(std::move(expr.error()));
                        std::println(std::cerr, "{}", errors.back());
                    }
                },
                [&](const TokCall& t) {
                    idx++;
                    if (auto ident = expect<TokIdent>(tokens)) {
                        nodes.push_back(std::make_unique<NodeCall>(t, ident->name));
                    } else {
                        errors.push_back(std::move(ident.error()));
                        std::println(std::cerr, "{}", errors.back());
                    }
                },
                [&](const TokJump& t) {
                    idx++;
                    if (auto ident = expect<TokIdent>(tokens)) {
                        nodes.push_back(std::make_unique<NodeCall>(t, ident->name));
                    } else {
                        errors.push_back(std::move(ident.error()));
                        std::println(std::cerr, "{}", errors.back());
                    }
                },
                [&](const TokImage &t) {
                    idx++;
                    std::vector<std::string> attrs;
                    auto name = expect<TokIdent>(tokens);
                    if (!name) {
                        errors.push_back(std::move(name.error()));
                        std::println(std::cerr, "{}", errors.back());
                        return;
                    }

                    while (!std::holds_alternative<TokOp>(tokens.at(idx))) {
                        auto attr = expect<TokIdent>(tokens);
                        if (!attr) {
                            errors.push_back(std::move(attr.error()));
                            std::println(std::cerr, "{}", errors.back());
                            return;
                        }
                        attrs.push_back(attr->name);
                    }

                    auto assign = expect<TokOp>(tokens);
                    if (!assign || assign->type != OpType::Assign) {
                        errors.push_back(std::move(assign.error()));
                        std::println(std::cerr, "{}", errors.back());
                        return;
                    }

                    auto file_path = expect<TokStrLit>(tokens);
                    if (!file_path) {
                        errors.push_back(std::move(file_path.error()));
                        std::println(std::cerr, "{}", errors.back());
                        return;
                    }
                    nodes.push_back(std::make_unique<NodeImage>(t, name->name, std::move(attrs), file_path->text));
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

Graph::Graph(const std::vector<Token>& tokens) {
    generate_nodes(tokens);
}

auto Graph::get_nodes() -> std::vector<std::unique_ptr<Node> > & {
    return nodes;
}

auto Graph::get_roots() -> std::vector<unsigned>& {
    return roots;
}

void Graph::print_as_graph() {
    visited = std::vector<bool>(nodes.size());
    const std::function<void(unsigned)> dfs_pre = [&](const unsigned node_idx) -> void {
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
        std::println("{:t}", node);

        // for (const auto &idx : node.next) {
        //     dfs_pre(idx);
        // }
    };

    for (const auto &idx: roots) {
        dfs_pre(idx);
    }
}

void Graph::print_all_nodes() const {
    for (const auto &n : nodes) {
        std::println("{}", *n);

        // if (const auto *chain = dynamic_cast<NodeIfChain *>(n.get())) {
            // for (const auto &o : chain->children) {
            //     std::println("\t{}", *nodes.at(o));
            // }
        // }
    }
}

auto Graph::graph_strs() -> std::vector<std::string> {
    visited = std::vector<bool>(nodes.size());
    std::function<void(unsigned node)> dfs_pre;
    std::vector<std::string> strs;
    dfs_pre = [&](const unsigned node_idx) -> void {
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
        strs.push_back(std::format("{:t}", node));

        if (!node.has_children() && node.next) {
            dfs_pre(*node.next);
        } else if (node.has_children()) {
            auto* parent = dynamic_cast<const NodeParent*>(&node);
            // for (const auto &idx : parent->children) {
            //     dfs_pre(idx);
            // }
        }
    };
    for (const auto &idx: roots) {
        dfs_pre(idx);
    }

    return strs;
}
