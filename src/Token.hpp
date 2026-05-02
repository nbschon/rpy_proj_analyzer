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
    Comma,
    LParen,
    RParen,
};

enum class TFProp : std::uint8_t;
enum class Warper : std::uint8_t;
enum class Transition : std::uint8_t;
enum class Event : std::uint8_t;

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

struct TokScene : Tok {
    static constexpr std::string_view type_name = "Scene";
};

struct TokNone : Tok {
    static constexpr std::string_view type_name = "None";
};

struct TokMenu : Tok {
    static constexpr std::string_view type_name = "Menu";
};

struct TokWith : Tok {
    static constexpr std::string_view type_name = "Transition";
};

struct TokAt : Tok {
    static constexpr std::string_view type_name = "Position";
};

struct TokAs : Tok {
    static constexpr std::string_view type_name = "As";
};

struct TokBehind : Tok {
    static constexpr std::string_view type_name = "Behind";
};

struct TokOnlayer : Tok {
    static constexpr std::string_view type_name = "On Layer";
};

struct TokZOrder : Tok {
    static constexpr std::string_view type_name = "Z Order";
};

struct TokLabel : Tok {
    static constexpr std::string_view type_name = "Label";
};

struct TokIdent : Tok {
    static constexpr std::string_view type_name = "Identifier";
    std::string name;
};

struct TokStrLit : Tok {
    static constexpr std::string_view type_name = "String Literal";
    std::string text;
};

struct TokIntLit : Tok {
    static constexpr std::string_view type_name = "Integer Literal";
    int value;
};

struct TokFloatLit : Tok {
    static constexpr std::string_view type_name = "Float Literal";
    // they're called `float` in Python, but are actually double precision. Go figure.
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

// struct TokSet : Tok {
//     static constexpr std::string_view type_name = "Set";
// };

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

struct TokPass : Tok {
    static constexpr std::string_view type_name = "Pass";
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

// ATL tokens start here:
struct TokTransform : Tok {
    static constexpr std::string_view type_name = "Transform";
};

struct TokATLProperty : Tok {
    static constexpr std::string_view type_name = "ATLProperty";
    TFProp type;
};

struct TokATLPause : Tok {
    static constexpr std::string_view type_name = "ATLPause";
};

struct TokATLWarp : Tok {
    static constexpr std::string_view type_name = "ATLWarp";
};

struct TokATLKnot : Tok {
    static constexpr std::string_view type_name = "ATLKnot";
};

struct TokATLClockwise : Tok {
    static constexpr std::string_view type_name = "ATLClockwise";
};

struct TokATLCCWise : Tok {
    static constexpr std::string_view type_name = "ATLCCWise";
};

struct TokATLCircles : Tok {
    static constexpr std::string_view type_name = "ATLCircles";
};

struct TokATLRepeat : Tok {
    static constexpr std::string_view type_name = "ATLRepeat";
};

struct TokATLBlock : Tok {
    static constexpr std::string_view type_name = "ATLBlock";
};

struct TokATLParallel : Tok {
    static constexpr std::string_view type_name = "ATLParallel";
};

struct TokATLChoice : Tok {
    static constexpr std::string_view type_name = "ATLChoice";
};

struct TokATLAnimation : Tok {
    static constexpr std::string_view type_name = "ATLAnimation";
};

struct TokATLOn : Tok {
    static constexpr std::string_view type_name = "ATLOn";
};

struct TokATLContains : Tok {
    static constexpr std::string_view type_name = "ATLContains";
};

struct TokATLFunction : Tok {
    static constexpr std::string_view type_name = "ATLFunction";
};

struct TokATLTime : Tok {
    static constexpr std::string_view type_name = "ATLTime";
};

struct TokATLEvent : Tok {
    static constexpr std::string_view type_name = "ATLEvent";
    Event event;
};

struct TokATLTransition : Tok {
    static constexpr std::string_view type_name = "ATLTransition";
    Transition trans;
};

struct TokATLWarper : Tok {
    static constexpr std::string_view type_name = "ATLWarper";
    Warper warper;
};

// ATL tokens end here.

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
    TokScene,
    TokNone,
    TokMenu,
    TokATLChoice,
    TokAs,
    TokAt,
    TokBehind,
    TokOnlayer,
    TokZOrder,
    TokWith,
    TokLabel,
    TokIdent,
    TokStrLit,
    TokIntLit,
    TokFloatLit,
    TokBoolLit,
    TokOp,
    TokDefault,
    TokDefine,
    // TokSet,
    TokPlay,
    TokMusic,
    TokSfx,
    TokIf,
    TokElif,
    TokElse,
    TokWhile,
    TokReturn,
    TokPass,
    TokCall,
    TokJump,
    TokImage,
    TokTransform,
    TokATLProperty,
    TokATLPause,
    TokATLWarp,
    TokATLKnot,
    TokATLClockwise,
    TokATLCCWise,
    TokATLCircles,
    TokATLRepeat,
    TokATLBlock,
    TokATLParallel,
    TokATLAnimation,
    TokATLOn,
    TokATLContains,
    TokATLFunction,
    TokATLTime,
    TokATLEvent,
    TokATLTransition,
    TokATLWarper,
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
        case OpType::Comma:
            return ",";
        case OpType::LParen:
            return "(";
        case OpType::RParen:
            return ")";
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

[[nodiscard]] auto raw_str(const Token &tok) -> std::string;

auto tok_str(const Token &token) -> std::string;
auto tok_pos(const Token &token) -> std::string;
auto tok_pos(const Tok &token) -> std::string;
auto tok_indent(const Token &token) -> unsigned;
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
        const auto *pos = ctx.begin();
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