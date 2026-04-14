//
// Created by Noah on 10/9/2025.
//

#include "Token.hpp"

#include "ATL.hpp"

#include <format>

auto raw_str(const Token &tok) -> std::string {
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
                return ATL::prop_str(t.type);
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
                return "atl transition";
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

auto tok_str(const Token& token) -> std::string {
    std::string ret = tok_pos(token);
    std::visit(
        Overload{
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
           [&](const TokATLProperty &t) -> void {
               ret += std::format("ATL Property: {}", ATL::prop_str(t.type));
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
           [&](const TokATLTransition &t) -> void {
               ret += std::format("ATL Transition");
           },
           [&](const TokATLWarper &t) -> void {
               ret += std::format("ATL Warper: {}", ATL::warper_str(t.warper));
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

auto tok_indent(const Token& token) -> unsigned {
    return std::visit(
        [](auto &t) -> unsigned {
            return t.indent;
        }, token);
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
            [&](const TokATLTransition &) -> std::uint32_t {
                return blue;
            },
            [&](const TokATLWarper &) -> std::uint32_t {
                return blue;
            },
            [&](const TokTab&) -> std::uint32_t {
                return black;
            },
        }, token);
}

auto operator<< (std::ostream& stream, const Token& token) -> std::ostream& {
    stream << tok_str(token);
    return stream;
}
