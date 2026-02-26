//
// Created by Noah Schonhorn on 2/8/26.
//

#ifndef RPY_PROJ_ANALYZER_APP_HPP
#define RPY_PROJ_ANALYZER_APP_HPP

#include <filesystem>
#include <optional>

#include "raylib-cpp.hpp"

struct State {
    enum class Mode : std::uint8_t {
        LoadPath,
        View,
        Exit,
    };

    enum class PathType : std::uint8_t {
        File,
        Directory,
    };
    Mode mode{};
    PathType path_type{};
    std::optional<std::filesystem::path> path;
};

class App {
#ifndef __APPLE__
    static constexpr auto mod_key_l = KEY_LEFT_CONTROL;
    static constexpr auto mod_key_r = KEY_RIGHT_CONTROL;
#else
    static constexpr auto mod_key_l = KEY_LEFT_SUPER;
    static constexpr auto mod_key_r = KEY_RIGHT_SUPER;
#endif //__APPLE__

public:
    static auto mod_down() -> bool {
        return IsKeyDown(mod_key_l) || IsKeyDown(mod_key_r);
    }

    static auto ctrl_down() -> bool {
        return IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    }

    static auto alt_down() -> bool {
        return IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);
    }

    static auto shift_down() -> bool {
        return IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    }

    static auto run() -> int;
    static auto run_no_gui() -> int;
};

#endif //RPY_PROJ_ANALYZER_APP_HPP
