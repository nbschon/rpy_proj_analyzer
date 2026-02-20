//
// Created by Noah Schonhorn on 11/11/25.
//

#include "Node.hpp"

#include <algorithm>
#include <format>
#include <utility>

auto operator<<(std::ostream& o, const Node& node) -> std::ostream& {
    o << std::string(node.indent, '\t') << node.to_string();
    return o;
}

Node::Node(const Tok& tok)
    : line(tok.line), col(tok.col), indent(tok.indent) {
}

Node::Node(const unsigned line, const unsigned col, const unsigned indent)
    : line(line), col(col), indent(indent) {
}

auto Node::has_children() const -> bool {
    return false;
}

// auto Node::line_col_str() const -> std::string {
//     return std::format("({:03d}:{:02d})", line, col);
// }

auto Node::line_and_col() const -> std::pair<unsigned, unsigned> {
    return {line, col};
}

NodeParent::NodeParent(const Tok& tok)
    : Node(tok) {
}

NodeParent::NodeParent(const unsigned line, const unsigned col, const unsigned indent)
    : Node(line, col, indent) {
}

auto NodeParent::has_children() const -> bool {
    return true;
}

NodeShow::NodeShow(const Tok& token, const std::string& name, const std::optional<std::string>& attr,
                   const std::optional<std::string>& pos, const std::optional<std::string>& trans)
    : Node(token) {
    this->name = name;
    this->attr = attr;
    this->pos = pos;
    this->trans = trans;
}

auto NodeShow::to_string() const -> std::string {
    auto ret = std::format("Show: \"{}\"", name);
    if (attr) {
        ret += std::format(" w/ attr \"{}\"", *attr);
    }
    if (pos) {
        ret += std::format(" at pos \"{}\"", *pos);
    }
    if (trans) {
        ret += std::format(" w/ trans. \"{}\"", *trans);
    }
    return ret;
}

auto NodeShow::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    // constexpr std::string title = "Show";
    std::vector<std::string> fields;
    fields.reserve(1 + attr.has_value() + pos.has_value() + trans.has_value());
    fields.push_back(std::format("Character: {}", name));
    if (attr) { fields.push_back(std::format("Attribute: {}", *attr)); }
    if (pos) { fields.push_back(std::format("Position: {}", *pos)); }
    if (trans) { fields.push_back(std::format("Transition: {}", *trans)); }
    return {this, rect, "Show", std::move(fields)};
}

NodeHide::NodeHide(const Tok& token, std::string name, const std::optional<std::string>& trans)
    : Node(token), name(std::move(name)), trans(trans) {
    // type = NodeType::Hide;
}

auto NodeHide::to_string() const -> std::string {
    auto ret = std::format("Hide: \"{}\"", name);
    if (trans) {
        ret += std::format(" w/ trans. \"{}\"", *trans);
    }
    return ret;
}

auto NodeHide::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    // constexpr std::string title = "Hide";
    std::vector<std::string> fields;
    fields.reserve(trans.has_value() + 1);
    fields.push_back(std::format("Character: {}", name));
    if (trans) { fields.push_back(std::format("Transition: {}", *trans)); }
    return {this, rect, "Hide", std::move(fields)};
}

NodeMenu::NodeMenu(const Tok& token, const std::optional<std::string>& text, const std::optional<std::string>& set)
    : NodeParent(token), text(text), set(set) {
}

auto NodeMenu::to_string() const -> std::string {
    std::string ret = "Menu: ";
    if (text) {
        ret += std::format("{} ", *text);
    }
    if (set) {
        ret += std::format(" using set {}", *set);
    }
    return ret;
}

auto NodeMenu::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    // constexpr std::string title = "Menu";
    std::vector<std::string> fields;
    fields.reserve(text.has_value() + set.has_value());
    if (text) { fields.push_back(std::format("\"{}\"", *text)); }
    if (set) { fields.push_back(std::format("Using set: {}", *set)); }
    return {this, rect, "Menu", std::move(fields)};
}

NodeChoice::NodeChoice(const Tok& token, std::string text)
    : NodeParent(token), text(std::move(text)) {
}

auto NodeChoice::to_string() const -> std::string {
    return std::format("Choice: \"{}\"", text);
}

auto NodeChoice::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    // constexpr std::string title = "Choice";
    return {this, rect, "Choice", {std::format("\"{}\"", text)}};
}

NodeLabel::NodeLabel(const Tok& token, std::string name)
    : NodeParent(token), name(std::move(name)) {
}

auto NodeLabel::to_string() const -> std::string {
    return std::format("Label: \"{}\"", name);
}

auto NodeLabel::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    // constexpr std::string title = "Label";
    return {this, rect, "Label", {name}};
}

NodeScene::NodeScene(const Tok& token, std::string name)
    : Node(token), name(std::move(name)) {
}

auto NodeScene::to_string() const -> std::string {
    return std::format("Scene \"{}\"", name);
}

auto NodeScene::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    // constexpr std::string title = "Scene";
    return {this, rect, "Scene", {std::format("Name: {}", name)}};
}

NodeDialogue::NodeDialogue(const Tok& token, std::string name, std::string text)
    : Node(token), name(std::move(name)), text(std::move(text)) {
}

NodeDialogue::NodeDialogue(const Tok& token, std::string text)
    : Node(token), name(std::nullopt), text(std::move(text)) {
}

auto NodeDialogue::to_string() const -> std::string {
    if (name) {
        return std::format(R"("{}": "{}")", *name, text);
    }

    return std::format("\"{}\"", text);
}

auto NodeDialogue::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    // constexpr std::string title = "Dialogue";
    std::vector<std::string> fields;
    fields.reserve(name.has_value() + 1); // text is always filled next
    if (name) { fields.push_back(std::format("Character: {}", *name)); }
    fields.push_back(std::format("\"{}\"", text));
    return {this, rect, "Dialogue", std::move(fields)};
}

NodeExpr::NodeExpr(const Tok& token, const std::span<const Token> expr_toks)
    : Node(token),
      expr_str(std::ranges::fold_left(expr_toks, std::string{}, [](std::string out, const Token& t) {
          out += std::format("{:r} ", t);
          return out;
      })),
      color_str(std::ranges::fold_left(expr_toks, std::string{}, [](std::string out, const Token& t) {
          out += std::format("{:cr} ", t);
          return out;
      })) {
    unsigned idx = 0;
    expr = fold_into_expr(expr_toks, idx);
    if (is_valid_assign(expr.get())) {
        type = DeclareType::Python;
    } else {
        type = DeclareType::None;
    }
}

NodeExpr::NodeExpr(const Tok& token, std::span<const Token> expr_toks, std::unique_ptr<Expr> expr, const bool ro)
    : Node(token), expr(std::move(expr)),
    expr_str(std::ranges::fold_left(expr_toks, std::string{}, [](std::string out, const Token& t) {
        out += std::format("{:r} ", t);
        return out;
    })),
    color_str(std::ranges::fold_left(expr_toks, std::string{}, [](std::string out, const Token& t) {
        out += std::format("{:cr} ", t);
        return out;
    })) {
    if (ro) {
        type = DeclareType::Define;
    } else {
        type = DeclareType::Default;
    }
}

auto NodeExpr::to_string() const -> std::string {
    if (type == DeclareType::Default) {
        return std::format("Default Init: {}", expr_str);
    }
    if (type == DeclareType::Define) {
        return std::format("Define Init: {}", expr_str);
    }

    return std::format("Expression: {}", expr_str);
}

auto NodeExpr::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    std::string title;
    std::vector fields = {color_str};
    if (type == DeclareType::Default) {
        title = "Default Init";
    } else if (type == DeclareType::Define) {
        title = "Define Init";
        fields.emplace_back("({i}read only{/i})");
    } else {
        title = "Expression";
    }

    return {this, rect, title, fields};
}

NodePlay::NodePlay(const Tok& token, const AudioChannel channel, std::string path)
    : Node(token), channel(channel), path(std::move(path)) {
}

auto NodePlay::to_string() const -> std::string {
    std::string ch_str;
    switch (channel) {
        case AudioChannel::Music:
            ch_str = "Music";
            break;
        case AudioChannel::Sfx:
            ch_str = "SFX";
            break;
    }

    return std::format("Play {}: \"{}\"", ch_str, path);
}

auto NodePlay::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    // std::string title = "Play";
    std::vector<std::string> fields;
    fields.reserve(2);
    switch (channel) {
        case AudioChannel::Music:
            fields.emplace_back("Channel: Music");
            break;
        case AudioChannel::Sfx:
            fields.emplace_back("Channel: SFX");
            break;
    }
    fields.push_back(std::format("File path: \"{}\"", path));
    return {this, rect, "Play", std::move(fields)};
}

NodeIf::NodeIf(const Tok& token, const std::span<const Token> expr_toks)
    : NodeParent(token),
      expr_str(std::ranges::fold_left(expr_toks, std::string{}, [](std::string out, const Token& t) {
          out += std::format("{:r} ", t);
          return out;
      })),
      color_str(std::ranges::fold_left(expr_toks, std::string{}, [](std::string out, const Token& t) {
          out += std::format("{:cr} ", t);
          return out;
      })) {
    unsigned idx = 0;
    expr = fold_into_expr(expr_toks, idx);
}

auto NodeIf::to_string() const -> std::string {
    return std::format("If: {}", expr_str);
}

auto NodeIf::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    // constexpr std::string title = "If";
    return {this, rect, "If", {color_str}};
}

NodeElif::NodeElif(const Tok& token, const std::span<const Token> expr_toks)
    : NodeParent(token.line, token.col, token.indent),
      expr_str(std::ranges::fold_left(expr_toks, std::string{}, [](std::string out, const Token& t) {
          out += std::format("{:r} ", t);
          return out;
      })),
      color_str(std::ranges::fold_left(expr_toks, std::string{}, [](std::string out, const Token& t) {
          out += std::format("{:cr} ", t);
          return out;
      })) {
    unsigned idx = 0;
    expr = fold_into_expr(expr_toks, idx);
}

auto NodeElif::to_string() const -> std::string {
    return std::format("Elif: {}", expr_str);
}

auto NodeElif::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    // constexpr std::string title = "Elif";
    std::vector<std::string> fields;
    fields.push_back(color_str);
    return {this, rect, "Elif", std::move(fields)};
}

NodeElse::NodeElse(const Tok& token)
    : NodeParent(token) {
}

auto NodeElse::to_string() const -> std::string {
    return "Else:";
}

auto NodeElse::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    // constexpr std::string title = "Else";
    return DisplayNode(this, rect, "Else");
}

NodeWhile::NodeWhile(const Tok& token, const std::span<const Token> expr_toks)
    : NodeParent(token.line, token.col, token.indent),
      expr_str(std::ranges::fold_left(expr_toks, std::string{}, [](std::string out, const Token& t) {
          out += std::format("{:r} ", t);
          return out;
      })),
      color_str(std::ranges::fold_left(expr_toks, std::string{}, [](std::string out, const Token& t) {
          out += std::format(
              "{:cr} ", t);
          return out;
      })) {
    unsigned idx = 0;
    expr = fold_into_expr(expr_toks, idx);
}

auto NodeWhile::to_string() const -> std::string {
    return std::format("While: {}", expr_str);
}

auto NodeWhile::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    // constexpr std::string title = "While";
    std::vector<std::string> fields;
    fields.push_back(color_str);
    return {this, rect, "While", std::move(fields)};
}

NodeReturn::NodeReturn(const Tok& token)
    : Node(token) {
    expr = nullptr;
}

NodeReturn::NodeReturn(const Tok& token, const std::span<const Token> expr_toks)
    : Node(token.line, token.col, token.indent),
      expr_str(std::ranges::fold_left(expr_toks, std::string{}, [](std::string out, const Token& t) {
          out += std::format(
              "{:r} ", t);
          return out;
      })),
      color_str(std::ranges::fold_left(expr_toks, std::string{}, [](std::string out, const Token& t) {
          out += std::format(
              "{:cr} ", t);
          return out;
      })) {
    unsigned idx = 0;
    expr = fold_into_expr(expr_toks, idx);
}

auto NodeReturn::to_string() const -> std::string {
    if (expr) {
        return std::format("Return: {}", expr_str);
    }

    return "Return";
}

auto NodeReturn::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    // constexpr std::string title = "Return";
    if (expr) {
        std::vector<std::string> fields;
        fields.push_back(color_str);
        return {this, rect, "Return", std::move(fields)};
    }
    return DisplayNode(this, rect, "Return");
}

NodeCall::NodeCall(const Tok& token, std::string label)
    : Node(token), label(std::move(label)) {
}

auto NodeCall::to_string() const -> std::string {
    return std::format("Call label: {}", label);
}

auto NodeCall::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    // constexpr std::string title = "Call";
    std::vector<std::string> fields;
    fields.push_back(std::format("Label: {}", label));
    return {this, rect, "Call", std::move(fields)};
}

NodeJump::NodeJump(const Tok& token, std::string label)
    : Node(token), label(std::move(label)) {
}

auto NodeJump::to_string() const -> std::string {
    return std::format("Jump to label: {}", label);
}

auto NodeJump::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    // constexpr std::string title = "Jump";
    std::vector<std::string> fields;
    fields.push_back(std::format("Label: {}", label));
    return {this, rect, "Jump", std::move(fields)};
}

NodeCharacter::NodeCharacter(const Tok& token, std::string name, std::string display_name)
    : Node(token), name(std::move(name)), display_name(std::move(display_name)), color(std::nullopt) {
}

NodeCharacter::NodeCharacter(const Tok& token, std::string name, std::string display_name, unsigned color)
    : Node(token), name(std::move(name)), display_name(std::move(display_name)), color(color) {
}

auto NodeCharacter::to_string() const -> std::string {
    if (color) {
        return std::format(R"(Character "{}", display: "{}", color: {:06X})", name, display_name, *color);
    }

    return std::format(R"(Character "{}", display: "{}")", name, display_name);
}

auto NodeCharacter::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    // constexpr std::string title = "Character";
    std::vector<std::string> fields;
    fields.reserve(2);
    if (color) {
        fields.push_back(std::format("Name: {{color=#{:06X}}}{}{{/color}}", *color, name));
    } else {
        fields.push_back(std::format("Name: {}", name));
    }
    fields.push_back(std::format("Display name: {}", display_name));
    return {this, rect, "Character", std::move(fields)};
}

NodeImage::NodeImage(const Tok& token, std::string char_name, std::string attr, std::string file_path)
    : Node(token), char_name(std::move(char_name)), attr(std::move(attr)), file_path(std::move(file_path)) {
}

auto NodeImage::to_string() const -> std::string {
    return std::format(R"(Image "{} {}", path: "{}")", char_name, attr, file_path);
}

auto NodeImage::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    // constexpr std::string title = "Image";
    std::vector<std::string> fields;
    fields.reserve(3);
    fields.push_back(std::format("Character: \"{}\"", char_name));
    fields.push_back(std::format("Attribute: \"{}\"", attr));
    fields.push_back(std::format("File path: \"{}\"", file_path));
    return {this, rect, "Image", std::move(fields)};
}

// NodeIfChain::NodeIfChain(const Tok& token,
//                          const unsigned if_idx, std::vector<unsigned> elif_idxs, const std::optional<unsigned> else_idx)
//     : NodeParent(token.line, token.col, token.indent), if_idx(if_idx), elif_idxs(std::move(elif_idxs)), else_idx(else_idx) {
// }
//
// auto NodeIfChain::to_string() const -> std::string {
//     return "=== If Chain ===";
// }
//
// auto NodeIfChain::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
//     const auto title = "If Chain";
//     return DisplayNode(this, rect, title);
// }
