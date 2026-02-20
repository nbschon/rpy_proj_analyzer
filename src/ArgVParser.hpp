//
// Created by Noah Schonhorn on 2/6/26.
//

#ifndef RPY_PROJ_ANALYZER_ARGVPARSER_HPP
#define RPY_PROJ_ANALYZER_ARGVPARSER_HPP

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

class ArgVParser {
    enum class ParseErr : std::uint8_t {
        Invalid,
        NoneGiven,
    };

    static constexpr unsigned FLAG_HELP      = 0b1;
    static constexpr unsigned FLAG_DARK_MODE = 0b10;
    static constexpr unsigned FLAG_NO_GUI    = 0b100;
    static inline unsigned bit_flags = 0;

    static auto parse_int(const std::vector<std::string_view> &args, std::string_view arg, int &idx) -> std::optional<int>;

public:
    static inline std::optional<std::filesystem::path> path;
    static inline std::optional<int> threads;
    static inline std::optional<int> width;
    static inline std::optional<int> height;

    static auto parse(int argc, char** argv) -> bool;
    static auto get_help_msg() -> std::string;

    static auto dark_mode() -> bool;
    static auto help() -> bool;
    static auto no_gui() -> bool;
};


#endif //RPY_PROJ_ANALYZER_ARGVPARSER_HPP
