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

NodeShow::NodeShow(const Tok& token, std::string name, std::vector<std::string> attrs, ShowProps& props, bool is_scene)
    : Node(token), name(std::move(name)), attrs(std::move(attrs)), is_scene(is_scene) {
    if (props.as) {
        as = std::move(props.as);
    }
    if (!props.transforms.empty()) {
        transforms = std::move(props.transforms);
    }
    if (props.behind) {
        behind = std::move(props.behind);
    }
    if (props.onlayer) {
        onlayer = std::move(props.onlayer);
    }
    if (props.zorder) {
        zorder = props.zorder;
    }
}

auto NodeShow::to_string() const -> std::string {
    auto ret = is_scene ? std::format("Scene: \"{}\"", name) : std::format("Show: \"{}\"", name);
    if (!attrs.empty()) {
        ret += std::ranges::fold_left(attrs, " w/ attrs", [](std::string out, const std::string& s) {
            out += std::format(" \"{}\",", s);
            return out;
        });
        if (ret.ends_with(',')) {
            ret.pop_back();
        }
    }
    if (!transforms.empty()) {
        ret += std::ranges::fold_left(transforms, " w/ transforms", [](std::string out, const std::string& s) {
            out += std::format(" \"{}\",", s);
            return out;
        });
        if (ret.ends_with(',')) {
            ret.pop_back();
        }
    }
    return ret;
}

auto NodeShow::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    std::vector<std::string> fields;
    fields.reserve(1 + attrs.size() + transforms.size());
    if (is_scene) {
        fields.push_back(std::format("Image: {}", name));
    } else {
        fields.push_back(std::format("Character: {}", name));
    }
    if (!attrs.empty()) {
        auto attrs_str = std::ranges::fold_left(attrs, "Attributes:", [](std::string out, const std::string& s) {
                out += std::format(" \"{}\",", s);
                return out;
            });
        if (attrs_str.ends_with(',')) {
            attrs_str.pop_back();
        }
        fields.push_back(attrs_str);
    }
    if (!transforms.empty()) {
        auto tf_str = std::ranges::fold_left(transforms, "Transforms:", [](std::string out, const std::string& s) {
                out += std::format(" \"{}\",", s);
                return out;
            });
        if (tf_str.ends_with(',')) {
            tf_str.pop_back();
        }
        fields.push_back(tf_str);
    }
    return {this, rect, is_scene ? "Scene" : "Show", std::move(fields)};
}

NodeHide::NodeHide(const Tok& token, std::string name, std::optional<std::string> onlayer)
    : Node(token), name(std::move(name)), onlayer(std::move(onlayer)) {
}

auto NodeHide::to_string() const -> std::string {
    auto ret = std::format("Hide: \"{}\"", name);
    return ret;
}

auto NodeHide::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    std::vector<std::string> fields;
    fields.reserve(1);
    fields.push_back(std::format("Character: {}", name));
    return {this, rect, "Hide", std::move(fields)};
}

NodeWith::NodeWith(const Tok& token, std::span<const Token> expr_toks)
    : Node(token),
    expr_str(std::ranges::fold_left(expr_toks, std::string{}, [](std::string out, const Token& t) {
        out += std::format("{:r} ", t);
        return out;
    })),
    display_str(std::ranges::fold_left(expr_toks, std::string{}, [](std::string out, const Token& t) {
        out += std::format("{:cr} ", t);
        return out;
    })) {
    trans = fold_into_expr(expr_toks);
}

NodeWith::NodeWith(const Tok& token, const Transition& trans)
    : Node(token),
    trans(std::make_unique<ExprLit>(ATL::trans_str(trans))),
    expr_str(ATL::trans_str(trans)),
    display_str(ATL::trans_str(trans)) {
}

auto NodeWith::to_string() const -> std::string {
    return std::format("With: {}", expr_str);
}

auto NodeWith::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    return {this, rect, "With", {display_str}};
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
    std::vector<std::string> fields;
    fields.reserve(text.has_value() + set.has_value());
    if (text) { fields.push_back(std::format("\"{}\"", *text)); }
    if (set) { fields.push_back(std::format("Using set: {}", *set)); }
    return {this, rect, "Menu", std::move(fields)};
}

NodeChoice::NodeChoice(const Tok& token, std::string text)
    : NodeParent(token), text(std::move(text)) {
}

NodeChoice::NodeChoice(const Tok& token, std::string text, std::span<const Token> expr_toks)
    : NodeParent(token), text(std::move(text)),
    expr_str(std::ranges::fold_left(expr_toks, std::string{}, [](std::string out, const Token& t) {
        out += std::format("{:r} ", t);
        return out;
    })),
    display_str(std::ranges::fold_left(expr_toks, std::string{}, [](std::string out, const Token& t) {
        out += std::format("{:r} ", t);
        return out;
    })) {
    clause = fold_into_expr(expr_toks);
}

auto NodeChoice::to_string() const -> std::string {
    return std::format("Choice: \"{}\"", text);
}

auto NodeChoice::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    if (clause != nullptr) {
        return {this, rect, "Choice", {
            std::format("\"{}\"", text),
            std::format("Clause: {}", *display_str),
        }};
    }
    return {this, rect, "Choice", {std::format("\"{}\"", text)}};
}

NodeLabel::NodeLabel(const Tok& token, std::string name)
    : NodeParent(token), name(std::move(name)) {
}

auto NodeLabel::to_string() const -> std::string {
    return std::format("Label: \"{}\"", name);
}

auto NodeLabel::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    return {this, rect, "Label", {name}};
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
      display_str(std::ranges::fold_left(expr_toks, std::string{}, [](std::string out, const Token& t) {
          out += std::format("{:cr} ", t);
          return out;
      })) {
    expr = fold_into_expr(expr_toks);
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
    display_str(std::ranges::fold_left(expr_toks, std::string{}, [](std::string out, const Token& t) {
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
    std::vector fields = {display_str};
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
    expr = fold_into_expr(expr_toks);
}

auto NodeIf::to_string() const -> std::string {
    return std::format("If: {}", expr_str);
}

auto NodeIf::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
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
    expr = fold_into_expr(expr_toks);
}

auto NodeElif::to_string() const -> std::string {
    return std::format("Elif: {}", expr_str);
}

auto NodeElif::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
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
    expr = fold_into_expr(expr_toks);
}

auto NodeWhile::to_string() const -> std::string {
    return std::format("While: {}", expr_str);
}

auto NodeWhile::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
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
    expr = fold_into_expr(expr_toks);
}

auto NodeReturn::to_string() const -> std::string {
    if (expr) {
        return std::format("Return: {}", expr_str);
    }

    return "Return";
}

auto NodeReturn::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    if (expr) {
        std::vector<std::string> fields;
        fields.push_back(color_str);
        return {this, rect, "Return", std::move(fields)};
    }
    return DisplayNode(this, rect, "Return");
}

NodePass::NodePass(const Tok& token)
    : Node(token) {
}

auto NodePass::to_string() const -> std::string {
    return "Pass";
}

auto NodePass::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    return DisplayNode(this, rect, "Pass");
}

NodeCall::NodeCall(const Tok& token, std::string label)
    : Node(token), label(std::move(label)) {
}

auto NodeCall::to_string() const -> std::string {
    return std::format("Call label: {}", label);
}

auto NodeCall::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
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
    std::vector<std::string> fields;
    fields.push_back(std::format("Label: {}", label));
    return {this, rect, "Jump", std::move(fields)};
}

NodeImage::NodeImage(const Tok& token, std::string char_name, std::vector<std::string> attrs, std::string file_path)
    : Node(token), char_name(std::move(char_name)), attrs(std::move(attrs)), file_path(std::move(file_path)) {
}

auto NodeImage::to_string() const -> std::string {
    return std::format(R"(Image "{} {}", path: "{}")", char_name, attrs, file_path);
}

auto NodeImage::make_display_node(raylib::Rectangle rect) const -> DisplayNode {
    std::vector<std::string> fields;
    fields.reserve(3);
    fields.push_back(std::format("Character: \"{}\"", char_name));
    fields.push_back(std::format("Attributes:"));
    for (const auto &a : attrs) {
        fields.push_back(std::format("\t{}", a));
    }
    fields.push_back(std::format("File path: \"{}\"", file_path));
    return {this, rect, "Image", std::move(fields)};
}
