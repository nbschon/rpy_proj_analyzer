//
// Created by Noah Schonhorn on 2/6/26.
//

#include "ArgVParser.hpp"

#include <charconv>
#include <format>
#include <iostream>
#include <print>
#include <string_view>
#include <system_error>
#include <vector>

auto ArgVParser::parse_int(const std::vector<std::string_view>& args, const std::string_view arg, int &idx) -> std::optional<int> {
    if (idx + 1 < args.size()) {
        const auto& num = args.at(idx + 1);
        int as_num = 0;
        if (const auto [ptr, ec] = std::from_chars(num.data(), num.data() + num.size(), as_num);
            ec == std::errc::invalid_argument) {
            std::println(std::cerr, "invalid option given for {}", arg);
            return std::nullopt;
        }

        ++idx;
        return as_num;
    }

    std::println(std::cerr, "no option given for {}", arg);
    return std::nullopt;
}

auto ArgVParser::parse(const int argc, char** argv) -> bool {
    const std::vector<std::string_view> args(argv, argv + argc);

    if (args.empty()) {
        return true;
    }

    int i = 0;

    if (args.size() >= 2) {
        path = std::filesystem::path(args.at(1));
        if (std::filesystem::is_directory(*path) || (std::filesystem::is_regular_file(*path) && path->extension() == ".rpy")) {
        } else {
            path = std::nullopt;
        }
        i++;
    }

    bool parse_ok = true;

    while (i < args.size() && parse_ok) {
        if (const auto& arg = args.at(i); arg == "-h" || arg == "--help") {
            bit_flags |= FLAG_HELP;
        } else if (arg == "-d" || arg == "--dark-mode") {
            bit_flags |= FLAG_DARK_MODE;
        } else if (arg == "--no-gui") {
            bit_flags |= FLAG_NO_GUI;
        } else if (arg == "-t" || arg == "--threads") {
            threads = parse_int(args, arg, i);
            parse_ok = threads.has_value();
        } else if (arg == "-w" || arg == "--width") {
            width = parse_int(args, arg, i);
            parse_ok = width.has_value();
        } else if (arg == "-h" || arg == "--height") {
            height = parse_int(args, arg, i);
            parse_ok = height.has_value();
        }

        ++i;
    }

    return parse_ok;
}

auto ArgVParser::get_help_msg() -> std::string {
    return R"(
    usage: ./rpy_proj_analyzer <"file.rpy" | "directory"> <options...>

    -h, --help
        display this message and exit.

    -t [threads], --threads [threads]
        use N threads for parsing files.

    -w [width], --width [width] 
        set window to a given width.

    -h [height], --height [height]
        set window to a given height.

    -d, --dark-mode
        use dark colors instead of the light defaults.

    --no-gui
        runs the tool on a single script with no GUI.
    )";
}

auto ArgVParser::dark_mode() -> bool {
    return (bit_flags & FLAG_DARK_MODE) != 0;
}

auto ArgVParser::help() -> bool {
    return (bit_flags & FLAG_HELP) != 0;
}

auto ArgVParser::no_gui() -> bool {
    return (bit_flags & FLAG_NO_GUI) > 0;
}
