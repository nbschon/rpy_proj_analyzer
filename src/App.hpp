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

public:
    static auto run() -> int;
    static auto run_no_gui() -> int;
};

#endif //RPY_PROJ_ANALYZER_APP_HPP
