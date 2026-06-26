//
// Created by Noah Schonhorn on 12/2/25.
//

#include "Graph.hpp"

#include <algorithm>
#include <format>
#include <unordered_map>

#include "Typing.hpp"

void Graph::connect_ancestors() const {
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

        for (int i = 0; i < last_child.size(); ++i) {
            if (last_child[i] > indent) {
                last_child.erase(i);
                i--;
            }
        }

        if (last_child.contains(indent)) {
            const auto prev = last_child[indent];
            nodes.at(prev)->next = i;
            curr->prev = prev;
        }

        if (!parents.empty()) { // if we have nodes
            if (Node* top = nodes.at(parents.back()).get()) { // if the top of the stack is valid
                if (top->has_children() && indent == top->indent + 1) { // if top is a parent and
                                                                        // current indent is one after the top
                    curr->parent = parents.back(); // set the current node's parent to the active parent
                    if (auto* parent = static_cast<NodeParent*>(top); !parent->first_child) {
                        // if the parent does not have a child yet, set the child
                        parent->first_child = i;
                    // } else if (parent->first_child) {
                    //     const unsigned prev = last_child.at(parents.back());
                    //     nodes.at(prev)->next = i;
                    //     curr->prev = prev;
                    }
                }
            }
        }

        last_child[indent] = i;

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

    // nodes.at(idx)->score_potential = static_cast<int>(best);
    return best;
}

auto Graph::add_show_node(const Tok& tok, bool& has_atl, bool is_scene) -> std::unique_ptr<NodeShow> {
    std::string name;
    std::vector<std::string> attrs;
    ShowProps props{};

    if (auto char_name = lexer.expect<TokIdent>()) {
        name = char_name->name;
    } else {
        errors.push_back(std::move(char_name.error()));
        std::println(std::cerr, "{}", errors.back());
        return nullptr;
    }

    while (lexer.curr_is<TokIdent>()) {
        // because we're already inside the loop, we already know this is valid
        auto attr = lexer.expect<TokIdent>();
        attrs.push_back(attr->name);
    }

    if (const auto as_tok = lexer.expect<TokAs>()) {
        if (auto as_ident = lexer.expect<TokIdent>()) {
            props.as = as_ident->name;
        } else {
            errors.push_back(std::move(as_ident.error()));
            std::println(std::cerr, "{}", errors.back());
            return nullptr;
        }
    }

    if (const auto at_tok = lexer.expect<TokAt>()) {
        if (auto tf_tok = lexer.expect<TokIdent>()) {
            props.transforms.push_back(tf_tok->name);
            while (auto comma_tok = lexer.expect<TokComma>()) {
                if (auto next_tf = lexer.expect<TokIdent>()) {
                    props.transforms.push_back(next_tf->name);
                } else {
                    errors.push_back(std::move(next_tf.error()));
                    std::println(std::cerr, "{}", errors.back());
                    return nullptr;
                }
            }
        } else {
            errors.push_back(std::move(tf_tok.error()));
            std::println(std::cerr, "{}", errors.back());
            return nullptr;
        }
    }

    if (const auto behind_tok = lexer.expect<TokBehind>()) {
        if (auto behind_list = lexer.expect<TokIdent>()) {
            props.behind = behind_list->name;
        } else {
            errors.push_back(std::move(behind_list.error()));
            std::println(std::cerr, "{}", errors.back());
            return nullptr;
        }
    }

    if (const auto onlayer_tok = lexer.expect<TokOnlayer>()) {
        if (auto layer = lexer.expect<TokIdent>()) {
            props.onlayer = layer->name;
        } else {
            errors.push_back(std::move(layer.error()));
            std::println(std::cerr, "{}", errors.back());
            return nullptr;
        }
    }

    if (const auto zorder_tok = lexer.expect<TokZOrder>()) {
        if (auto zorder = lexer.expect<TokIntLit>()) {
            props.zorder = zorder->value;
        } else {
            errors.push_back(std::move(zorder.error()));
            std::println(std::cerr, "{}", errors.back());
            return nullptr;
        }
    }

    if (const auto colon = lexer.expect<TokColon>()) {
        while (lexer.curr_is<TokNewline, TokTab>()) {
            ++lexer;
        }

        props.atl_stmts = ATL::make_atl_block(lexer, colon->indent);
        has_atl = true;
    }

    // this is a bit of a hack, should probably be changed in the future
    if (lexer.curr_is<TokWith>()) {
        --lexer;
    }

    return std::make_unique<NodeShow>(tok, name, attrs, props, is_scene);
}

void Graph::generate_nodes() {
    while (lexer.has_more()) {
        const auto& token = lexer.curr();
        std::visit(Overload {
            [&](const TokDollarSign& t) {
                ++lexer;
                if (auto slice = expr_slice(lexer)) {
                    nodes.push_back(std::make_unique<NodeExpr>(t, *slice));
                    nodes_w_expr.push_back(nodes.back().get());
                } else {
                    errors.push_back(std::move(slice.error()));
                    std::println(std::cerr, "{}", errors.back());
                }
            },
            [&](const TokShow& t) {
                ++lexer;
                bool has_atl = false;
                if (auto show_node = add_show_node(t, has_atl)) {
                    nodes.push_back(std::move(show_node));
                }
                if (has_atl) {
                    nodes_w_atl.push_back(nodes.back().get());
                }
            },
            [&](const TokScene& t) {
                ++lexer;
                bool has_atl = false;
                if (auto scene_node = add_show_node(t, has_atl, true)) {
                    nodes.push_back(std::move(scene_node));
                }
                if (has_atl) {
                    nodes_w_atl.push_back(nodes.back().get());
                }
            },
            [&](const TokHide& t) {
                ++lexer;
                std::string name;
                std::optional<std::string> onlayer;
                if (auto char_name = lexer.expect<TokIdent>()) {
                    name = char_name->name;
                } else {
                    errors.push_back(std::move(char_name.error()));
                    std::println(std::cerr, "{}", errors.back());
                    return;
                }

                if (const auto onlayer_tok = lexer.expect<TokOnlayer>()) {
                    if (auto layer = lexer.expect<TokIdent>()) {
                        onlayer = layer->name;
                    } else {
                        errors.push_back(std::move(layer.error()));
                        std::println(std::cerr, "{}", errors.back()) ;
                        return;
                    }
                }

                // same hack as in `add_show_node`. should probably be changed.
                if (lexer.curr_is<TokWith>()) {
                    --lexer;
                }

                nodes.emplace_back(std::make_unique<NodeHide>(t, name, onlayer));
            },
            [&](const TokWith& t) {
                ++lexer;
                if (auto trans = lexer.expect<TokATLTransition>()) {
                    nodes.push_back(std::make_unique<NodeWith>(t, trans->trans));
                } else if (auto slice = expr_slice(lexer)) {
                    nodes.push_back(std::make_unique<NodeWith>(t, *slice));
                } else {
                    errors.push_back(lexer.multi_tok_error<TokATLTransition>({"valid expression"}));
                    std::println(std::cerr, "{}", errors.back());
                }
            },
            [&](const TokMenu& t) {
                ++lexer;
                std::optional<std::string> set = std::nullopt;
                std::optional<std::string> text;
                if (auto colon = lexer.expect<TokColon>(); !colon) {
                    errors.push_back(std::move(colon.error()));
                    std::println(std::cerr, "{}", errors.back());
                    return;
                }

                while (lexer.curr_is<TokNewline, TokTab>()) {
                    ++lexer;
                }

                if (const auto set_tok = lexer.expect<TokIdent>(); set_tok && set_tok->name == "set") {
                    if (auto ident = lexer.expect<TokIdent>();
                        ident && set_tok->indent == t.indent + 1) {
                        set = ident->name;
                        while (lexer.curr_is_not<TokNewline, TokTab>()) {
                            ++lexer;
                        }
                    } else {
                        errors.push_back(std::move(ident.error()));
                        std::println(std::cerr, "{}", errors.back());
                        return;
                    }
                }

                std::unique_ptr<NodeChoice> choice = nullptr;

                // special case: if the menu has a second line, it's
                // either a say statement or a choice.
                if (const auto str_tok = lexer.expect<TokStrLit>()) {
                    if (auto newline = lexer.expect<TokNewline>();
                        newline && str_tok->indent == t.indent + 1) {
                        text = str_tok->text;
                    } else if (const auto colon = lexer.expect<TokColon>()) {
                        choice = std::make_unique<NodeChoice>(*colon, str_tok->text);
                    } else if (const auto if_tok = lexer.expect<TokIf>()) {
                        if (auto slice = expr_slice(lexer);
                            slice && lexer.curr_is<TokColon>()) {
                            choice = std::make_unique<NodeChoice>(*if_tok, str_tok->text, *slice);
                        } else {
                            errors.push_back(std::move(slice.error()));
                            std::println(std::cerr, "{}", errors.back());
                            return;
                        }
                    } else {
                        errors.push_back(lexer.multi_tok_error<TokColon, TokIf, TokNewline>());
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
                ++lexer;
                auto ident = lexer.expect<TokIdent>();
                if (!ident) {
                    errors.push_back(std::move(ident.error()));
                    std::println(std::cerr, "{}", errors.back());
                    return;
                }

                if (auto colon = lexer.expect<TokColon>(); !colon) {
                    errors.push_back(std::move(colon.error()));
                    std::println(std::cerr, "{}", errors.back());
                    return;
                }
                nodes.push_back(std::make_unique<NodeLabel>(t, ident->name));
            },
            [&](const TokIdent &t) {
                ++lexer;
                if (auto str_lit = lexer.expect<TokStrLit>()) {
                    nodes.push_back(std::make_unique<NodeDialogue>(t, t.name, str_lit->text));
                } else {
                    errors.push_back(std::move(str_lit.error()));
                    std::println(std::cerr, "{}", errors.back());
                }
            },
            [&](const TokStrLit &t) {
                /*
                 * a few cases for string literals:
                 *  1. colon at the end? menu choice
                 *  2. if at the end? menu choice w/ expression
                 *  3. newline? normal dialogue
                 */
                ++lexer;
                if (const auto colon = lexer.expect<TokColon>()) {
                    nodes.push_back(std::make_unique<NodeChoice>(t, t.text));
                } else if (const auto if_tok = lexer.expect<TokIf>()) {
                    if (auto slice = expr_slice(lexer);
                        slice && lexer.curr_is<TokColon>()) {
                        nodes.push_back(std::make_unique<NodeChoice>(*if_tok, t.text, *slice));
                    } else {
                        errors.push_back(std::move(slice.error()));
                        std::println(std::cerr, "{}", errors.back());
                    }
                } else if (lexer.curr_is<TokNewline>()) {
                    nodes.push_back(std::make_unique<NodeDialogue>(t, t.text));
                } else {
                    errors.push_back(lexer.multi_tok_error<TokColon, TokIf, TokNewline>());
                    std::println(std::cerr, "{}", errors.back());
                }
            },
            [&](const TokDefault &t) {
                ++lexer;
                if (auto slice = expr_slice(lexer)) {
                    if (auto new_expr = try_get_expr(lexer)) {
                        if (is_valid_assign(new_expr->get())) {
                            nodes.push_back(std::make_unique<NodeExpr>(t, *slice, std::move(*new_expr), false));
                            nodes_w_expr.push_back(nodes.back().get());
                        } else {
                            errors.emplace_back(std::format("invalid Default declaration at {}", tok_pos(t)));
                        }
                    }
                } else {
                    errors.push_back(std::move(slice.error()));
                    std::println(std::cerr, "{}", errors.back());
                }
            },
            [&](const TokDefine &t) {
                ++lexer;
                if (auto slice = expr_slice(lexer)) {

                    if (auto new_expr = fold_into_expr(*slice); is_valid_assign(new_expr->get())) {
                        nodes.push_back(std::make_unique<NodeExpr>(t, *slice, std::move(*new_expr), true));
                        nodes_w_expr.push_back(nodes.back().get());
                    } else {
                        errors.emplace_back(std::format("invalid Define declaration at {}", tok_pos(t)));
                    }
                } else {
                    errors.push_back(std::move(slice.error()));
                    std::println(std::cerr, "{}", errors.back());
                }
            },
            [&](const TokPlay& t) {
                ++lexer;
                AudioChannel channel;
                if (auto music = lexer.expect<TokMusic>()) {
                    channel = AudioChannel::Music;
                } else if (const auto sfx = lexer.expect<TokSfx>()) {
                    channel = AudioChannel::Sfx;
                } else {
                    errors.push_back(lexer.multi_tok_error<TokMusic, TokSfx>());
                    std::println(std::cerr, "{}", errors.back());
                }

                if (auto path = lexer.expect<TokStrLit>()) {
                    nodes.push_back(std::make_unique<NodePlay>(t, channel, path->text));
                } else {
                    errors.push_back(std::move(path.error()));
                    std::println(std::cerr, "{}", errors.back());
                }
            },
            [&](const TokIf& t) {
                if (auto if_node = add_cond_node<NodeIf>(t)) {
                    nodes.push_back(std::move(if_node));
                    nodes_w_expr.push_back(nodes.back().get());
                }
            },
            [&](const TokElif& t) {
                if (auto elif_node = add_cond_node<NodeElif>(t)) {
                    nodes.push_back(std::move(elif_node));
                    nodes_w_expr.push_back(nodes.back().get());
                }
            },
            [&](const TokElse& t) {
                ++lexer;
                if (auto colon = lexer.expect<TokColon>()) {
                    nodes.push_back(std::make_unique<NodeElse>(t));
                } else {
                    errors.push_back(std::move(colon.error()));
                    std::println(std::cerr, "{}", errors.back());
                }
            },
            [&](const TokWhile& t) {
                if (auto while_node = add_cond_node<NodeWhile>(t)) {
                    nodes.push_back(std::move(while_node));
                    nodes_w_expr.push_back(nodes.back().get());
                }
            },
            [&](const TokReturn& t) {
                ++lexer;
                if (auto slice = expr_slice(lexer)) {
                    nodes.push_back(std::make_unique<NodeReturn>(t, *slice));
                    nodes_w_expr.push_back(nodes.back().get());
                } else {
                    nodes.push_back(std::make_unique<NodeReturn>(t));
                }
            },
            [&](const TokPass& t) {
                ++lexer;
                nodes.push_back(std::make_unique<NodePass>(t));
            },
            [&](const TokCall& t) {
                ++lexer;
                if (auto ident = lexer.expect<TokIdent>()) {
                    nodes.push_back(std::make_unique<NodeCall>(t, ident->name));
                } else {
                    errors.push_back(std::move(ident.error()));
                    std::println(std::cerr, "{}", errors.back());
                }
            },
            [&](const TokJump& t) {
                ++lexer;
                if (auto ident = lexer.expect<TokIdent>()) {
                    nodes.push_back(std::make_unique<NodeCall>(t, ident->name));
                } else {
                    errors.push_back(std::move(ident.error()));
                    std::println(std::cerr, "{}", errors.back());
                }
            },
            [&](const TokImage &t) {
                ++lexer;
                std::vector<std::string> attrs;
                auto name = lexer.expect<TokIdent>();
                if (!name) {
                    errors.push_back(std::move(name.error()));
                    std::println(std::cerr, "{}", errors.back());
                    return;
                }

                while (!lexer.curr_is<TokOp>()) {
                    auto attr = lexer.expect<TokIdent>();
                    if (!attr) {
                        errors.push_back(std::move(attr.error()));
                        std::println(std::cerr, "{}", errors.back());
                        return;
                    }
                    attrs.push_back(attr->name);
                }

                auto assign = lexer.expect<TokOp>();
                if (!assign || assign->type != OpType::Assign) {
                    errors.push_back(std::move(assign.error()));
                    std::println(std::cerr, "{}", errors.back());
                    return;
                }

                auto file_path = lexer.expect<TokStrLit>();
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
        ++lexer;
    }

    for (unsigned i = 0; i < nodes.size(); ++i) {
        if (nodes.at(i)->indent == 0) {
            roots.push_back(i);
        }
    }

    std::println("--------------------");
    if (errors.empty()) {
        std::println("parsing script OK!");
        connect_ancestors();
        connect_nexts();
        // auto wc = find_highest_wc_path();
        // std::println("max wc: {}", wc);
    } else {
        std::println(std::cerr, "Parsing script encountered {} error(s):", errors.size());
        for (const auto& error : errors) {
            std::println(std::cerr, "\t{}", error);
        }
    }
    std::println("--------------------");
    if (nodes_w_atl.empty()) {
        std::println("no nodes with ATL.");
    } else {
        for (const auto &n : nodes_w_atl) {
            std::println("{:p}", *n);
        }
    }
    if (nodes_w_expr.empty()) {
        std::println("no nodes with expr.");
    } else {
        for (const auto &n : nodes) {
            if (dynamic_cast<NodeExpr*>(n.get())) {
                // Typing::deduce_type(n.);
            }
        }
    }
    std::println("--------------------");
    int total_wc = 0;
    dfs<TrvOrd::Pre>([&](const Node &n) {
        if (auto dialogue = dynamic_cast<const NodeDialogue*>(&n)) {
            total_wc += dialogue->word_count;
        }
    });

    // std::vector<std::set<const Node*>> groups;
    // std::set<const Node*> visits;
    // dfs<TrvOrd::Pre>([&](const Node &n) {
    //     if (n.has_children() && !visits.contains(&n)) {
    //         std::set<const Node*> new_group;
    //         new_group.insert(&n);
    //         visits.insert(&n);
    //         std::optional<unsigned> next = n.next;
    //         while (next) {
    //             if (nodes.at(*next)->has_children() && !visits.contains(nodes.at(*next).get())) {
    //                 new_group.insert(nodes.at(*next).get());
    //                 visits.insert(nodes.at(*next).get());
    //                 next = nodes.at(*next)->next;
    //             } else {
    //                 break;
    //             }
    //         }
    //         groups.push_back(new_group);
    //     }
    // });
    // for (const auto &g : groups) {
    //     for (const auto &n : g) {
    //         std::print("{:p}", *n);
    //     }
    //     std::println("");
    // }
    std::println("total word count: {}", total_wc);
    std::println("--------------------");
}

auto Graph::find_highest_wc_path() const -> int {
    struct Memo {
        bool computed = false;
        bool visiting = false;
        int words = 0;
        std::optional<unsigned> next;
    };

    std::vector<Memo> memos(nodes.size());

    auto successors = [&](unsigned i) -> std::vector<unsigned> {
        const auto &n = nodes.at(i);
        if (const auto *menu = dynamic_cast<NodeMenu*>(n.get())) {
            std::vector<unsigned> choices;

            auto child = menu->first_child;
            while (child && *child < *menu->after_block) {
                if (dynamic_cast<NodeChoice*>(nodes.at(*child).get())) {
                    choices.push_back(*child);
                }

                if (!nodes.at(*child)->next) {
                    break;
                }

                child = nodes.at(*child)->next;
            }

            return choices;
        }

        if (dynamic_cast<NodeIf*>(n.get())) {
            std::vector<unsigned> branches;
            std::optional<unsigned> curr = i;

            while (curr) {
                const auto &branch = nodes.at(*curr);
                if (dynamic_cast<NodeIf*>(branch.get())
                    || dynamic_cast<NodeElif*>(branch.get())
                    || dynamic_cast<NodeElse*>(branch.get())) {
                    branches.push_back(*curr);
                } else {
                    break;
                }

                if (!branch->next) {
                    break;
                }

                curr = branch->next;
                if (dynamic_cast<NodeIf*>(nodes.at(*curr).get())) {
                    break;
                }
            }

            return branches;
        }

        if (const auto *parent = dynamic_cast<NodeParent*>(n.get())) {
            if (parent->first_child) {
                return {*parent->first_child};
            }
        }

        if (n->next) {
            return {*n->next};
        }

        return {};
    };

    auto best_from = [&](this auto self, unsigned i) -> int {
        auto &m = memos.at(i);

        if (m.computed) {
            return m.words;
        }

        if (m.visiting) {
            return 0;
        }
        m.visiting = true;

        int best_tail = 0;
        std::optional<unsigned> best_next;

        for (const auto &s : successors(i)) {
            if (const auto candidate = self(s); candidate > best_tail) {
                best_tail = candidate;
                best_next = s;
            }
        }

        m.words = best_tail;
        if (const auto *d = dynamic_cast<NodeDialogue*>(nodes.at(i).get())) {
            m.words += d->word_count;
        }
        m.next = best_next;
        m.visiting = false;
        m.computed = true;

        return m.words;
    };

    int best_total = 0;

    for (const auto &r : roots) {
        const auto candidate = best_from(r);
        best_total += candidate;
        std::optional<unsigned> curr = r;
        while (curr) {
            nodes.at(*curr)->path_flags |= Node::BEST_WC;
            curr = memos.at(*curr).next;
        }
    }

    return best_total;
}

Graph::Graph(const std::filesystem::path& path)
    : lexer(path) {
    generate_nodes();
}

auto Graph::get_nodes() -> std::vector<std::unique_ptr<Node>>& {
    return nodes;
}

auto Graph::get_roots() -> std::vector<unsigned>& {
    return roots;
}

void Graph::print_all_nodes() const {
    for (const auto &n : nodes) {
        std::println("{}", *n);
    }
}
