//
// Created by Noah Schonhorn on 2/8/26.
//

#ifndef RPY_PROJ_ANALYZER_SCREEN_HPP
#define RPY_PROJ_ANALYZER_SCREEN_HPP

#include <chrono>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

#include "raylib-cpp.hpp"

#include "DisplayNode.hpp"
#include "Graph.hpp"
#include "GraphLayout.hpp"
#include "Lexer.hpp"
#include "Panel.hpp"

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

    struct RenpyFile {
        Lexer lexer;
        Graph graph;
        GraphLayout layout;

        explicit RenpyFile(const std::filesystem::path &path)
            : lexer(path), graph(lexer.tokenize()), layout(graph) {
        }
    };

    std::unordered_map<std::string, std::unique_ptr<RenpyFile>> paths;
    std::string active_file;

    std::vector<DisplayNode> display_nodes;
    std::vector<std::array<raylib::Vector2, 5>> line_points;

    raylib::Camera2D camera;
    float min_x;
    float max_x;
    float min_y;
    float max_y;

    std::optional<std::chrono::steady_clock::time_point> last_clicked;
    DisplayNode *clicked_ptr = nullptr;

    bool debug = true;

    std::vector<DisplayNode*> on_screen;

    std::unique_ptr<FileTreePanel> file_tree = nullptr;

public:
    explicit ViewScreen(const std::filesystem::path &path, const raylib::Window &win, bool is_dir);
    void update(const raylib::Window &win, State& state) override;
    void draw(const raylib::Window &win) override;
};

#endif //RPY_PROJ_ANALYZER_SCREEN_HPP
