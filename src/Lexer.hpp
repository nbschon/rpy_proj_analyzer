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
#include <type_traits>
#include <vector>

template<class T, class... Us>
struct variant_has;

template<class T, class... Us>
struct variant_has<T, std::variant<Us...>>
    : std::bool_constant<(std::is_same_v<T, Us> || ...)> {};

template <class T>
concept InTokens = variant_has<T, std::remove_cvref_t<Token>>::value;

class Lexer {
    std::string input_str;
    std::vector<Token> tokens;

    static constexpr unsigned TAB_WIDTH = 4;

    unsigned line = 1;
    unsigned col = 1;
    unsigned offset = 0;
    unsigned indent_level = 0;

    unsigned idx = 0;

    auto peek() -> std::optional<char>;
    auto consume() -> char;
    [[nodiscard]] auto identifiers() const -> std::set<std::string>;

    void parse_num(bool starts_neg = false);
    void remove_empty_lines();
    [[nodiscard]] auto get_str_lit() -> std::string;

public:
    template<typename T>
    requires InTokens<T>
    [[nodiscard]] auto expect() -> std::expected<T, std::string> {
        auto const& tok = tokens.at(idx);
        if (std::holds_alternative<T>(tok)) {
            return std::get<T>(tokens.at(idx++));
        }

        const std::string actual = std::visit([]<typename U>(U const& t) -> std::string {
            return std::format("{} at {}", tok_name<std::decay_t<U>>(), tok_pos(t));
        }, tok);
        return std::unexpected(std::format("expected {}, got {}", tok_name<T>(), actual));
    }

    /**
     * @brief returns a string containing the potential expected tokens, and the actual one.
     *
     * @tparam Ts all potential types that should've been encountered instead.
     */
    template<typename... Ts>
    requires (InTokens<Ts> && ...)
    [[nodiscard]] auto multi_tok_error(const std::vector<std::string_view> &other = {}) -> std::string {
        std::vector<std::string_view> names;
        ((names.push_back(tok_name<Ts>())), ...);

        if (!other.empty()) {
            for (const auto &o : other) {
                names.push_back(o);
            }
        }

        auto const &tok = tokens.at(idx);
        const std::string actual = std::visit([]<typename V>(V const &t) -> std::string {
            return std::format("{} at {}", tok_name<std::decay_t<V>>(), tok_pos(t));
        }, tok);

        std::string expected;
        switch (names.size()) {
            case 1:
                expected = names.at(0);
                break;
            case 2:
                expected = std::format("{} or {}", names.at(0), names.at(1));
                break;
            default:
                for (int i = 0; i < names.size() - 1; i++) {
                    expected += std::format("{}, ", names.at(i));
                }
                expected += std::format("or {}", names.back());
                break;
        }

        return std::format("expected {}, got {}", expected, actual);
    }

    /**
     * @brief returns whether the current token is any of the given types.
     *
     * @tparam Ts any types we want to know if being at the current index.
     */
    template<typename... Ts>
    requires (InTokens<Ts> && ...)
    [[nodiscard]] auto curr_is() const -> bool {
        return (std::holds_alternative<Ts>(tokens.at(idx)) || ...);
    }

    /**
     * @brief returns whether the current token is NOT any of the given types.
     *
     * @tparam Ts any types we want to know are not at the current index.
     */
    template<typename... Ts>
    requires (InTokens<Ts> && ...)
    [[nodiscard]] auto curr_is_not() const -> bool {
        return (!std::holds_alternative<Ts>(tokens.at(idx)) && ...);
    }

    explicit Lexer(const std::filesystem::path &path);
    auto tokenize() -> std::vector<Token>;
    [[nodiscard]] auto curr() const -> const Token&;
    void adv();
    auto get_tokens() -> std::vector<Token>&;
    [[nodiscard]] auto get_idx() const -> unsigned;
    [[nodiscard]] auto has_more() const -> bool;
    void print_tokens(unsigned n_lines = 0) const;

    // evil operator overloading...
    auto operator++() -> unsigned&;
    auto operator++(int) -> unsigned;
    auto operator--() -> unsigned&;
    auto operator--(int) -> unsigned;
};


#endif //RPY_PROJ_ANALYZER_LEXER_HPP
