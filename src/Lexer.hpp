//
// Created by Noah on 10/9/2025.
//

#ifndef RPY_PROJ_ANALYZER_LEXER_HPP
#define RPY_PROJ_ANALYZER_LEXER_HPP

#include "Node.hpp"
#include "Token.hpp"

#include <filesystem>
#include <set>
#include <string>
#include <vector>

class Lexer {
    std::string input_str;
    std::vector<Token> tokens;

    static constexpr unsigned TAB_WIDTH = 4;

    unsigned line = 1;
    unsigned col = 1;
    unsigned offset = 0;
    unsigned indent_level = 0;

    auto peek() -> std::optional<char>;
    auto consume() -> char;
    [[nodiscard]] auto identifiers() const -> std::set<std::string>;

    void parse_num(bool starts_neg = false);
    void remove_empty_lines();
    [[nodiscard]] auto get_str_lit() -> std::string;

public:
    explicit Lexer(const std::filesystem::path &path);
    auto tokenize() -> std::vector<Token>;
    void print_tokens(unsigned n_lines = 0) const;
};


#endif //RPY_PROJ_ANALYZER_LEXER_HPP
