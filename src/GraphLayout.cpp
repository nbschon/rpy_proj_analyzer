//
// Created by Noah Schonhorn on 1/9/26.
//

#include "GraphLayout.hpp"

#include <algorithm>
#include <cassert>
#include <ranges>

auto Layout::make_ifs(const std::vector<std::unique_ptr<Node>>& nodes, unsigned idx,
                      unsigned prev_idx) -> std::unique_ptr<LayoutGroup> {
    std::vector<LayoutColumn> branches;
    const unsigned if_idx = idx;
    std::vector<unsigned> elif_idxs;

    std::vector<unsigned> col_header_idxs;

    bool do_grouping = true;
    if (auto if_node = dynamic_cast<NodeIf *>(nodes.at(idx).get())) {
        col_header_idxs.push_back(idx);
        if (if_node->first_child && if_node->after_block) {
            LayoutColumn col(nodes, prev_idx, idx, if_node);
            branches.push_back(std::move(col));
        }
    }

    auto next = nodes.at(idx)->next;
    while (do_grouping && next) {
        if (dynamic_cast<NodeIf *>(nodes.at(*next).get())) {
            do_grouping = false;
            // auto if_node = dynamic_cast<NodeIf*>(nodes.at(*next).get());
            // if (if_node->first_child && if_node->after_block) {
            //     branches.emplace_back(nodes, *if_node->first_child, *if_node->after_block);
            // }
        } else if (dynamic_cast<NodeElif *>(nodes.at(*next).get())) {
            col_header_idxs.push_back(*next);
            if (auto elif_node = dynamic_cast<NodeElif *>(nodes.at(*next).get());
                elif_node->first_child && elif_node->after_block) {
                // branches.emplace_back(nodes, *elif_node->first_child, *elif_node->after_block);
                LayoutColumn col(nodes, prev_idx, *next, elif_node);
                branches.push_back(std::move(col));
            }
        } else if (dynamic_cast<NodeElse *>(nodes.at(*next).get())) {
            col_header_idxs.push_back(*next);
            if (auto else_node = dynamic_cast<NodeElse *>(nodes.at(*next).get());
                else_node->first_child && else_node->after_block) {
                // branches.emplace_back(nodes, *else_node->first_child, *else_node->after_block);
                LayoutColumn col(nodes, prev_idx, *next, else_node);
                branches.push_back(std::move(col));
            }
            do_grouping = false;
        }
        next = nodes.at(*next).get()->next;
    }
    auto before_idx = *nodes.at(idx)->prev;

    auto c_to_p =
        col_header_idxs
        | std::views::transform([&](const unsigned& h_idx) {
            return std::pair{nodes.at(h_idx).get(), nodes.at(before_idx).get()};
        })
        | std::ranges::to<std::unordered_map<Node *, Node *>>();

    // std::println("--------------------");
    // for (const auto [child_ptr, parent_ptr] : c_to_p) {
    //     std::println("{} branch points to {}", *child_ptr, *parent_ptr);
    // }
    // std::println("--------------------");

    return std::make_unique<LayoutGroup>(if_idx, GroupType::If, std::move(branches), std::move(c_to_p));
}

auto Layout::make_menu(const std::vector<std::unique_ptr<Node>>& nodes, unsigned idx,
                       unsigned parent_idx) -> std::unique_ptr<LayoutGroup> {
    std::vector<LayoutColumn> choices;
    const auto menu_idx = idx;

    std::vector<unsigned> col_header_idxs;

    const auto* menu = dynamic_cast<NodeMenu *>(nodes.at(idx).get());

    bool do_grouping = true;
    auto next = menu->first_child;
    while (do_grouping && next) {
        if (const auto choice = dynamic_cast<NodeChoice *>(nodes.at(*next).get());
            choice->first_child && choice->after_block) {
            col_header_idxs.push_back(*next);
            LayoutColumn col(nodes, parent_idx, *next, choice);
            choices.emplace_back(std::move(col));
        } else if (dynamic_cast<NodeMenu *>(nodes.at(*next).get())) {
            do_grouping = false;
        }
        next = nodes.at(*next).get()->next;
    }

    auto c_to_p =
        col_header_idxs
        | std::views::transform([&](const unsigned& h_idx) {
            return std::pair{nodes.at(h_idx).get(), nodes.at(menu_idx).get()};
        })
        | std::ranges::to<std::unordered_map<Node *, Node *>>();
    // std::println("--------------------");
    // for (const auto [child_ptr, parent_ptr] : c_to_p) {
    //     std::println("{} points to menu {}", *child_ptr, *parent_ptr);
    // }
    // std::println("--------------------");

    return std::make_unique<LayoutGroup>(menu_idx, GroupType::Menu, std::move(choices), std::move(c_to_p));
}

auto Layout::make_label(const std::vector<std::unique_ptr<Node>>& nodes,
                        unsigned idx, unsigned header_idx) -> std::unique_ptr<LayoutGroup> {
    const auto label_idx = idx;

    const auto label = dynamic_cast<NodeLabel *>(nodes.at(idx).get());

    std::vector<LayoutColumn> column;
    LayoutColumn col(nodes, header_idx, idx, label);
    column.push_back(std::move(col));

    return std::make_unique<LayoutGroup>(label_idx, GroupType::Label, std::move(column));
}

void Layout::layout_node(LayoutBase& disp, int left_x, unsigned row) {
    if (auto* group = dynamic_cast<LayoutGroup *>(&disp)) {
        layout_group(*group, left_x, row);
    } else {
        disp.layout = {left_x, row, disp.width, disp.height};
    }
}

void Layout::layout_column(const LayoutColumn& col, int left_x, unsigned row) {
    unsigned curr_row = row;
    for (auto& display : col.displays) {
        const int parent_center = left_x + static_cast<int>(col.center_offset);

        if (const auto* g = dynamic_cast<LayoutGroup *>(display.get())) {
            const int child_left = parent_center - g->anchor_x();
            layout_node(*display, child_left, curr_row);
        } else {
            const int item_left = parent_center - static_cast<int>(display->width) / 2;
            display->layout = {item_left, curr_row, display->width, display->height};
        }

        curr_row += display->height;
    }
}

void Layout::layout_group(LayoutGroup& group, int left_x, unsigned row) {
    group.layout = {left_x, row, group.width, group.height};

    int x_pos = left_x;

    for (auto& col : group.columns) {
        layout_column(col, x_pos, row);

        x_pos += col.width;
    }
}

LayoutBase::LayoutBase(const unsigned idx) : idx(idx) {
}

auto LayoutBase::has_children() -> bool {
    return false;
}

auto LayoutBase::update_width() -> unsigned {
    return 1;
}

auto LayoutBase::update_height() -> unsigned {
    return 1;
}

void LayoutBase::flatten(std::vector<LayoutBase *>& flat_disps) {
    flat_disps.push_back(this);
}

void LayoutBase::collect_edges(std::unordered_map<Node *, Node *>& edges) {
}

auto LayoutBase::get_idx() const -> unsigned {
    return idx;
}

LayoutItem::LayoutItem(const unsigned idx) : LayoutBase(idx) {
}

auto LayoutItem::to_string() -> std::string {
    return std::format("LayoutItem w node idx {}", idx);
}

auto LayoutItem::get_pre_item() -> std::optional<unsigned> {
    return pre_idx;
}

LayoutColumn::LayoutColumn(const std::vector<std::unique_ptr<Node>>& nodes, const unsigned parent_idx,
                           const unsigned first_node, NodeParent* parent_ptr)
    : LayoutBase(parent_idx) {
    auto i = *parent_ptr->first_child;
    // header_idx = parent_idx;

    this->displays.emplace_back(std::make_unique<LayoutItem>(first_node));
    std::optional<unsigned> prev_idx = first_node;

    while (i < *parent_ptr->after_block) {
        const auto& n = nodes.at(i);
        std::unique_ptr<LayoutGroup> group_ptr = nullptr;

        if (n->has_children()) {
            if (dynamic_cast<NodeIf *>(n.get())) {
                this->displays.emplace_back(Layout::make_ifs(nodes, i, *prev_idx));
            } else if (dynamic_cast<NodeMenu *>(n.get())) {
                // ==============================================================
                // kind of a hack, but necessary due to the way menus are grouped
                this->displays.emplace_back(std::make_unique<LayoutItem>(i));
                // ==============================================================

                // then just do it like normal
                this->displays.emplace_back(Layout::make_menu(nodes, i, *prev_idx));
            } else if (dynamic_cast<NodeLabel *>(n.get())) {
                this->displays.emplace_back(Layout::make_label(nodes, i, *prev_idx));
            }
        } else {
            this->displays.emplace_back(std::make_unique<LayoutItem>(i));
        }


        prev_idx = i;
        if (!n->next) {
            break;
        }
        i = *n->next;
    }
}

auto LayoutColumn::to_string() -> std::string {
    return std::format("LayoutColumn w header idx {} and {} items", idx, displays.size());
}

auto LayoutColumn::has_children() -> bool {
    return true;
}

// auto LayoutColumn::update_width() -> unsigned {
//     unsigned acc_width = 1;
//
//     for (const auto& node : displays) {
//         acc_width = std::max(acc_width, node->update_width());
//     }
//
//     // flat_disps.push_back(this);
//     this->width = acc_width;
//     return this->width;
// }
auto LayoutColumn::update_width() -> unsigned {
    unsigned left_extent  = 0;
    unsigned right_extent = 0;

    for (const auto& node : displays) {
        const unsigned w = node->update_width();

        if (auto* g = dynamic_cast<LayoutGroup*>(node.get())) {
            const unsigned ax = static_cast<unsigned>(std::max(0, g->anchor_x()));
            left_extent  = std::max(left_extent, ax);
            right_extent = std::max(right_extent, w - std::min(ax, w));
        } else {
            const unsigned half = w / 2;
            left_extent  = std::max(left_extent, half);
            right_extent = std::max(right_extent, w - half); // handles odd widths
        }
    }

    center_offset = left_extent;
    this->width = std::max(1u, left_extent + right_extent);
    return this->width;
}


auto LayoutColumn::update_height() -> unsigned {
    unsigned acc_height = 0;
    for (const auto& node : displays) {
        acc_height += node->update_height();
    }
    this->height = std::max(1u, acc_height);
    return this->height;
}

void LayoutColumn::flatten(std::vector<LayoutBase *>& flat_disps) {
    for (const auto& node : displays) {
        node->flatten(flat_disps);
    }
}

void LayoutColumn::collect_edges(std::unordered_map<Node *, Node *>& edges) {
    for (const auto& node : displays) {
        node->collect_edges(edges);
    }
}

LayoutGroup::LayoutGroup(const unsigned idx, const GroupType type, std::vector<LayoutColumn> columns, std::unordered_map<Node*, Node*> c_to_p)
    : LayoutBase(idx), type(type), columns(std::move(columns)), children_to_parents(std::move(c_to_p)) {
}

LayoutGroup::LayoutGroup(unsigned idx, GroupType type, std::vector<LayoutColumn> columns)
    : LayoutBase(idx), type(type), columns(std::move(columns)), children_to_parents(std::unordered_map<Node*, Node*>()) {

}

auto LayoutGroup::to_string() -> std::string {
    return std::format("LayoutGroup w {} cols", columns.size());
}

auto LayoutGroup::has_children() -> bool {
    return true;
}

auto LayoutGroup::update_width() -> unsigned {
    unsigned acc_width = 0;
    anchor_offset = 0;

    for (auto& col : columns) {
        acc_width += col.update_width();
    }

    const unsigned n = columns.size();
    const unsigned mid = n / 2;

    for (unsigned i = 0; i < columns.size(); ++i) {
        if (i < mid) {
            anchor_offset += columns.at(i).width;
            // acc_width += columns.at(i).width;
        }
    }

    // flat_disps.push_back(this);
    width = std::max(1u, acc_width);
    return width;
}

auto LayoutGroup::update_height() -> unsigned {
    unsigned max_col = 0;
    for (auto& col : columns) {
        max_col = std::max(max_col, col.update_height());
    }
    height = max_col;
    return height;
}

void LayoutGroup::flatten(std::vector<LayoutBase *>& flat_disps) {
    for (auto& col : columns) {
        col.flatten(flat_disps);
    }
}

void LayoutGroup::collect_edges(std::unordered_map<Node *, Node *>& edges) {
    edges.insert(children_to_parents.begin(), children_to_parents.end());
    for (auto& col : columns) {
        col.collect_edges(edges);
    }
}

auto LayoutGroup::anchor_x() const -> int {
    if (columns.empty()) {
        return 1;
    }

    const unsigned left_mid = (columns.size() - 1) / 2;
    unsigned left_sum = 0;
    for (unsigned i = 0; i < left_mid; ++i) {
        left_sum += columns.at(i).width;
    }

    if (columns.size() % 2 == 0) {
        return static_cast<int>(left_sum + columns.at(left_mid).width);
    }
    return static_cast<int>(left_sum + columns.at(left_mid).width / 2);
}

void GraphLayout::assign_dimensions() const {
    for (const auto& group : groups) {
        group->update_width();
        group->update_height();
    }
}

void GraphLayout::assign_layouts() const {
    unsigned y_pos = 0;
    for (auto& group : groups) {
        Layout::layout_node(*group, 0, y_pos);
        y_pos += group->height + 1;
    }
}

void GraphLayout::flatten() {
    for (const auto& group : groups) {
        group->flatten(flat_disps);
    }
}

auto GraphLayout::collect_edges() const -> std::unordered_map<Node*, Node*> {
    std::unordered_map<Node*, Node*> edges;
    for (const auto &group : groups) {
        group->collect_edges(edges);
    }

    return edges;
}

GraphLayout::GraphLayout(Graph& graph) {
    for (int i = 0; i < graph.get_nodes().size(); ++i) {
        const auto& n = graph.get_nodes().at(i);
        // TODO: make this *not* just assume labels are the roots
        if (dynamic_cast<NodeLabel *>(n.get())) {
            groups.emplace_back(Layout::make_label(graph.get_nodes(), i, i));
        }
    }

    assign_dimensions();
    assign_layouts();
    flatten();

    assert(flat_disps.size() == graph.get_nodes().size());
}

auto GraphLayout::get_groups() -> std::vector<std::unique_ptr<LayoutBase>>& {
    return groups;
}

auto GraphLayout::get_max_width() -> unsigned {
    return std::ranges::max(
        groups | std::views::transform(&LayoutGroup::width)
    );
}

auto GraphLayout::make_displayables(Graph& graph) -> std::pair<std::vector<DisplayNode>, std::vector<std::array<raylib::Vector2, 5>>> {
    std::vector<unsigned> idxs;
    auto displayables =
            flat_disps
            | std::views::transform([&](const auto& ptr) {
                auto idx = ptr->get_idx();
                idxs.push_back(idx);
                const auto& node = graph.get_nodes().at(idx);
                raylib::Rectangle disp_box{
                    ptr->layout.left_x * DisplayNode::get_width(),
                    ptr->layout.top_y * DisplayNode::get_height(),
                    DisplayNode::get_width(),
                    DisplayNode::get_height()
                };
                // disp_box.x *= 1.05;
                // disp_box.y *= 1.05;
                auto disp = node->make_display_node(disp_box);
                return disp;
            })
            | std::ranges::to<std::vector<DisplayNode>>();

    std::unordered_map<Node*, DisplayNode*> nodes_to_disps;
    for (int i = 0; i < idxs.size(); ++i) {
        nodes_to_disps[graph.get_nodes().at(idxs.at(i)).get()] = &displayables.at(i);
    }

    for (int i = 0; i < displayables.size(); ++i) {
        for (int j = 0; j < displayables.size(); ++j) {
            if (i != j) {
                if (displayables.at(i).padding_box.CheckCollision(displayables.at(j).padding_box)) {
                    std::println(std::cerr, "WARNING: two boxes colliding");
                }
            }
        }
    }

    std::vector<std::array<raylib::Vector2, 5>> line_points;

    auto edges = collect_edges();
    for (auto &[child, parent] : edges) {
        auto child_disp = nodes_to_disps[child];
        auto parent_disp = nodes_to_disps[parent];
        auto child_x = child_disp->main_box.x + (child_disp->main_box.width / 2);
        auto child_y = child_disp->main_box.y;
        auto parent_x = parent_disp->main_box.x + (parent_disp->main_box.width / 2);
        auto parent_y = parent_disp->main_box.y + parent_disp->main_box.height;
        auto step_y = (child_y - parent_y) / 5;
        auto parent_down = raylib::Vector2{parent_x, parent_y + step_y};
        auto child_up = raylib::Vector2{child_x, child_y - step_y};
        auto middle = raylib::Vector2{(child_x + parent_x) / 2, (child_y + parent_y) / 2};
        raylib::Vector2 child_pt(child_x, child_y);
        raylib::Vector2 parent_pt(parent_x, parent_y);
        line_points.push_back({child_pt, child_up, middle, parent_down, parent_pt});
        // std::println("{} -> {}", child_disp->to_string(), parent_disp->to_string());
    }

    return {std::move(displayables), std::move(line_points)};
}
