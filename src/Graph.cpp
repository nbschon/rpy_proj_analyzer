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

/* unsigned max_indent = 0;
for (const auto &n: nodes) {
    if (n) {
        max_indent = std::max(max_indent, n->indent);
    }
}

std::vector<std::optional<unsigned>> last_at_indent(max_indent + 1);
std::vector<unsigned> root_nodes;

for (int i = 0; i < nodes.size(); ++i) {
    const Node *curr_node = nodes.at(i).get();
    const auto curr_ind = curr_node->indent;
    if (curr_node != nullptr) {
        std::optional<unsigned> prev;
        for (int j = static_cast<int>(curr_ind); j >= 0; --j) {
            if (j < last_at_indent.size() && last_at_indent.at(j)) {
                prev = last_at_indent.at(j);
                break;
            }
        }

        if (prev && !nodes.at(*prev)->next) {
            nodes.at(*prev)->next = i;
        } else {
            root_nodes.push_back(i);
        }

        last_at_indent.at(curr_ind) = i;

        for (auto j = curr_ind + 1; j < last_at_indent.size(); ++j) {
            last_at_indent.at(j).reset();
        }
    }
}

for (const auto &r: root_nodes) {
    assign_scores(r, 0.0, OpType::PlusEq);
}
this->roots = std::move(root_nodes); */

// void Graph::connect_ins() const {
//     for (unsigned i = 0; i < nodes.size(); ++i) {
//         if (auto &next = nodes.at(i)->next; next) {
//             nodes.at(*next)->in.insert(i);
//         }
//     }
// }

// void Graph::group_conditionals() {
//     std::vector<std::pair<Node*, unsigned>> if_refs;
//     std::vector<unsigned> orig_ifs;
//
//     for (unsigned i = 0; i < nodes.size(); ++i) {
//         if (auto *if_ptr = dynamic_cast<NodeIf*>(nodes.at(i).get())) {
//             if_refs.emplace_back(if_ptr, i);
//             orig_ifs.push_back(i);
//         }
//     }
//
//     std::vector<unsigned> ifs_as_chains;
//
//     // this loop works, but is dumb
//     for (const auto [ptr, idx]: if_refs) {
//         unsigned if_idx = 0;
//         std::vector<unsigned> elif_idxs;
//         std::optional<unsigned> else_idx = std::nullopt;
//
//         for (unsigned i = idx + 1; i < nodes.size(); ++i) {
//             Node *node = nodes.at(i).get();
//             if_idx = idx;
//             if (node->indent == ptr->indent) {
//                 if (dynamic_cast<NodeIf *>(node) != nullptr) {
//                     ifs_as_chains.push_back(nodes.size());
//                     nodes.emplace_back(std::make_unique<NodeIfChain>(Tok{}, if_idx, elif_idxs, else_idx));
//                     break;
//                 }
//                 if (dynamic_cast<NodeElif *>(node) != nullptr) {
//                     elif_idxs.push_back(i);
//                 } else if (dynamic_cast<NodeElse *>(node) != nullptr) {
//                     else_idx = i;
//                     ifs_as_chains.push_back(nodes.size());
//                     nodes.emplace_back(std::make_unique<NodeIfChain>(Tok{}, if_idx, elif_idxs, else_idx));
//                     break;
//                 }
//             }
//         }
//     }
//
//     assert(orig_ifs.size() == ifs_as_chains.size());
//     for (int i = 0; i < orig_ifs.size(); ++i) {
//         const auto orig_if_idx = orig_ifs.at(i);
//         const auto if_chain_idx = ifs_as_chains.at(i);
//
//         Node &orig_if_ref = *nodes.at(orig_if_idx);
//         Node &if_chain_ref = *nodes.at(if_chain_idx);
//
//         for (const auto &idx: orig_if_ref.in) {
//             if_chain_ref.in.insert(idx);
//         }
//         orig_if_ref.in.clear();
//         orig_if_ref.in.insert(ifs_as_chains.at(i));
//
//         // for (const auto &idx : if_chain_ref.next) {
//         //     nodes.at(idx)->in.erase(orig_if_idx);
//         //     nodes.at(idx)->in.insert(if_chain_idx);
//         // }
//
//         // for (const auto &idx: if_chain_ref.in) {
//         //     nodes.at(idx)->next.erase(orig_if_idx);
//         //     nodes.at(idx)->next.insert(if_chain_idx);
//         // }
//     }
// }

// void Graph::group_menus() {
//     for (int i = 0; i < nodes.size(); ++i) {
//         if (auto* menu_ptr = dynamic_cast<NodeMenu*>(nodes.at(i).get())) {
//
//         }
//     }
//
// }

// void Graph::find_widths() const {
//     /**
//      * this dfs is supposed to propagate the width needed for drawing up
//      * the graph / tree / whatever you wanna call it
//      */
//     std::function<unsigned(unsigned node_idx)> dfs_post = [&](const unsigned node_idx) -> unsigned {
//         // std::vector<unsigned> widths;
//         // widths.reserve(nodes.at(node_idx)->next.size());
//         // for (const auto &idx : nodes.at(node_idx)->next) {
//         //     dfs_post(idx);
//         // }
//         // const int width = std::ranges::fold_left(
//         //     nodes.at(node_idx)->next,
//         //     0,
//         //     [&](int acc, unsigned idx) {
//         //         return acc + nodes.at(idx)->next.size();
//         //     }
//         //     );
//         // nodes.at(node_idx)->width = width;
//         auto &n = *nodes.at(node_idx);
//
//         // if (n.next.empty()) {
//         //     n.width = 1;
//         //     return 1;
//         // }
//         //
//         // unsigned width = 0;
//         // for (auto& idx : n.next) {
//         //     width += dfs_post(idx);
//         // }
//
//         // n.width = width;
//         // return width;
//     };
//
//     for (auto& root : roots) {
//         dfs_post(root);
//     }
// }

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

void Graph::generate_nodes(const std::vector<Token>& tokens) {
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
                    std::string name;
                    std::vector<std::string> attrs;
                    ShowProps props{};

                    if (const auto char_name = expect_tok<TokIdent>(tokens, idx)) {
                        name = char_name->name;
                    } else {
                        errors.push_back(char_name.error());
                        std::println(std::cerr, "{}", char_name.error());
                        return;
                    }

                    while (std::holds_alternative<TokIdent>(tokens.at(idx))) {
                        auto attr = expect_tok<TokIdent>(tokens, idx);
                        // because we're already inside the loop, we already know this is valid
                        attrs.push_back(attr->name);
                    }

                    if (auto as_tok = expect_tok<TokAs>(tokens, idx)) {
                        if (const auto as_ident = expect_tok<TokIdent>(tokens, idx)) {
                            props.as = as_ident->name;
                        } else {
                            errors.push_back(as_ident.error());
                            std::println(std::cerr, "{}", as_ident.error());
                            return;
                        }
                    }

                    if (auto at_tok = expect_tok<TokAt>(tokens, idx)) {
                        if (const auto at_pos = expect_tok<TokIdent>(tokens, idx)) {
                            props.at = at_pos->name;
                        } else {
                            errors.push_back(at_pos.error());
                            std::println(std::cerr, "{}", at_pos.error());
                            return;
                        }
                    }

                    if (auto behind_tok = expect_tok<TokBehind>(tokens, idx)) {
                        if (const auto behind_list = expect_tok<TokIdent>(tokens, idx)) {
                            props.behind = behind_list->name;
                        } else {
                            errors.push_back(behind_list.error());
                            std::println(std::cerr, "{}", behind_list.error());
                            return;
                        }
                    }

                    if (auto onlayer_tok = expect_tok<TokOnlayer>(tokens, idx)) {
                        if (const auto layer = expect_tok<TokIdent>(tokens, idx)) {
                            props.onlayer = layer->name;
                        } else {
                            errors.push_back(layer.error());
                            std::println(std::cerr, "{}", layer.error());
                            return;
                        }
                    }

                    if (auto zorder_tok = expect_tok<TokZOrder>(tokens, idx)) {
                        if (const auto zorder = expect_tok<TokNumLit>(tokens, idx)) {
                            props.zorder = zorder->value;
                        } else {
                            errors.push_back(zorder.error());
                            std::println(std::cerr, "{}", zorder.error());
                            return;
                        }
                    }

                    if (auto with_tok = expect_tok<TokWith>(tokens, idx)) {
                        if (const auto trans = expect_tok<TokIdent>(tokens, idx)) {
                            props.trans = trans->name;
                        } else {
                            errors.push_back(trans.error());
                            std::println(std::cerr, "{}", trans.error());
                            return;
                        }
                    }

                    nodes.emplace_back(std::make_unique<NodeShow>(t, name, attrs, props));
                },
                [&](const TokHide& t) {
                    idx++;
                    std::string name;
                    HideProps props{};
                    if (const auto char_name = expect_tok<TokIdent>(tokens, idx)) {
                        name = char_name->name;
                    } else {
                        errors.push_back(char_name.error());
                        std::println(std::cerr, "{}", char_name.error());
                        return;
                    }

                    if (const auto onlayer = expect_tok<TokOnlayer>(tokens, idx)) {
                        if (const auto layer = expect_tok<TokIdent>(tokens, idx)) {
                            props.onlayer = layer->name;
                        } else {
                            errors.push_back(layer.error());
                            std::println(std::cerr, "{}", layer.error());
                            return;
                        }
                    }

                    if (const auto with = expect_tok<TokWith>(tokens, idx)) {
                        if (const auto trans = expect_tok<TokIdent>(tokens, idx)) {
                            props.trans = trans->name;
                        } else {
                            errors.push_back(trans.error());
                            std::println(std::cerr, "{}", trans.error());
                            return;
                        }
                    }

                    nodes.emplace_back(std::make_unique<NodeHide>(t, name, props));
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
                            const std::string err = std::format("invalid Define declaration at {}", tok_pos(t));
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
                [&](const TokImage &t) {
                    idx++;
                    std::vector<std::string> attrs;
                    const auto name = expect_tok<TokIdent>(tokens, idx);
                    if (!name) {
                        errors.push_back(name.error());
                        std::println(std::cerr, "{}", name.error());
                        return;
                    }
                    while (!std::holds_alternative<TokOp>(tokens.at(idx))) {
                        const auto attr = expect_tok<TokIdent>(tokens, idx);
                        if (!attr) {
                            errors.push_back(attr.error());
                            std::println(std::cerr, "{}", attr.error());
                            return;
                        }
                        attrs.push_back(attr->name);
                    }
                    const auto assign = expect_tok<TokOp>(tokens, idx);
                    if (!assign || assign->type != OpType::Assign) {
                        errors.push_back(assign.error());
                        std::println(std::cerr, "{}", assign.error());
                        return;
                    }
                    const auto file_path = expect_tok<TokStrLit>(tokens, idx);
                    if (!file_path) {
                        errors.push_back(file_path.error());
                        std::println(std::cerr, "{}", file_path.error());
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
    const std::function dfs_pre = [&](const unsigned node_idx) -> void {
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
    for (const auto &n: nodes) {
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
