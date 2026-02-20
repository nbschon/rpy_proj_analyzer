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
