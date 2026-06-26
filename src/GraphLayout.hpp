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
    float left_x{};
    float top_y{};
    float w_units = 1;
    float h_units = 1;
};

enum class GroupType : std::uint8_t {
    If,
    Menu,
    Label,
};

class LayoutBase {
protected:
    unsigned idx;

public:
    float width = 1;
    float height = 1;
    LayoutDims layout{};
    explicit LayoutBase(unsigned idx);
    virtual ~LayoutBase() = default;

    virtual auto to_string() -> std::string = 0;
    virtual auto has_children() -> bool;
    virtual auto update_width() -> float;
    virtual auto update_height() -> float;
    virtual auto update_highest_wc(const std::vector<std::unique_ptr<Node>> &nodes) -> int;
    virtual void mark_highest_wc(const std::vector<std::unique_ptr<Node>>& nodes);
    virtual void flatten(std::vector<LayoutBase*>& flat_disps);
    virtual void collect_edges(std::unordered_map<Node*, Node*>& edges);
    [[nodiscard]] auto get_idx() const -> unsigned;
};

class LayoutItem : public LayoutBase {
    std::optional<unsigned> pre_idx;

public:
    explicit LayoutItem(unsigned idx);
    auto to_string() -> std::string override;
    [[nodiscard]] auto get_pre_item() const -> std::optional<unsigned>;
};

class LayoutColumn : public LayoutBase {

public:
    float center_offset = 0;
    std::vector<std::unique_ptr<LayoutBase>> displays;

    LayoutColumn(const std::vector<std::unique_ptr<Node>>& nodes, unsigned parent_idx, unsigned first_node, const NodeParent* parent_ptr);
    auto to_string() -> std::string override;
    auto has_children() -> bool override;
    auto update_width() -> float override;
    auto update_height() -> float override;
    auto update_highest_wc(const std::vector<std::unique_ptr<Node>>& nodes) -> int override;
    void mark_highest_wc(const std::vector<std::unique_ptr<Node>>& nodes) override;
    void flatten(std::vector<LayoutBase*>& flat_disps) override;
    void collect_edges(std::unordered_map<Node*, Node*>& edges) override;
};

class LayoutGroup : public LayoutBase {
    GroupType type{};
    float anchor_offset = 0;

public:
    std::vector<LayoutColumn> columns;
    std::unordered_map<Node*, Node*> children_to_parents;

    explicit LayoutGroup(unsigned idx, GroupType type, std::vector<LayoutColumn> columns, std::unordered_map<Node*, Node*> c_to_p);
    LayoutGroup(unsigned idx, GroupType type, std::vector<LayoutColumn> columns);
    auto to_string() -> std::string override;
    auto has_children() -> bool override;
    auto update_width() -> float override;
    auto update_height() -> float override;
    auto update_highest_wc(const std::vector<std::unique_ptr<Node>>& nodes) -> int override;
    void mark_highest_wc(const std::vector<std::unique_ptr<Node>>& nodes) override;
    void flatten(std::vector<LayoutBase*>& flat_disps) override;
    void collect_edges(std::unordered_map<Node*, Node*>& edges) override;
    [[nodiscard]] auto anchor_x() const -> float;
};

constexpr int N_POINTS = 5;

struct LayoutData {
    std::vector<DisplayNode> disps;
    std::vector<std::array<raylib::Vector2, N_POINTS>> line_points;
    std::vector<raylib::Rectangle> highlights;
};

class GraphLayout {
    std::vector<std::unique_ptr<LayoutBase>> top_levels;
    std::vector<LayoutBase*> flat_disps;
    void assign_dimensions() const;
    void assign_layouts();
    void assign_wc(const std::vector<std::unique_ptr<Node>>&) const;
    void flatten();
    [[nodiscard]] auto collect_edges() const -> std::unordered_map<Node*, Node*>;

public:
    explicit GraphLayout(Graph &graph);
    auto get_groups() -> std::vector<std::unique_ptr<LayoutBase>>&;
    auto get_max_width() -> float;
    auto make_displayables(Graph &graph) -> LayoutData;
};

namespace Layout {
    auto make_ifs(const std::vector<std::unique_ptr<Node>>& nodes, unsigned prev_idx, unsigned idx) -> std::unique_ptr<LayoutGroup>;
    auto make_menu(const std::vector<std::unique_ptr<Node>>& nodes, unsigned parent_idx, unsigned idx) -> std::unique_ptr<LayoutGroup>;
    auto make_label(const std::vector<std::unique_ptr<Node>>& nodes, unsigned header_idx, unsigned idx) -> std::unique_ptr<LayoutGroup>;

    void layout_node(LayoutBase &disp, float left_x, float row);
    void layout_column(const LayoutColumn &col, float left_x, float row);
    void layout_group(LayoutGroup &group, float left_x, float row);
}

#endif //RPY_PROJ_ANALYZER_GRAPHLAYOUT_HPP
