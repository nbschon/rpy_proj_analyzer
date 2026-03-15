//
// Created by Noah Schonhorn on 2/8/26.
//

#include "App.hpp"

#include <format>
#include <print>
#include <raylib.h>

#include "raylib-cpp.hpp"

#include "ArgVParser.hpp"
#include "Panel.hpp"
#include "Screen.hpp"

auto App::run() -> int {
    const int screen_width = ArgVParser::width ? *ArgVParser::width : 1280;
    const int screen_height = ArgVParser::height ? *ArgVParser::height : 720;
    const auto f_width = static_cast<float>(screen_width);
    const auto f_height = static_cast<float>(screen_height);

    SetConfigFlags(FLAG_VSYNC_HINT);

    raylib::Window window(screen_width, screen_height, std::format("rpy_proj_analyzer"));
    const raylib::RenderTexture bg_tex(screen_width, screen_height);

    BeginTextureMode(bg_tex);
    {
        const auto col = ArgVParser::dark_mode() ? raylib::Color(0x26, 0x2C, 0x36) : raylib::Color::RayWhite();
        ClearBackground(col);
        raylib::Rectangle(0, 0, f_width, f_height).Draw(col);
    }
    EndTextureMode();

    DisplayNode::default_color = ArgVParser::dark_mode() ? raylib::Color(0x14, 0x18, 0x1F) : raylib::Color(0xE2, 0xE2, 0xE2);
    DisplayNode::line_color = ArgVParser::dark_mode() ? raylib::Color::White() : raylib::Color::Black();
    TextHelper::default_color = ArgVParser::dark_mode() ? raylib::Color::White() : raylib::Color::Black();
    TextHelper::load_fonts();
    raylib::Image dir_img("./img/icons/directory.png");
    const auto new_color = ArgVParser::dark_mode() ? raylib::Color::White() : raylib::Color::Black();
    constexpr auto size = static_cast<int>(TextHelper::font_size);
    dir_img.ColorReplace(raylib::Color(0x00, 0xFF, 0xFF), new_color);
    dir_img.Resize(size, size);
    raylib::Image doc_img("./img/icons/document.png");
    doc_img.ColorReplace(raylib::Color(0x00, 0xFF, 0xFF), new_color);
    doc_img.Resize(size, size);
    FileTreePanel::dir_icon = std::make_unique<raylib::Texture2D>(dir_img);
    FileTreePanel::doc_icon = std::make_unique<raylib::Texture2D>(doc_img);
    dir_img.Unload();
    doc_img.Unload();

    bool run = true;

    State state;
    state.mode = ArgVParser::path ? State::Mode::View : State::Mode::LoadPath;
    if (state.mode == State::Mode::View) {
        bool bad_path = false;
        state.path_type = [&] -> State::PathType {
            if (std::filesystem::is_directory(*ArgVParser::path)) {
                const auto proj = build_dir_tree(*ArgVParser::path);
                const auto f = flat_files(proj);
                return State::PathType::Directory;
            }
            if (std::filesystem::is_regular_file(*ArgVParser::path) && ArgVParser::path->extension() == ".rpy") {
                return State::PathType::File;
            }
            bad_path = true;
            return State::PathType::File;
        }();
        if (bad_path) {
            state.mode = State::Mode::LoadPath;
        }
    }

    std::unique_ptr<Screen> screen;
    bool is_dir = state.path_type == State::PathType::Directory;
    switch (state.mode) {
        case State::Mode::LoadPath:
            screen = std::make_unique<LoadScreen>();
            break;
        case State::Mode::View:
            screen = std::make_unique<ViewScreen>(*ArgVParser::path, window, is_dir);
            break;
        case State::Mode::Exit:
            run = false;
            break;
    }

    while (run) {
        if ((mod_down() && IsKeyPressed(KEY_Q)) || window.ShouldClose()) {
            run = false;
        }

        const auto prev_mode = state.mode;

        screen->update(window, state);

        BeginDrawing();
        {
            window.ClearBackground(raylib::Color::RayWhite());
            DrawTexture(bg_tex.texture, 0, 0, raylib::Color::White());
            screen->draw(window);
        }
        EndDrawing();

        if (prev_mode != state.mode) {
            switch (state.mode) {
                case State::Mode::LoadPath:
                    screen = std::make_unique<LoadScreen>();
                    break;
                case State::Mode::View:
                    screen = std::make_unique<ViewScreen>(*state.path, window, is_dir);
                    break;
                case State::Mode::Exit:
                    run = false;
                    break;
            }
        }
    }

    TextHelper::unload_fonts();
    FileTreePanel::unload_textures();

    return 0;
}

auto App::run_no_gui() -> int {
    if (ArgVParser::path) {
        Lexer lexer(*ArgVParser::path);
        const auto tokens = lexer.tokenize();
        lexer.print_tokens();
        Graph graph(tokens);
        return 0;
    }

    std::println(std::cerr, "No file name given. Run with --help for options.");
    return -1;
}
