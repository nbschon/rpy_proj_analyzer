//
// Created by Noah on 10/9/2025.
//

#ifndef RPY_PROJ_ANALYZER_TOKEN_HPP
#define RPY_PROJ_ANALYZER_TOKEN_HPP

#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

enum class OpType : std::uint8_t {
    Not,
    Plus,
    Minus,
    Mult,
    Div,
    Assign,
    PlusEq,
    MinusEq,
    MultEq,
    DivEq,
    Eq,
    NotEq,
    Less,
    LessEq,
    Greater,
    GreaterEq,
    In,
    And,
    Or,
    Neg,
};

struct Tok {
    unsigned line;
    unsigned col;
    unsigned indent;
    static constexpr std::string_view type_name = "Tok";
};

struct TokDollarSign : Tok {
    static constexpr std::string_view type_name = "DollarSign";
};

struct TokColon : Tok {
    static constexpr std::string_view type_name = "Colon";
};

struct TokLParen : Tok {
    static constexpr std::string_view type_name = "LParen";
};

struct TokRParen : Tok {
    static constexpr std::string_view type_name = "RParen";
};

struct TokLCurly : Tok {
    static constexpr std::string_view type_name = "LCurly";
};

struct TokRCurly : Tok {
    static constexpr std::string_view type_name = "RCurly";
};

struct TokLBracket : Tok {
    static constexpr std::string_view type_name = "LBracket";
};

struct TokRBracket : Tok {
    static constexpr std::string_view type_name = "RBracket";
};

struct TokComma : Tok {
    static constexpr std::string_view type_name = "Comma";
};

struct TokShow : Tok {
    static constexpr std::string_view type_name = "Show";
};

struct TokHide : Tok {
    static constexpr std::string_view type_name = "Hide";
};

struct TokMenu : Tok {
    static constexpr std::string_view type_name = "Menu";
};

struct TokChoice : Tok {
    static constexpr std::string_view type_name = "Choice";
};

struct TokTransition : Tok {
    static constexpr std::string_view type_name = "Transition";
};

struct TokPos : Tok {
    static constexpr std::string_view type_name = "Position";
};

struct TokLabel : Tok {
    static constexpr std::string_view type_name = "Label";
};

struct TokScene : Tok {
    static constexpr std::string_view type_name = "Scene";
};

struct TokIdent : Tok {
    std::string name;
    static constexpr std::string_view type_name = "Identifier";
};

struct TokStrLit : Tok {
    std::string text;
    static constexpr std::string_view type_name = "String Literal";
};

struct TokNumLit : Tok {
    static constexpr std::string_view type_name = "Numeric Literal";
    double value;
};

struct TokBoolLit : Tok {
    static constexpr std::string_view type_name = "Boolean Literal";
    bool value;
};

struct TokOp : Tok {
    static constexpr std::string_view type_name = "Operator";
    OpType type;
};

struct TokDefault : Tok {
    static constexpr std::string_view type_name = "Default";
};

struct TokDefine : Tok {
    static constexpr std::string_view type_name = "Define";
};

struct TokSet : Tok {
    static constexpr std::string_view type_name = "Set";
};

struct TokPlay : Tok {
    static constexpr std::string_view type_name = "Play";
};

struct TokMusic : Tok {
    static constexpr std::string_view type_name = "Music";
};

struct TokSfx : Tok {
    static constexpr std::string_view type_name = "Sfx";
};

struct TokIf : Tok {
    static constexpr std::string_view type_name = "If";
};

struct TokElif : Tok {
    static constexpr std::string_view type_name = "Elif";
};

struct TokElse : Tok {
    static constexpr std::string_view type_name = "Else";
};

struct TokWhile : Tok {
    static constexpr std::string_view type_name = "While";
};

struct TokReturn : Tok {
    static constexpr std::string_view type_name = "Return";
};

struct TokCall : Tok {
    static constexpr std::string_view type_name = "Call";
};

struct TokJump : Tok {
    static constexpr std::string_view type_name = "Jump";
};

struct TokCharacter : Tok {
    static constexpr std::string_view type_name = "Character";
};

struct TokImage : Tok {
    static constexpr std::string_view type_name = "Image";
};

struct TokNewline : Tok {
    static constexpr std::string_view type_name = "Newline";
};

struct TokTab : Tok {
    static constexpr std::string_view type_name = "Tab";
};

using Token = std::variant<
    TokDollarSign,
    TokColon,
    TokLParen,
    TokRParen,
    TokLCurly,
    TokRCurly,
    TokLBracket,
    TokRBracket,
    TokComma,
    TokShow,
    TokHide,
    TokMenu,
    TokChoice,
    TokTransition,
    TokPos,
    TokLabel,
    TokScene,
    TokIdent,
    TokStrLit,
    TokNumLit,
    TokBoolLit,
    TokOp,
    TokDefault,
    TokDefine,
    TokSet,
    TokPlay,
    TokMusic,
    TokSfx,
    TokIf,
    TokElif,
    TokElse,
    TokWhile,
    TokReturn,
    TokCall,
    TokJump,
    TokCharacter,
    TokImage,
    TokNewline,
    TokTab>;

inline auto op_str(const OpType &type) -> std::string {
    switch (type) {
        case OpType::Not:
            return "not";
        case OpType::Plus:
            return "+";
        case OpType::Minus:
            return "-";
        case OpType::Mult:
            return "*";
        case OpType::Div:
            return "/";
        case OpType::Assign:
            return "=";
        case OpType::PlusEq:
            return "+=";
        case OpType::MinusEq:
            return "-=";
        case OpType::MultEq:
            return "*=";
        case OpType::DivEq:
            return "/=";
        case OpType::Eq:
            return "==";
        case OpType::NotEq:
            return "!=";
        case OpType::Less:
            return "<";
        case OpType::LessEq:
            return "<=";
        case OpType::Greater:
            return ">";
        case OpType::GreaterEq:
            return ">=";
        case OpType::In:
            return "in";
        case OpType::And:
            return "and";
        case OpType::Or:
            return "or";
        case OpType::Neg:
            return "-";
        default:
            std::println(std::cerr, "Invalid operator in token");
            std::unreachable();
    }
}

template<class... Ts>
struct Overload : Ts... {
    using Ts::operator()...;
};

template<class... Ts>
Overload(Ts...) -> Overload<Ts...>;

template<typename T>
auto tok_name() -> std::string_view {
    return T::type_name;
}

[[nodiscard]] inline auto raw_str(const Token &tok) -> std::string {
    return std::visit(
        Overload{
            [&](const TokDollarSign &) -> std::string {
                return "$";
            },
            [&](const TokColon &) -> std::string {
                return ":";
            },
            [&](const TokLParen &) -> std::string {
                return "(";
            },
            [&](const TokRParen &) -> std::string {
                return ")";
            },
            [&](const TokLCurly &) -> std::string {
                return "{";
            },
            [&](const TokRCurly &) -> std::string {
                return "}";
            },
            [&](const TokLBracket &) -> std::string {
                return "[";
            },
            [&](const TokRBracket &) -> std::string {
                return "]";
            },
            [&](const TokComma &) -> std::string {
                return ",";
            },
            [&](const TokShow &) -> std::string {
                return "show";
            },
            [&](const TokHide &) -> std::string {
                return "hide";
            },
            [&](const TokMenu &) -> std::string {
                return "menu";
            },
            [&](const TokChoice &) -> std::string {
                return "ch";
            },
            [&](const TokTransition &) -> std::string {
                return "with";
            },
            [&](const TokPos &) -> std::string {
                return "at";
            },
            [&](const TokLabel &) -> std::string {
                return "label";
            },
            [&](const TokScene &) -> std::string {
                return "scene";
            },
            [&](const TokIdent &t) -> std::string {
                return t.name;
            },
            [&](const TokStrLit &t) -> std::string {
                return t.text;
            },
            [&](const TokNumLit &t) -> std::string {
                return std::format("{}", t.value);
            },
            [&](const TokBoolLit &t) -> std::string {
                return std::format("{}", t.value ? "True" : "False");
            },
            [&](const TokOp &t) -> std::string {
                return op_str(t.type);
            },
            [&](const TokDefault &) -> std::string {
                return "default";
            },
            [&](const TokDefine &) -> std::string {
                return "define";
            },
            [&](const TokSet &) -> std::string {
                return "set";
            },
            [&](const TokPlay &) -> std::string {
                return "play";
            },
            [&](const TokMusic &) -> std::string {
                return "music";
            },
            [&](const TokSfx &) -> std::string {
                return "sfx";
            },
            [&](const TokIf &) -> std::string {
                return "if";
            },
            [&](const TokElif &) -> std::string {
                return "elif";
            },
            [&](const TokElse &) -> std::string {
                return "else";
            },
            [&](const TokWhile &) -> std::string {
                return "while";
            },
            [&](const TokReturn &) -> std::string {
                return "return";
            },
            [&](const TokCall &) -> std::string {
                return "call";
            },
            [&](const TokJump &) -> std::string {
                return "jump";
            },
            [&](const TokCharacter &) -> std::string {
                return "Character";
            },
            [&](const TokImage &) -> std::string {
                return "image";
            },
            [&](const TokNewline &) -> std::string {
                return "\n";
            },
            [&](const TokTab &) -> std::string {
                return "\t";
            },
        }, tok);
}

auto tok_str(const Token &token) -> std::string;
auto tok_pos(const Token &token) -> std::string;
auto tok_pos(const Tok &token) -> std::string;
auto tok_color(const Token &token) -> std::uint32_t;

auto operator<<(std::ostream &stream, const Token &token) -> std::ostream &;

template<>
struct std::formatter<OpType> {
    constexpr auto parse(const std::format_parse_context &ctx) {
        return ctx.begin();
    }

    auto format(const OpType &op, std::format_context &ctx) const {
        return std::format_to(ctx.out(), "{}", op_str(op));
    }
};

template<>
struct std::formatter<Token> {
    bool raw = false;
    bool w_pos = false;
    bool w_colors = false;

    constexpr auto parse(const std::format_parse_context &ctx) {
        auto pos = ctx.begin();
        while (pos != ctx.end() && *pos != '}') {
            if (*pos == 'r' || *pos == 'R') {
                raw = true;
            }
            if (*pos == 'p' || *pos == 'P') {
                w_pos = true;
            }
            if (*pos == 'c' || *pos == 'C') {
                w_colors = true;
            }
            ++pos;
        }
        return pos;
    }

    auto format(const Token &token, std::format_context &ctx) const {
        std::string out_str = raw ? raw_str(token) : tok_str(token);
        if (w_pos) {
            out_str = std::format("{} {}", tok_pos(token), out_str);
        }
        if (w_colors) {
            // this will return the color formatted for raylib text drawing
            const auto color = tok_color(token);
            out_str = std::format("{{color=#{:06X}}}{}{{/color}}", color, out_str);
        }
        return std::format_to(ctx.out(), "{}", out_str);
    }
};

#endif //RPY_PROJ_ANALYZER_TOKEN_HPP
