//
// Created by Noah Schonhorn on 2/8/26.
//

#include "Screen.hpp"

#include <ranges>
#include <raylib.h>

#include "App.hpp"

LoadScreen::LoadScreen() = default;

auto LoadScreen::get_dropped_path() -> std::optional<std::filesystem::path> {
    if (IsFileDropped()) {
        const auto dropped = LoadDroppedFiles();
        // for (int i = 0; i < dropped.count; ++i) {
        //     std::println("{}", dropped.paths[i]);
        // }
        auto path = std::filesystem::path(dropped.paths[0]);
        UnloadDroppedFiles(dropped);
        return path;
    }

    return std::nullopt;
}

void LoadScreen::update(const raylib::Window &win, State& state) {
    if (const auto path = get_dropped_path(); std::filesystem::is_directory(*path)) {
        got_path = false;
        state.path = path;
        state.path_type = State::PathType::Directory;
        state.mode = State::Mode::View;
    } else if (std::filesystem::is_regular_file(*path) && path->extension() == ".rpy") {
        got_path = false;
        state.path = path;
        state.path_type = State::PathType::File;
        state.mode = State::Mode::View;
    } else if (path) {
        dropped_path = std::format("{{i}}{}{{/i}} is not a valid Ren'Py script or project folder.", path->string());
    }
}

void LoadScreen::draw(const raylib::Window &win) {
    const auto f_width = static_cast<float>(win.GetWidth());
    const auto f_height = static_cast<float>(win.GetHeight());

    const raylib::Rectangle drop_rect{50, 50, f_width - 100, f_height - 100};
    constexpr std::string_view no_input = "No file or directory provided.";
    constexpr std::string_view drag_pls = "Please drag a script or project folder onto the window to get started.";
    const auto no_input_w = TextHelper::text_width(no_input);
    const auto drag_pls_w = TextHelper::text_width(drag_pls);
    const int no_input_x = (drop_rect.width / 2.0f) - (no_input_w / 2.0f) + 50.0f;
    const int drag_pls_x = (drop_rect.width / 2.0f) - (drag_pls_w / 2.0f) + 50.0f;

    drop_rect.Draw(DisplayNode::default_color);
    TextHelper::draw_text(no_input, drop_rect.width, {static_cast<float>(no_input_x), drop_rect.y + 50});
    TextHelper::draw_text(drag_pls, drop_rect.width, {static_cast<float>(drag_pls_x), drop_rect.y + 100});
    if (!dropped_path.empty()) {
        const auto dropped_w = TextHelper::text_width(dropped_path);
        const int dropped_x = (drop_rect.width / 2) - (dropped_w / 2) + 50;
        TextHelper::draw_text(dropped_path, drop_rect.width, {static_cast<float>(dropped_x), drop_rect.y + 200});
    }
}

ViewScreen::ViewScreen(const std::filesystem::path &path, const raylib::Window &win) :
    lexer(path), graph(lexer.tokenize()), graph_layout(graph) {
    std::tie(display_nodes, line_points) = graph_layout.make_displayables(graph);

    raylib::SetWindowTitle(std::format("rpy_proj_analyzer: {}", path.filename().string()));

    auto dn_min_x = std::numeric_limits<float>::max();
    auto dn_max_x = -std::numeric_limits<float>::max();
    auto dn_min_y = std::numeric_limits<float>::max();
    auto dn_max_y = -std::numeric_limits<float>::max();

    const auto &first_node = display_nodes.front();

    for (const auto &dn : display_nodes) {
        if (dn.padding_box.x < dn_min_x) {
            dn_min_x = dn.padding_box.x;
        } else if (dn.padding_box.x > dn_max_x) {
            dn_max_x = dn.padding_box.x;
        }

        if (dn.padding_box.y < dn_min_y) {
            dn_min_y = dn.padding_box.y;
        } else if (dn.padding_box.y > dn_max_y) {
            dn_max_y = dn.padding_box.y;
        }
    }

    const auto init_x = first_node.padding_box.x + (first_node.padding_box.width / 2) - (static_cast<float>(win.GetWidth()) / 2);
    camera.target = {init_x, 0.0f};
    camera.offset = {0.0f, 0.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    min_x = 0.0f;
    max_x = (graph_layout.get_max_width() * DisplayNode::get_width()) - 40.0f;
    // const float min_y = -40.0f;
    min_y = 0.0f;
    // const float max_y = display_nodes.back().box.y + display_nodes.back().box.height - 40.0f;
    max_y = dn_max_y - 40.0f;
}

void ViewScreen::update(const raylib::Window &win, State& state) {
    if (IsKeyPressed(KEY_UP)) {
        scroll_speed += 5.0f;
    } else if (IsKeyPressed(KEY_DOWN)) {
        scroll_speed -= 5.0f;
        if (scroll_speed < 0) {
            scroll_speed = 5.0f;
        }
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_D)) {
        debug = !debug;
    }

    // if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Q) || win.ShouldClose()) {
    //     run = false;
    // }

    if (!IsKeyDown(KEY_LEFT_CONTROL)) {
        if (IsKeyDown(KEY_S)) {
            camera.target.y += 5;
        } else if (IsKeyDown(KEY_W)) {
            camera.target.y -= 5;
        }

        if (IsKeyDown(KEY_D)) {
            camera.target.x += 5;
        } else if (IsKeyDown(KEY_A)) {
            camera.target.x -= 5;
        }
    }

    if (IsKeyDown(KEY_LEFT_ALT)) {
        scroll_speed += GetMouseWheelMove();
    } else if (IsKeyDown(KEY_LEFT_SHIFT)) {
        camera.target.x -= (GetMouseWheelMoveV().y * scroll_speed);
    } else if (IsKeyUp(KEY_LEFT_CONTROL)) {
        camera.target.x -= GetMouseWheelMoveV().x * scroll_speed;
        camera.target.y -= GetMouseWheelMoveV().y * scroll_speed;
    }

    const raylib::Vector2 before_zoom = GetScreenToWorld2D(GetMousePosition(), camera);

    if (IsKeyDown(KEY_LEFT_CONTROL)) {
        camera.zoom += GetMouseWheelMove() / 10;
    } else if (IsKeyPressed(KEY_MINUS) || (IsKeyPressed(KEY_KP_SUBTRACT))) {
        camera.zoom -= 0.1f;
    } else if (IsKeyPressed(KEY_EQUAL) || (IsKeyPressed(KEY_KP_ADD))) {
        camera.zoom += 0.1f;
    } else if (IsKeyPressed(KEY_R)) {
        camera.zoom = 1.0f;
    }

    camera.zoom = std::clamp(camera.zoom, min_zoom, max_zoom);

    const raylib::Vector2 curr_mouse_pos = GetScreenToWorld2D(GetMousePosition(), camera);

    camera.target.x += (before_zoom.x - curr_mouse_pos.x);
    camera.target.y += (before_zoom.y - curr_mouse_pos.y);

    camera.target.x = std::clamp(camera.target.x, min_x, max_x);
    camera.target.y = std::clamp(camera.target.y, min_y, max_y);

    auto [cam_min_x, cam_min_y] = GetScreenToWorld2D({0.0f, 0.0f}, camera);
    auto [cam_max_x, cam_max_y] = GetScreenToWorld2D({static_cast<float>(win.GetWidth()), static_cast<float>(win.GetHeight())}, camera);

    on_screen = display_nodes
        | std::views::filter([&](const DisplayNode &dn) -> bool {
            const auto top_y = cam_min_y - dn.padding_box.height - 50;
            const auto bottom_y = cam_max_y + 50;
            const auto left_x = cam_min_x - dn.padding_box.width - 50;
            const auto right_x = cam_max_x + 50;
            const auto visible_x = dn.main_box.x == std::clamp(dn.main_box.x, left_x, right_x);
            const auto visible_y = dn.main_box.y == std::clamp(dn.main_box.y, top_y, bottom_y);
            return visible_x && visible_y;
        })
        | std::views::transform([](auto &dn) -> DisplayNode* { return &dn; })
        | std::ranges::to<std::vector<DisplayNode*>>();

    for (const auto &dn : on_screen) {
        dn->is_mouse_hovering(camera);
    }
}

void ViewScreen::draw(const raylib::Window &win) {
    std::string hover_text;

    camera.BeginMode();
    {
        for (const auto &points : line_points) {
            // DrawSplineBasis(points.data(), 5, 2.0, raylib::Color::Red());
            DrawLineEx(points.at(1), points.at(3), 2.0, raylib::Color::Red());
        }

        for (const auto& dn : on_screen) {
            if (const auto &text = dn->draw()) {
                hover_text = *text;
            }
            if (debug) {
                dn->margin_box.DrawLines(raylib::Color::Orange());
                dn->main_box.DrawLines(raylib::Color::SkyBlue());
                dn->padding_box.DrawLines(raylib::Color::Lime());
            }
        }
    }

    camera.EndMode();

    if (!hover_text.empty()) {
        constexpr auto box_height = 40.0f; // TODO: make this not magic
        const auto text_width = TextHelper::text_width(hover_text);
        auto x = static_cast<float>(GetMouseX());
        auto y = static_cast<float>(GetMouseY()) - box_height;

        const raylib::Rectangle hover_box(x, y, text_width + 2, box_height);
        hover_box.Draw(DisplayNode::default_color);
        hover_box.DrawLines(DisplayNode::line_color);
        TextHelper::draw_text(hover_text, hover_box.width, {x, y});
    }

    if (debug) {
        raylib::Rectangle(0, 0, static_cast<float>(win.GetWidth()), 50).Draw(raylib::Color{0xF5F5F5AF});
        DrawFPS(GetScreenWidth() - 80, 5);
        raylib::DrawText(std::format("scroll speed: {:.1f}", scroll_speed).c_str(),
            10, 5, 20, raylib::Color::Blue());
        raylib::DrawText(std::format("cam x: {}", camera.target.x).c_str(),
            200, 5, 20, raylib::Color::Blue());
        raylib::DrawText(std::format("cam y: {}", camera.target.y).c_str(),
            400, 5, 20, raylib::Color::Blue());
        raylib::DrawText(std::format("on screen nodes: {}", on_screen.size()).c_str(),
            10, 25, 20, raylib::Color::Blue());
        raylib::DrawText(std::format("camera zoom: {:.2f}", camera.zoom).c_str(),
            300, 25, 20, raylib::Color::Blue());
    }
}
