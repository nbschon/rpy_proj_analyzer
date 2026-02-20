//
// Created by Noah on 10/9/2025.
//

#include "Token.hpp"

#include <format>

auto tok_str(const Token& token) -> std::string {
    std::string ret = tok_pos(token);
    std::visit(Overload{
        [&](const TokDollarSign &t) -> void {
            ret += "$";
        },
        [&](const TokColon &t) -> void {
            ret += ":";
        },
        [&](const TokLParen &t) -> void {
            ret += "(";
        },
        [&](const TokRParen &t) -> void {
            ret += ")";
        },
        [&](const TokLCurly &t) -> void {
            ret += "{";
        },
        [&](const TokRCurly &t) -> void {
            ret += "}";
        },
        [&](const TokLBracket &t) -> void {
            ret += "[";
        },
        [&](const TokRBracket &t) -> void {
            ret += "]";
        },
        [&](const TokComma &t) -> void {
            ret += ",";
        },
        [&](const TokShow &t) -> void {
            ret += "Show";
        },
        [&](const TokHide &t) -> void {
            ret += "Hide";
        },
        [&](const TokMenu &t) -> void {
            ret += "Menu";
        },
        [&](const TokChoice &t ) -> void {
            ret += "Choice";
        },
        [&](const TokTransition &t) -> void {
            ret += "Transition";
        },
        [&](const TokPos &t) -> void {
            ret += "Position";
        },
        [&](const TokLabel &t) -> void {
            ret += "Label";
        },
        [&](const TokScene &t) -> void {
            ret += "Scene";
        },
        [&](const TokIdent &t) -> void {
            ret += std::format("Identifier: \"{}\"", t.name);
        },
        [&](const TokStrLit &t) -> void {
            ret += std::format("Dialogue: \"{}\"", t.text);
        },
        [&](const TokNumLit &t) -> void {
            ret += std::format("Number: \"{}\"", t.value);
        },
        [&](const TokBoolLit &t) -> void {
            ret += std::format("Bool: \"{}\"", t.value ? "True" : "False");
        },
        [&](const TokOp &t) -> void {
            ret += std::format("Operator: \"{}\"", t.type);
        },
        [&](const TokDefault &t) -> void {
            ret += "Default";
        },
        [&](const TokDefine &t) -> void {
            ret += "Define";
        },
        [&](const TokSet &t) -> void {
            ret += "Set";
        },
        [&](const TokPlay &t) -> void {
            ret += "Play";
        },
        [&](const TokMusic &t) -> void {
            ret += "Music";
        },
        [&](const TokSfx &t) -> void {
            ret += "SFX";
        },
        [&](const TokIf &t) -> void {
            ret += "If";
        },
        [&](const TokElif &t) -> void {
            ret += "Elif";
        },
        [&](const TokElse &t) -> void {
            ret += "Else";
        },
        [&](const TokWhile &t) -> void {
            ret += "While";
        },
        [&](const TokReturn &t) -> void {
            ret += "Return";
        },
        [&](const TokCall &t) -> void {
            ret += "Label call";
        },
        [&](const TokJump &t) -> void {
            ret += "Jump to";
        },
        [&](const TokCharacter &) -> void {
            ret += "Character";
        },
        [&](const TokImage &) -> void {
            ret += "Image";
        },
        [&](const TokNewline &t) -> void {
            ret = "\n";
        },
        [&](const TokTab &t)-> void {
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
            [&](const TokMenu &) -> std::uint32_t {
                return green;
            },
            [&](const TokChoice &) -> std::uint32_t {
                return green;
            },
            [&](const TokTransition &) -> std::uint32_t {
                return purple;
            },
            [&](const TokPos &) -> std::uint32_t {
                return purple;
            },
            [&](const TokLabel &) -> std::uint32_t {
                return green;
            },
            [&](const TokScene &) -> std::uint32_t {
                return green;
            },
            [&](const TokIdent &) -> std::uint32_t {
                return blue;
            },
            [&](const TokStrLit &) -> std::uint32_t {
                return green;
            },
            [&](const TokNumLit &) -> std::uint32_t {
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
            [&](const TokCall &) -> std::uint32_t {
                return purple;
            },
            [&](const TokJump &) -> std::uint32_t {
                return purple;
            },
            [&](const TokNewline &) -> std::uint32_t {
                return black;
            },
            [&](const TokCharacter &) -> std::uint32_t {
                return blue;
            },
            [&](const TokImage &) -> std::uint32_t {
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