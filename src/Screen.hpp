//
// Created by Noah Schonhorn on 2/8/26.
//

#ifndef RPY_PROJ_ANALYZER_SCREEN_HPP
#define RPY_PROJ_ANALYZER_SCREEN_HPP

#include <filesystem>
#include <optional>
#include <string>

#include "raylib-cpp.hpp"

#include "DisplayNode.hpp"
#include "Graph.hpp"
#include "GraphLayout.hpp"
#include "Lexer.hpp"

struct State;

enum class PathType : std::uint8_t {
    File,
    Directory,
};

class Screen {
public:
    virtual ~Screen() = default;

    virtual void update(const raylib::Window &win, State& state) = 0;
    virtual void draw(const raylib::Window &win) = 0;
};

class LoadScreen final : public Screen {
    std::string dropped_path;
    bool got_path = false;

    static auto get_dropped_path() -> std::optional<std::filesystem::path>;

public:
    explicit LoadScreen();
    void update(const raylib::Window &win, State& state) override;
    void draw(const raylib::Window &win) override;
};

class ViewScreen final : public Screen {
    #ifndef __APPLE__
        float scroll_speed = 45.0f;
    #else
        float scroll_speed = 5.0f;
    #endif //__APPLE__

    static constexpr float min_zoom = 0.2f;
    static constexpr float max_zoom = 2.0f;

    Lexer lexer;
    Graph graph;
    GraphLayout graph_layout;
    std::vector<DisplayNode> display_nodes;
    std::vector<std::array<raylib::Vector2, 5>> line_points;

    raylib::Camera2D camera;
    float min_x;
    float max_x;
    float min_y;
    float max_y;

    bool debug = true;

    std::vector<DisplayNode*> on_screen;

public:
    explicit ViewScreen(const std::filesystem::path &path, const raylib::Window &win);
    void update(const raylib::Window &win, State& state) override;
    void draw(const raylib::Window &win) override;
};

#endif //RPY_PROJ_ANALYZER_SCREEN_HPP
