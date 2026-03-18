//
// Created by Noah on 10/9/2025.
//

#include "Token.hpp"

#include <format>

auto tok_str(const Token& token) -> std::string {
    std::string ret = tok_pos(token);
    std::visit(Overload{
        [&](const TokDollarSign &) -> void {
            ret += "$";
        },
        [&](const TokColon &) -> void {
            ret += ":";
        },
        [&](const TokLParen &) -> void {
            ret += "(";
        },
        [&](const TokRParen &) -> void {
            ret += ")";
        },
        [&](const TokLCurly &) -> void {
            ret += "{";
        },
        [&](const TokRCurly &) -> void {
            ret += "}";
        },
        [&](const TokLBracket &) -> void {
            ret += "[";
        },
        [&](const TokRBracket &) -> void {
            ret += "]";
        },
        [&](const TokComma &) -> void {
            ret += ",";
        },
        [&](const TokShow &) -> void {
            ret += "Show";
        },
        [&](const TokHide &) -> void {
            ret += "Hide";
        },
        [&](const TokMenu &) -> void {
            ret += "Menu";
        },
        [&](const TokScene &) -> void {
            ret += "Scene";
        },
        [&](const TokNone &) -> void {
            ret += "None";
        },
        [&](const TokChoice &) -> void {
            ret += "Choice";
        },
        [&](const TokWith &) -> void {
            ret += "Transition";
        },
        [&](const TokAs &) -> void {
            ret += "As";
        },
        [&](const TokAt &) -> void {
            ret += "At";
        },
        [&](const TokBehind &) -> void {
            ret += "Behind";
        },
        [&](const TokOnlayer &) -> void {
            ret += "On Layer";
        },
        [&](const TokZOrder &) -> void {
            ret += "Z Order";
        },
        [&](const TokLabel &) -> void {
            ret += "Label";
        },
        [&](const TokIdent &t) -> void {
            ret += std::format("Identifier: {}", t.name);
        },
        [&](const TokStrLit &t) -> void {
            ret += std::format("String Lit.: \"{}\"", t.text);
        },
        [&](const TokIntLit &t) -> void {
            ret += std::format("Integer Lit.: {}", t.value);
        },
        [&](const TokFloatLit &t) -> void {
            ret += std::format("Float Lit.: {}", t.value);
        },
        [&](const TokBoolLit &t) -> void {
            ret += std::format("Bool Lit.: {}", t.value ? "True" : "False");
        },
        [&](const TokOp &t) -> void {
            ret += std::format("Operator: {}", t.type);
        },
        [&](const TokDefault &) -> void {
            ret += "Default";
        },
        [&](const TokDefine &) -> void {
            ret += "Define";
        },
        [&](const TokSet &) -> void {
            ret += "Set";
        },
        [&](const TokPlay &) -> void {
            ret += "Play";
        },
        [&](const TokMusic &) -> void {
            ret += "Music";
        },
        [&](const TokSfx &) -> void {
            ret += "SFX";
        },
        [&](const TokIf &) -> void {
            ret += "If";
        },
        [&](const TokElif &) -> void {
            ret += "Elif";
        },
        [&](const TokElse &) -> void {
            ret += "Else";
        },
        [&](const TokWhile &) -> void {
            ret += "While";
        },
        [&](const TokReturn &) -> void {
            ret += "Return";
        },
        [&](const TokPass &) -> void {
            ret += "Pass";
        },
        [&](const TokCall &) -> void {
            ret += "Label call";
        },
        [&](const TokJump &) -> void {
            ret += "Jump to";
        },
        [&](const TokImage &) -> void {
            ret += "Image";
        },
        [&](const TokTransform &) -> void {
            ret += "Transform";
        },
        [&](const TokATLProperty &) -> void {
            ret += "ATL Property";
        },
        [&](const TokATLPause &) -> void {
            ret += "ATL Pause";
        },
        [&](const TokATLWarp &) -> void {
            ret += "ATL Warp";
        },
        [&](const TokATLKnot &) -> void {
            ret += "ATL Knot";
        },
        [&](const TokATLClockwise &) -> void {
            ret += "ATL Clockwise";
        },
        [&](const TokATLCCWise &) -> void {
            ret += "ATL Counterclockwise";
        },
        [&](const TokATLCircles &) -> void {
            ret += "ATL Circles";
        },
        [&](const TokATLRepeat &) -> void {
            ret += "ATL Repeat";
        },
        [&](const TokATLBlock &) -> void {
            ret += "ATL Block";
        },
        [&](const TokATLParallel &) -> void {
            ret += "ATL Parallel";
        },
        [&](const TokATLAnimation &) -> void {
            ret += "ATL Animation";
        },
        [&](const TokATLOn &) -> void {
            ret += "ATL On";
        },
        [&](const TokATLContains &) -> void {
            ret += "ATL Contains";
        },
        [&](const TokATLFunction &) -> void {
            ret += "ATL Function";
        },
        [&](const TokATLTime &) -> void {
            ret += "ATL Time";
        },
        [&](const TokATLEvent &) -> void {
            ret += "ATL Event";
        },
        [&](const TokNewline &) -> void {
            ret = "\n";
        },
        [&](const TokTab &)-> void {
            ret = "\t";
        },
    }, token);

    return ret;
}

auto tok_pos(const Token &token) -> std::string {
    return std::visit(
        [](auto &t) -> std::string {
            return std::format("({}:{}) ", t.line, t.col);
        }, token);
}

auto tok_pos(const Tok &token) -> std::string {
    return std::format("({}:{}) ", token.line, token.col);
}

/*
 * Color codes borrowed from here:
 * https://htmlcolorcodes.com/colors/
 */
auto tok_color(const Token &token) -> std::uint32_t {
    constexpr std::uint32_t black = 0x343434;
    constexpr std::uint32_t orange = 0xFFAC1C;
    constexpr std::uint32_t blue = 0x0096FF;
    constexpr std::uint32_t purple = 0xC678DD;
    constexpr std::uint32_t green = 0x98C379;
    return std::visit(
        Overload{
            [&](const TokDollarSign &) -> std::uint32_t {
                return orange;
            },
            [&](const TokColon &) -> std::uint32_t {
                return black;
            },
            [&](const TokLParen &) -> std::uint32_t {
                return black;
            },
            [&](const TokRParen &) -> std::uint32_t {
                return black;
            },
            [&](const TokLCurly &) -> std::uint32_t {
                return black;
            },
            [&](const TokRCurly &) -> std::uint32_t {
                return black;
            },
            [&](const TokLBracket &) -> std::uint32_t {
                return black;
            },
            [&](const TokRBracket &) -> std::uint32_t {
                return black;
            },
            [&](const TokComma &) -> std::uint32_t {
                return black;
            },
            [&](const TokShow &) -> std::uint32_t {
                return green;
            },
            [&](const TokHide &) -> std::uint32_t {
                return green;
            },
            [&](const TokScene &) -> std::uint32_t {
                return green;
            },
            [&](const TokNone &) -> std::uint32_t {
                return green;
            },
            [&](const TokWith &) -> std::uint32_t {
                return blue;
            },
            [&](const TokMenu &) -> std::uint32_t {
                return green;
            },
            [&](const TokChoice &) -> std::uint32_t {
                return green;
            },
            [&](const TokAs &) -> std::uint32_t {
                return purple;
            },
            [&](const TokAt &) -> std::uint32_t {
                return purple;
            },
            [&](const TokBehind &) -> std::uint32_t {
                return purple;
            },
            [&](const TokOnlayer &) -> std::uint32_t {
                return purple;
            },
            [&](const TokZOrder &) -> std::uint32_t {
                return purple;
            },
            [&](const TokLabel &) -> std::uint32_t {
                return green;
            },
            [&](const TokIdent &) -> std::uint32_t {
                return blue;
            },
            [&](const TokStrLit &) -> std::uint32_t {
                return green;
            },
            [&](const TokIntLit &) -> std::uint32_t {
                return green;
            },
            [&](const TokFloatLit &) -> std::uint32_t {
                return green;
            },
            [&](const TokBoolLit &) -> std::uint32_t {
                return green;
            },
            [&](const TokOp &) -> std::uint32_t {
                return black;
            },
            [&](const TokDefault &)  -> std::uint32_t {
                return black;
            },
            [&](const TokDefine &)  -> std::uint32_t {
                return black;
            },
            [&](const TokSet &) -> std::uint32_t {
                return green;
            },
            [&](const TokPlay &) -> std::uint32_t {
                return purple;
            },
            [&](const TokMusic &) -> std::uint32_t {
                return purple;
            },
            [&](const TokSfx &) -> std::uint32_t {
                return purple;
            },
            [&](const TokIf &) -> std::uint32_t {
                return purple;
            },
            [&](const TokElif &) -> std::uint32_t {
                return purple;
            },
            [&](const TokElse &) -> std::uint32_t {
                return purple;
            },
            [&](const TokWhile &) -> std::uint32_t {
                return purple;
            },
            [&](const TokReturn &) -> std::uint32_t {
                return purple;
            },
            [&](const TokPass &) -> std::uint32_t {
                return purple;
            },
            [&](const TokCall &) -> std::uint32_t {
                return purple;
            },
            [&](const TokJump &) -> std::uint32_t {
                return purple;
            },
            [&](const TokNewline &) -> std::uint32_t {
                return black;
            },
            // [&](const TokCharacter &) -> std::uint32_t {
            //     return blue;
            // },
            [&](const TokImage &) -> std::uint32_t {
                return blue;
            },
            [&](const TokTransform &) -> std::uint32_t {
                return green;
            },
            [&](const TokATLProperty &) -> std::uint32_t {
                return green;
            },
            [&](const TokATLPause &) -> std::uint32_t {
                return green;
            },
            [&](const TokATLWarp &) -> std::uint32_t {
                return green;
            },
            [&](const TokATLKnot &) -> std::uint32_t {
                return green;
            },
            [&](const TokATLClockwise &) -> std::uint32_t {
                return green;
            },
            [&](const TokATLCCWise &) -> std::uint32_t {
                return green;
            },
            [&](const TokATLCircles &) -> std::uint32_t {
                return green;
            },
            [&](const TokATLRepeat &) -> std::uint32_t {
                return green;
            },
            [&](const TokATLBlock &) -> std::uint32_t {
                return green;
            },
            [&](const TokATLParallel &) -> std::uint32_t {
                return green;
            },
            [&](const TokATLAnimation &) -> std::uint32_t {
                return green;
            },
            [&](const TokATLOn &) -> std::uint32_t {
                return green;
            },
            [&](const TokATLContains &) -> std::uint32_t {
                return green;
            },
            [&](const TokATLFunction &) -> std::uint32_t {
                return green;
            },
            [&](const TokATLTime &) -> std::uint32_t {
                return green;
            },
            [&](const TokATLEvent &) -> std::uint32_t {
                return green;
            },
            [&](const TokTab&) -> std::uint32_t {
                return black;
            },
    }, token);
}

auto prop_from_str(const std::string_view& str) -> TFPropType {
    if (str == "pos") {
        return TFPropType::Pos;
    }
    if (str == "xpos") {
        return TFPropType::XPos;
    }
    if (str == "ypos") {
        return TFPropType::YPos;
    }
    if (str == "anchor") {
        return TFPropType::Anchor;
    }
    if (str == "xanchor") {
        return TFPropType::XAnchor;
    }
    if (str == "yanchor") {
        return TFPropType::YAnchor;
    }
    if (str == "align") {
        return TFPropType::Align;
    }
    if (str == "xalign") {
        return TFPropType::XAlign;
    }
    if (str == "yalign") {
        return TFPropType::YAlign;
    }
    if (str == "offset") {
        return TFPropType::Offset;
    }
    if (str == "xoffset") {
        return TFPropType::XOffset;
    }
    if (str == "yoffset") {
        return TFPropType::YOffset;
    }
    if (str == "xycenter") {
        return TFPropType::XYCenter;
    }
    if (str == "xcenter") {
        return TFPropType::XCenter;
    }
    if (str == "ycenter") {
        return TFPropType::YCenter;
    }
    if (str == "subpixel") {
        return TFPropType::SubPixel;
    }
    if (str == "rotate") {
        return TFPropType::Rotate;
    }
    if (str == "rotate_pad") {
        return TFPropType::Rotate_Pad;
    }
    if (str == "transform_anchor") {
        return TFPropType::TF_Anchor;
    }
    if (str == "zoom") {
        return TFPropType::Zoom;
    }
    if (str == "xzoom") {
        return TFPropType::XZoom;
    }
    if (str == "yzoom") {
        return TFPropType::YZoom;
    }
    if (str == "nearest") {
        return TFPropType::Nearest;
    }
    if (str == "alpha") {
        return TFPropType::Alpha;
    }
    if (str == "additive") {
        return TFPropType::Additive;
    }
    if (str == "matrixcolor") {
        return TFPropType::MatrixColor;
    }
    if (str == "blur") {
        return TFPropType::Blur;
    }
    if (str == "around") {
        return TFPropType::Around;
    }
    if (str == "angle") {
        return TFPropType::Angle;
    }
    if (str == "radius") {
        return TFPropType::Radius;
    }
    if (str == "anchoraround") {
        return TFPropType::AnchorAround;
    }
    if (str == "anchorangle") {
        return TFPropType::AnchorAngle;
    }
    if (str == "anchorradius") {
        return TFPropType::AnchorRadius;
    }
    if (str == "crop") {
        return TFPropType::Crop;
    }
    if (str == "corner1") {
        return TFPropType::Corner1;
    }
    if (str == "corner2") {
        return TFPropType::Corner2;
    }
    if (str == "xysize") {
        return TFPropType::XYSize;
    }
    if (str == "xsize") {
        return TFPropType::XSize;
    }
    if (str == "ysize") {
        return TFPropType::YSize;
    }
    if (str == "fit") {
        return TFPropType::Fit;
    }
    if (str == "xpan") {
        return TFPropType::XPan;
    }
    if (str == "ypan") {
        return TFPropType::YPan;
    }
    if (str == "xtile") {
        return TFPropType::XTile;
    }
    if (str == "ytile") {
        return TFPropType::YTile;
    }
    if (str == "delay") {
        return TFPropType::Delay;
    }
    if (str == "events") {
        return TFPropType::Events;
    }
    if (str == "fps") {
        return TFPropType::FPS;
    }
    if (str == "show_cancels_hide") {
        return TFPropType::Show_Cancels_Hide;
    }
    std::println(std::cerr, "unknown property type");
    std::unreachable();
}

auto operator<< (std::ostream& stream, const Token& token) -> std::ostream& {
    stream << tok_str(token);
    return stream;
}
