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
        [&](const TokCall &) -> void {
            ret += "Label call";
        },
        [&](const TokJump &) -> void {
            ret += "Jump to";
        },
        // [&](const TokCharacter &) -> void {
        //     ret += "Character";
        // },
        [&](const TokImage &) -> void {
            ret += "Image";
        },
        [&](const TokNewline &) -> void {
            ret = "\n";
        },
        [&](const TokTab &)-> void {
            ret = "\t";
        },
        // [&](const TokComma &t) -> void {
        //     ret = ",";
        // },
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
            [&](const TokMenu &) -> std::uint32_t {
                return green;
            },
            [&](const TokChoice &) -> std::uint32_t {
                return green;
            },
            [&](const TokWith &) -> std::uint32_t {
                return purple;
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
            [&](const TokTab&) -> std::uint32_t {
                return black;
            },
    }, token);
}

auto operator<< (std::ostream& stream, const Token& token) -> std::ostream& {
    stream << tok_str(token);
    return stream;
}
