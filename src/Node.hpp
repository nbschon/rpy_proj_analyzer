//
// Created by Noah Schonhorn on 11/11/25.
//

#ifndef RPY_PROJ_ANALYZER_NODE_HPP
#define RPY_PROJ_ANALYZER_NODE_HPP

#include "DisplayNode.hpp"
#include "Expr.hpp"
#include "Token.hpp"

#include <cstdint>
#include <format>
#include <memory>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <vector>

#include "raylib-cpp.hpp"

enum class AudioChannel : std::uint8_t {
    Music,
    Sfx,
};

enum class DeclareType : std::uint8_t {
    None,
    Python,
    Default,
    Define,
};

enum class NodeType : std::uint8_t {
    Show,
    Hide,
    Menu,
    Choice,
    Label,
    Scene,
    Dialogue,
    Assign,
    Play,
    If,
    Elif,
    Else,
    While,
    Return,
    Call,
    Jump,
    IfChain,
};

class Node {
protected:
    unsigned line = 0;
    unsigned col = 0;

public:
    std::optional<unsigned> parent;
    std::optional<unsigned> next;
    std::optional<unsigned> prev;
    unsigned indent = 0;
    unsigned width = 0;
    int score_potential = 0;

    Node(const Tok &tok);
    Node(unsigned line, unsigned col, unsigned indent);

    virtual ~Node() = default;

    // i.e. meaning you must either proceed through children first
    // or simply continue to the following node

    [[nodiscard]] auto line_and_col() const -> std::pair<unsigned, unsigned>;

    [[nodiscard]] virtual auto to_string() const -> std::string = 0;

    [[nodiscard]] virtual auto has_children() const -> bool;

    [[nodiscard]] virtual auto make_display_node(raylib::Rectangle rect) const -> DisplayNode = 0;

    friend auto operator<<(std::ostream& o, const Node& node) -> std::ostream&;
};

class NodeParent : public Node {
public:
    std::optional<unsigned> first_child;
    std::optional<unsigned> after_block;

    NodeParent(const Tok& tok);
    NodeParent(unsigned line, unsigned col, unsigned indent);

    [[nodiscard]] auto has_children() const -> bool override;
};

class NodeShow final : public Node {
    std::string name;
    std::optional<std::string> attr;
    std::optional<std::string> pos;
    std::optional<std::string> trans;

public:
    explicit NodeShow(const Tok& token,
                      const std::string& name, const std::optional<std::string>& attr,
                      const std::optional<std::string>& pos, const std::optional<std::string>& trans);

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

class NodeHide final : public Node {
    std::string name;
    std::optional<std::string> trans;

public:
    explicit NodeHide(const Tok& token, std::string name, const std::optional<std::string>& trans);

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

class NodeMenu final : public NodeParent {
    std::optional<std::string> text;
    std::optional<std::string> set;
    std::vector<unsigned> choices;

public:
    explicit NodeMenu(const Tok& token, const std::optional<std::string>& text, const std::optional<std::string>& set);

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

class NodeChoice final : public NodeParent {
    std::string text;

public:
    explicit NodeChoice(const Tok& token, std::string text);

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

class NodeLabel final : public NodeParent {
    std::string name;

public:
    explicit NodeLabel(const Tok& token, std::string name);

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

class NodeScene final : public Node {
    std::string name;

public:
    explicit NodeScene(const Tok& token, std::string name);

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

class NodeDialogue final : public Node {
    std::optional<std::string> name;
    std::string text;

public:
    NodeDialogue(const Tok& token, std::string name, std::string text);

    NodeDialogue(const Tok& token, std::string text);

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

class NodeExpr final : public Node {
    std::unique_ptr<Expr> expr;
    std::string expr_str;
    std::string color_str;
    DeclareType type;

public:
    explicit NodeExpr(const Tok& token, std::span<const Token> expr_toks);

    NodeExpr(const Tok& token, std::span<const Token> expr_toks, std::unique_ptr<Expr> expr, bool ro);

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

// class NodeDeclare final : public Node {
//     std::unique_ptr<Expr> expr;
//     std::string expr_str;
//     std::string color_str;
//     bool read_only;
// };

class NodePlay final : public Node {
    AudioChannel channel;
    std::string path;

public:
    explicit NodePlay(const Tok& token, AudioChannel channel, std::string path);

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

class NodeIf final : public NodeParent {
    std::unique_ptr<Expr> expr;
    std::string expr_str;
    std::string color_str;

public:
    explicit NodeIf(const Tok& token, std::span<const Token> expr_toks);

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

class NodeElif final : public NodeParent {
    std::unique_ptr<Expr> expr;
    std::string expr_str;
    std::string color_str;

public:
    explicit NodeElif(const Tok& token, std::span<const Token> expr_toks);

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

class NodeElse final : public NodeParent {
public:
    explicit NodeElse(const Tok& token);

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

class NodeWhile final : public NodeParent {
    std::unique_ptr<Expr> expr;
    std::string expr_str;
    std::string color_str;

public:
    explicit NodeWhile(const Tok& token, std::span<const Token> expr_toks);

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

class NodeReturn final : public Node {
    std::unique_ptr<Expr> expr;
    std::string expr_str;
    std::string color_str;

public:
    explicit NodeReturn(const Tok& token);

    NodeReturn(const Tok& token, std::span<const Token> expr_toks);

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

class NodeCall final : public Node {
    std::string label;

public:
    explicit NodeCall(const Tok& token, std::string label);

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

class NodeJump final : public Node {
    std::string label;

public:
    explicit NodeJump(const Tok& token, std::string label);

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

class NodeCharacter final : public Node {
    std::string name;
    std::string display_name;
    std::optional<unsigned> color;

public:
    explicit NodeCharacter(const Tok& token, std::string name, std::string display_name);
    NodeCharacter(const Tok& token, std::string name, std::string display_name, unsigned color);

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

class NodeImage final : public Node {
    std::string char_name;
    std::string attr;
    std::string file_path;

public:
    explicit NodeImage(const Tok& token, std::string char_name, std::string attr, std::string file_path);

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

// class NodeIfChain final : public NodeParent {
//     unsigned if_idx;
//     std::vector<unsigned> elif_idxs;
//     std::optional<unsigned> else_idx;
//
// public:
//     explicit NodeIfChain(const Tok& token,
//                          unsigned if_idx, std::vector<unsigned> elif_idxs, std::optional<unsigned> else_idx);
//
//     [[nodiscard]] auto to_string() const -> std::string override;
//
//     [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
//
//     [[nodiscard]] auto get_if_idx() const -> unsigned {
//         return if_idx;
//     }
//
//     [[nodiscard]] auto get_elif_idxs() const -> std::vector<unsigned> {
//         return elif_idxs;
//     }
//
//     [[nodiscard]] auto get_else_idx() const -> std::optional<unsigned> {
//         return else_idx;
//     }
// };

template<>
struct std::formatter<Node> {
    bool has_tabs = false;
    bool has_pos = false;

    constexpr auto parse(const std::format_parse_context& ctx) {
        const auto *pos = ctx.begin();
        while (pos != ctx.end() && *pos != '}') {
            if (*pos == 't' || *pos == 'T') {
                has_tabs = true;
            }
            if (*pos == 'p' || *pos == 'P') {
                has_pos = true;
            }
            ++pos;
        }
        return pos;
    }

    auto format(const Node& node, std::format_context& ctx) const {
        std::string out = [&] {
            if (has_pos) {
                const auto [line, col] = node.line_and_col();
                return std::format("({:03d}:{:02d}) {}", line, col, node.to_string());
            }

            return node.to_string();
        }();

        if (has_tabs) {
            return std::format_to(ctx.out(), "{}{}", std::string(node.indent, '\t'), out);
        }
        return std::format_to(ctx.out(), "{}", out);
    }
};

#endif //RPY_PROJ_ANALYZER_NODE_HPP
