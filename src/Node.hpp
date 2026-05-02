//
// Created by Noah Schonhorn on 11/11/25.
//

#ifndef RPY_PROJ_ANALYZER_NODE_HPP
#define RPY_PROJ_ANALYZER_NODE_HPP

#include "ATL.hpp"
#include "DisplayNode.hpp"
#include "Expr.hpp"
#include "Token.hpp"

#include <format>
#include <memory>
#include <optional>
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

struct ShowProps {
    std::optional<std::string> as;
    std::vector<std::string> transforms;
    std::optional<std::string> behind;
    std::optional<std::string> onlayer;
    std::optional<int> zorder;
    std::vector<ATLStmt> atl_stmts;
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

    explicit Node(const Tok &tok);
    Node(unsigned line, unsigned col, unsigned indent);

    virtual ~Node() = default;

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

    explicit NodeParent(const Tok& tok);
    NodeParent(unsigned line, unsigned col, unsigned indent);

    [[nodiscard]] auto has_children() const -> bool override;
};

class NodeShow final : public Node {
    std::string name;
    std::vector<std::string> attrs;
    std::optional<std::string> as;
    std::vector<std::string> transforms;
    std::optional<std::string> behind;
    std::optional<std::string> onlayer;
    std::optional<int> zorder;
    std::vector<ATLStmt> atl_stmts;
    bool is_scene = false;

public:
    explicit NodeShow(const Tok& token, std::string name, std::vector<std::string> attrs, ShowProps& props, bool is_scene = false);

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

class NodeHide final : public Node {
    std::string name;
    std::optional<std::string> onlayer;

public:
    explicit NodeHide(const Tok& token, std::string name, std::optional<std::string> onlayer);

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

class NodeWith final : public Node {
    std::unique_ptr<Expr> trans;
    std::string expr_str;
    std::string display_str;

public:
    explicit NodeWith(const Tok& token, std::span<const Token> expr_toks);
    NodeWith(const Tok& token, const Transition &trans);

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

class NodeMenu final : public NodeParent {
    std::optional<std::string> text;
    std::optional<std::string> set;

public:
    explicit NodeMenu(const Tok& token, const std::optional<std::string>& text, const std::optional<std::string>& set);

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

class NodeChoice final : public NodeParent {
    std::string text;
    std::optional<std::string> expr_str;
    std::optional<std::string> display_str;
    std::unique_ptr<Expr> clause = nullptr;

public:
    explicit NodeChoice(const Tok& token, std::string text);
    NodeChoice(const Tok& token, std::string text, std::span<const Token> expr_toks);

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
    std::string display_str;
    DeclareType type;

public:
    explicit NodeExpr(const Tok& token, std::span<const Token> expr_toks);

    NodeExpr(const Tok& token, std::span<const Token> expr_toks, std::unique_ptr<Expr> expr, bool ro); // "ro" i.e. read only

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

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

class NodePass final : public Node {
public:
    explicit NodePass(const Tok& token);
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

class NodeImage final : public Node {
    std::string char_name;
    std::vector<std::string> attrs;
    std::string file_path;

public:
    explicit NodeImage(const Tok& token, std::string char_name, std::vector<std::string> attrs, std::string file_path);

    [[nodiscard]] auto to_string() const -> std::string override;

    [[nodiscard]] auto make_display_node(raylib::Rectangle rect) const -> DisplayNode override;
};

// class NodeTransform final : public NodeParent {
//     std::string name;
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
