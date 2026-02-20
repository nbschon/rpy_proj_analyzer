//
// Created by Noah Schonhorn on 1/9/26.
//

#ifndef RPY_PROJ_ANALYZER_GRAPHLAYOUT_HPP
#define RPY_PROJ_ANALYZER_GRAPHLAYOUT_HPP

#include "Graph.hpp"

#include "DisplayNode.hpp"

#include <array>
#include <memory>
#include <string>
#include <unordered_map>

#include "raylib-cpp.hpp"

struct LayoutDims {
    int left_x{};
    unsigned top_y{};
    unsigned w_units = 1;
    unsigned h_units = 1;
    // raylib::Color color = raylib::Color::White();
};

enum class GroupType {
    If,
    Menu,
    Label,
};


class LayoutBase {
protected:
    unsigned idx;

public:
    unsigned width = 1;
    unsigned height = 1;
    LayoutDims layout{};
    explicit LayoutBase(unsigned idx);
    virtual ~LayoutBase() = default;

    virtual auto to_string() -> std::string = 0;
    virtual auto has_children() -> bool;
    virtual auto update_width() -> unsigned;
    virtual auto update_height() -> unsigned;
    virtual void flatten(std::vector<LayoutBase*>& flat_disps);
    virtual void collect_edges(std::unordered_map<Node*, Node*>& edges);
    [[nodiscard]] auto get_idx() const -> unsigned;
};

class LayoutItem : public LayoutBase {
    std::optional<unsigned> pre_idx;

public:
    explicit LayoutItem(unsigned idx);
    auto to_string() -> std::string override;
    auto get_pre_item() -> std::optional<unsigned>;
};

class LayoutColumn : public LayoutBase {

public:
    unsigned center_offset = 0;
    std::vector<std::unique_ptr<LayoutBase>> displays;

    LayoutColumn(const std::vector<std::unique_ptr<Node>>& nodes, unsigned parent_idx, unsigned first_node, NodeParent* parent_ptr);
    auto to_string() -> std::string override;
    auto has_children() -> bool override;
    auto update_width() -> unsigned override;
    auto update_height() -> unsigned override;
    void flatten(std::vector<LayoutBase*>& flat_disps) override;
    void collect_edges(std::unordered_map<Node*, Node*>& edges) override;
};

class LayoutGroup : public LayoutBase {
    GroupType type{};
    unsigned anchor_offset = 0;

public:
    std::vector<LayoutColumn> columns;
    std::unordered_map<Node*, Node*> children_to_parents;

    explicit LayoutGroup(unsigned idx, GroupType type, std::vector<LayoutColumn> columns, std::unordered_map<Node*, Node*> c_to_p);
    LayoutGroup(unsigned idx, GroupType type, std::vector<LayoutColumn> columns);
    auto to_string() -> std::string override;
    auto has_children() -> bool override;
    auto update_width() -> unsigned override;
    auto update_height() -> unsigned override;
    void flatten(std::vector<LayoutBase*>& flat_disps) override;
    void collect_edges(std::unordered_map<Node*, Node*>& edges) override;
    auto anchor_x() const -> int;
};

struct LayoutData {
    std::vector<DisplayNode> disps;
    std::vector<std::array<raylib::Vector2, 5>> line_points;
};

class GraphLayout {
    std::vector<std::unique_ptr<LayoutBase>> groups;
    std::vector<LayoutBase*> flat_disps;
    void assign_dimensions() const;
    void assign_layouts() const;
    void flatten();
    auto collect_edges() const -> std::unordered_map<Node*, Node*>;

public:
    explicit GraphLayout(Graph &graph);
    auto get_groups() -> std::vector<std::unique_ptr<LayoutBase>>&;
    auto get_max_width() -> unsigned;
    auto make_displayables(Graph &graph) -> std::pair<std::vector<DisplayNode>, std::vector<std::array<raylib::Vector2, 5>>>;
};

namespace Layout {
    auto make_ifs(const std::vector<std::unique_ptr<Node>>& nodes, unsigned parent_idx, unsigned idx) -> std::unique_ptr<LayoutGroup>;
    auto make_menu(const std::vector<std::unique_ptr<Node>>& nodes, unsigned parent_idx, unsigned idx) -> std::unique_ptr<LayoutGroup>;
    auto make_label(const std::vector<std::unique_ptr<Node>>& nodes, unsigned parent_idx, unsigned idx) -> std::unique_ptr<LayoutGroup>;

    void layout_node(LayoutBase &disp, int left_x, unsigned row);
    void layout_column(const LayoutColumn &col, int left_x, unsigned row);
    void layout_group(LayoutGroup &group, int left_x, unsigned row);
}

#endif //RPY_PROJ_ANALYZER_GRAPHLAYOUT_HPP
