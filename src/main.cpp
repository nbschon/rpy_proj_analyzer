#include <print>

#include "App.hpp"
#include "ArgVParser.hpp"

auto main(const int argc, char** argv) -> int {
    if (!ArgVParser::parse(argc, argv)) {
        return 1;
    }

    if (ArgVParser::help()) {
        std::println("{}", ArgVParser::get_help_msg());
        return 0;
    }

    if (ArgVParser::no_gui()) {
        return App::run_no_gui();
    }

    return App::run();
}
