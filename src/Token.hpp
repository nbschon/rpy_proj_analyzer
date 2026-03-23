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

#include "ATL.hpp"

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

/*
 * Transformation property information borrowed from here:
 * https://www.renpy.org/doc/html/transform_properties.html
 */
enum class TFPropType : std::uint8_t {
    /* Name of property    Params */
    // Positioning
    Pos,                // (pos, pos)
    XPos,               // pos
    YPos,               // pos
    Anchor,             // (pos, pos)
    XAnchor,            // pos
    YAnchor,            // pos
    Align,              // (float, float)
    XAlign,             // float
    YAlign,             // float
    Offset,             // (abs, abs)
    XOffset,            // abs
    YOffset,            // abs
    XYCenter,           // (pos, pos)
    XCenter,            // pos
    YCenter,            // pos
    SubPixel,           // bool
    // Rotation
    Rotate,             // float | None
    Rotate_Pad,         // bool
    TF_Anchor,          // bool
    // Zoom & Flip
    Zoom,               // float
    XZoom,              // float
    YZoom,              // float
    // Pixel Effects
    Nearest,            // bool
    Alpha,              // float
    Additive,           // float
    MatrixColor,        // None | Matrix | MatrixColor
    Blur,               // float | None
    // Polar Positioning
    Around,             // (pos, pos)
    Angle,              // float
    Radius,             // pos
    // Polar Positioning of Anchor
    AnchorAround,       // (pos, pos)
    AnchorAngle,        // (float)
    AnchorRadius,       // (pos)
    // Crop & Resize
    Crop,               // None | (pos, pos, pos, pos)
    Corner1,            // None | (pos, pos)
    Corner2,            // None | (pos, pos)
    XYSize,             // None | (pos, pos)
    XSize,              // None | pos
    YSize,              // None | pos
    Fit,                // None | str
    // Pan & Tile
    XPan,               // None | float
    YPan,               // None | float
    XTile,              // int
    YTile,              // int
    // Transitions
    Delay,              // bool
    Events,             // float
    // Other
    FPS,                // None | float
    Show_Cancels_Hide,  // bool
    // not adding deprecated TF properties...
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

struct TokScene : Tok {
    static constexpr std::string_view type_name = "Scene";
};

struct TokNone : Tok {
    static constexpr std::string_view type_name = "None";
};

struct TokMenu : Tok {
    static constexpr std::string_view type_name = "Menu";
};

struct TokChoice : Tok {
    static constexpr std::string_view type_name = "Choice";
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
    std::string name;
    static constexpr std::string_view type_name = "Identifier";
};

struct TokStrLit : Tok {
    std::string text;
    static constexpr std::string_view type_name = "String Literal";
};

struct TokIntLit : Tok {
    int value;
    static constexpr std::string_view type_name = "Integer Literal";
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
    TFPropType type;
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
    TokChoice,
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
    TokSet,
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

inline auto prop_str(const TFPropType &type) -> std::string {
    switch (type) {
        case TFPropType::Pos:
            return "pos";
        case TFPropType::XPos:
            return "xpos";
        case TFPropType::YPos:
            return "ypos";
        case TFPropType::Anchor:
            return "anchor";
        case TFPropType::XAnchor:
            return "xanchor";
        case TFPropType::YAnchor:
            return "yanchor";
        case TFPropType::Align:
            return "align";
        case TFPropType::XAlign:
            return "xalign";
        case TFPropType::YAlign:
            return "yalign";
        case TFPropType::Offset:
            return "offset";
        case TFPropType::XOffset:
            return "xoffset";
        case TFPropType::YOffset:
            return "yoffset";
        case TFPropType::XYCenter:
            return "xycenter";
        case TFPropType::XCenter:
            return "xcenter";
        case TFPropType::YCenter:
            return "ycenter";
        case TFPropType::SubPixel:
            return "subpixel";
        case TFPropType::Rotate:
            return "rotate";
        case TFPropType::Rotate_Pad:
            return "rotate_pad";
        case TFPropType::TF_Anchor:
            return "transform_anchor";
        case TFPropType::Zoom:
            return "zoom";
        case TFPropType::XZoom:
            return "xzoom";
        case TFPropType::YZoom:
            return "yzoom";
        case TFPropType::Nearest:
            return "nearest";
        case TFPropType::Alpha:
            return "alpha";
        case TFPropType::Additive:
            return "additive";
        case TFPropType::MatrixColor:
            return "matrixcolor";
        case TFPropType::Blur:
            return "blur";
        case TFPropType::Around:
            return "around";
        case TFPropType::Angle:
            return "angle";
        case TFPropType::Radius:
            return "radius";
        case TFPropType::AnchorAround:
            return "anchoraround";
        case TFPropType::AnchorAngle:
            return "anchorangle";
        case TFPropType::AnchorRadius:
            return "anchorradius";
        case TFPropType::Crop:
            return "crop";
        case TFPropType::Corner1:
            return "corner1";
        case TFPropType::Corner2:
            return "corner2";
        case TFPropType::XYSize:
            return "xysize";
        case TFPropType::XSize:
            return "xsize";
        case TFPropType::YSize:
            return "ysize";
        case TFPropType::Fit:
            return "fit";
        case TFPropType::XPan:
            return "xpan";
        case TFPropType::YPan:
            return "ypan";
        case TFPropType::XTile:
            return "xtile";
        case TFPropType::YTile:
            return "ytile";
        case TFPropType::Delay:
            return "delay";
        case TFPropType::Events:
            return "events";
        case TFPropType::FPS:
            return "fps";
        case TFPropType::Show_Cancels_Hide:
            return "show_cancels_hide";
        default:
            std::println(std::cerr, "Invalid property in transformation");
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
            [&](const TokScene &) -> std::string {
                return "scene";
            },
            [&](const TokNone &) -> std::string {
                return "None";
            },
            [&](const TokMenu &) -> std::string {
                return "menu";
            },
            [&](const TokChoice &) -> std::string {
                return "ch";
            },
            [&](const TokAt &) -> std::string {
                return "at";
            },
            [&](const TokAs &) -> std::string {
                return "as";
            },
            [&](const TokBehind &) -> std::string {
                return "behind";
            },
            [&](const TokOnlayer &) -> std::string {
                return "onlayer";
            },
            [&](const TokZOrder &) -> std::string {
                return "zorder";
            },
            [&](const TokWith &) -> std::string {
                return "with";
            },
            [&](const TokLabel &) -> std::string {
                return "label";
            },
            [&](const TokIdent &t) -> std::string {
                return t.name;
            },
            [&](const TokStrLit &t) -> std::string {
                return t.text;
            },
            [&](const TokIntLit &t) -> std::string {
                return std::format("{}", t.value);
            },
            [&](const TokFloatLit &t) -> std::string {
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
            [&](const TokPass &) -> std::string {
                return "pass";
            },
            [&](const TokCall &) -> std::string {
                return "call";
            },
            [&](const TokJump &) -> std::string {
                return "jump";
            },
            [&](const TokImage &) -> std::string {
                return "image";
            },
            [&](const TokTransform &) -> std::string {
                return "transform";
            },
            [&](const TokATLProperty &t) -> std::string {
                return prop_str((t.type));
            },
            [&](const TokATLPause &) -> std::string {
                return "pause";
            },
            [&](const TokATLWarp &) -> std::string {
                return "warp";
            },
            [&](const TokATLKnot &) -> std::string {
                return "knot";
            },
            [&](const TokATLClockwise &) -> std::string {
                return "clockwise";
            },
            [&](const TokATLCCWise &) -> std::string {
                return "counterclockwise";
            },
            [&](const TokATLCircles &) -> std::string {
                return "circles";
            },
            [&](const TokATLRepeat &) -> std::string {
                return "repeat";
            },
            [&](const TokATLBlock &) -> std::string {
                return "block";
            },
            [&](const TokATLParallel &) -> std::string {
                return "parallel";
            },
            [&](const TokATLAnimation &) -> std::string {
                return "animation";
            },
            [&](const TokATLOn &) -> std::string {
                return "on";
            },
            [&](const TokATLContains &) -> std::string {\
                return "contains";
            },
            [&](const TokATLFunction &) -> std::string {
                return "function";
            },
            [&](const TokATLTime &) -> std::string {
                return "time";
            },
            [&](const TokATLEvent &) -> std::string {
                return "event";
            },
            [&](const TokATLTransition &t) -> std::string {
                return ATL::trans_str(t.trans);
            },
            [&](const TokATLWarper &t) -> std::string {
                return ATL::warper_str(t.warper);
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
auto prop_from_str(const std::string_view &str) -> TFPropType;

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