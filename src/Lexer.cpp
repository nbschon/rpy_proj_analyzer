//
// Created by Noah on 10/9/2025.
//

#include "Lexer.hpp"

#include "ATL.hpp"
#include "Node.hpp"
#include "Token.hpp"

#include <format>
#include <fstream>
#include <list>
#include <print>
#include <ranges>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>


auto Lexer::peek() -> std::optional<char> {
    if (this->offset < this->input_str.length()) {
        return input_str.at(offset);
    }

    return std::nullopt;
}

auto Lexer::consume() -> char {
    col++;
    return input_str.at(offset++);
}

auto Lexer::identifiers() const -> std::set<std::string> {
    std::set<std::string> idents;

    for (const auto &tok : this->tokens) {
        if (std::holds_alternative<TokIdent>(tok)) {
            auto ident = std::get<TokIdent>(tok);
            idents.insert(ident.name);
        }
    }

    return idents;
}

void Lexer::parse_num(const bool starts_neg) {
    std::string num_buff;
    if (starts_neg) {
        num_buff += '-';
    }
    unsigned pt_count = 0;
    while (peek().has_value() && ((std::isdigit(peek().value()) != 0) || peek().value() == '.')) {
        num_buff += consume();
        if (num_buff.back() == '.') {
            pt_count++;
        }
    }
    const unsigned new_col = col - num_buff.length();
    if (pt_count == 0) {
        tokens.emplace_back(TokIntLit{line, new_col, indent_level, std::stoi(num_buff)});
    } else {
        tokens.emplace_back(TokFloatLit{line, new_col, indent_level, std::stod(num_buff)});
    }
    if (pt_count > 1) {
        std::println(std::cerr, "warning: incorrect number format on line {}", line);
    }
}

void Lexer::remove_empty_lines() {
    std::vector<Token> cleaned;
    cleaned.reserve(tokens.size());
    std::list<Token> tok_buff;
    bool exc_tabs = true;
    for (const auto &tok : tokens) {
        tok_buff.push_back(tok);

        if (std::holds_alternative<TokNewline>(tok)) {
            if (!exc_tabs && !tok_buff.empty()) {
                cleaned.insert(cleaned.end(), tok_buff.begin(), tok_buff.end());
            }
            tok_buff.clear();
            exc_tabs = true;
        } else if (!std::holds_alternative<TokTab>(tok)) {
            exc_tabs = false;
        }
    }

    for (const auto &tok : tok_buff) {
        if (!std::holds_alternative<TokTab>(tok)) {
            exc_tabs = false;
        }
    }

    if (!tok_buff.empty() && exc_tabs) {
        cleaned.insert(cleaned.end(), tok_buff.begin(), tok_buff.end());
    }

    tokens = cleaned;
}

auto Lexer::get_str_lit() -> std::string {
    static std::unordered_set escaped = {'\'', '\\', '\"', 'n', 'r', 't', 'b', 'f'};
    consume();
    std::string txt_buff;

    while (peek()) {
        if (const auto curr_char = *peek(); curr_char == '\\') {
            consume();
            if (escaped.contains(*peek())) {
                switch (consume()) {
                    case '\'':
                        txt_buff += '\'';
                        break;
                    case '\\':
                        txt_buff += '\\';
                        break;
                    case '\"':
                        txt_buff += '\"';
                        break;
                    case 'n':
                        txt_buff += '\n';
                        break;
                    case 'r':
                        txt_buff += '\r';
                        break;
                    case 't':
                        txt_buff += '\t';
                        break;
                    case 'b':
                        txt_buff += '\b';
                        break;
                    case 'f':
                        txt_buff += '\f';
                        break;
                    default:
                        std::println(std::cerr, "invalid escape sequence at {}:{}", line, col);
                        break;
                }
            }
        } else if (curr_char == '\"') {
            break;
        } else {
            txt_buff += consume();
        }
    }
    consume();
    return txt_buff;
}

Lexer::Lexer(const std::filesystem::path &path) {
    const auto input_file = std::ifstream(path);
    std::stringstream buff;
    buff << input_file.rdbuf();
    this->input_str = buff.str();

    if (input_str.empty()) {
        std::println(std::cerr, "Could not open file: {}", path.string());
    }
}

auto Lexer::tokenize() -> std::vector<Token> {
    static const std::unordered_map<std::string, TFProp> atl_tf_props = {
        { "pos", TFProp::Pos },
        { "xpos", TFProp::XPos },
        { "ypos", TFProp::YPos },
        { "anchor", TFProp::Anchor },
        { "xanchor", TFProp::XAnchor },
        { "yanchor", TFProp::YAnchor },
        { "align", TFProp::Align },
        { "xalign", TFProp::XAlign },
        { "yalign", TFProp::YAlign },
        { "offset", TFProp::Offset },
        { "xoffset", TFProp::XOffset },
        { "yoffset", TFProp::YOffset },
        { "xycenter", TFProp::XYCenter },
        { "xcenter", TFProp::XCenter },
        { "ycenter", TFProp::YCenter },
        { "subpixel", TFProp::SubPixel },
        { "rotate", TFProp::Rotate },
        { "rotate_pad", TFProp::Rotate_Pad },
        { "transform_anchor", TFProp::TF_Anchor },
        { "zoom", TFProp::Zoom },
        { "xzoom", TFProp::XZoom },
        { "yzoom", TFProp::YZoom },
        { "nearest", TFProp::Nearest },
        { "alpha", TFProp::Alpha },
        { "additive", TFProp::Additive },
        { "matrixcolor", TFProp::MatrixColor },
        { "blur", TFProp::Blur },
        { "around", TFProp::Around },
        { "angle", TFProp::Angle },
        { "radius", TFProp::Radius },
        { "anchoraround", TFProp::AnchorAround },
        { "anchorangle", TFProp::AnchorAngle },
        { "anchorradius", TFProp::AnchorRadius },
        { "crop", TFProp::Crop },
        { "corner1", TFProp::Corner1 },
        { "corner2", TFProp::Corner2 },
        { "xysize", TFProp::XYSize },
        { "xsize", TFProp::XSize },
        { "ysize", TFProp::YSize },
        { "fit", TFProp::Fit },
        { "xpan", TFProp::XPan },
        { "ypan", TFProp::YPan },
        { "xtile", TFProp::XTile },
        { "ytile", TFProp::YTile },
        { "delay", TFProp::Delay },
        { "events", TFProp::Events },
        { "fps", TFProp::FPS },
        { "show_cancels_hide", TFProp::Show_Cancels_Hide },
    };

    static const std::unordered_map<std::string, Transition> atl_trans = {
        { "dissolve", Transition::Dissolve },
        { "fade", Transition::Fade },
        { "pixellate", Transition::Pixellate },
        { "move", Transition::Move },
        { "moveinright", Transition::MoveInRight },
        { "moveinleft", Transition::MoveInLeft },
        { "moveintop", Transition::MoveInTop },
        { "moveinbottom", Transition::MoveInBottom },
        { "moveoutright", Transition::MoveOutRight },
        { "moveoutleft", Transition::MoveOutLeft },
        { "moveouttop", Transition::MoveOutTop },
        { "moveoutbottom", Transition::MoveOutBottom },
        { "ease", Transition::Ease },
        { "easeinright", Transition::EaseInRight },
        { "easeinleft", Transition::EaseInLeft },
        { "easeintop", Transition::EaseInTop },
        { "easeinbottom", Transition::EaseInBottom },
        { "easeoutright", Transition::EaseOutRight },
        { "easeoutleft", Transition::EaseOutLeft },
        { "easeouttop", Transition::EaseOutTop },
        { "easeoutbottom", Transition::EaseOutBottom },
        { "zoomin", Transition::ZoomIn },
        { "zoomout", Transition::ZoomOut },
        { "zoominout", Transition::ZoomInOut },
        { "vpunch", Transition::VPunch },
        { "hpunch", Transition::HPunch },
        { "blinds", Transition::Blinds },
        { "squares", Transition::Squares },
        { "wipeleft", Transition::WipeLeft },
        { "wiperight", Transition::WipeRight },
        { "wipeup", Transition::WipeUp },
        { "wipedown", Transition::WipeDown },
        { "slideleft", Transition::SlideLeft },
        { "slideright", Transition::SlideRight },
        { "slideup", Transition::SlideUp },
        { "slidedown", Transition::SlideDown },
        { "slideawayleft", Transition::SlideAwayLeft },
        { "slideawayright", Transition::SlideAwayRight },
        { "slideawayup", Transition::SlideAwayUp },
        { "slideawaydown", Transition::SlideAwayDown },
        { "pushright", Transition::PushRight },
        { "pushleft", Transition::PushLeft },
        { "pushup", Transition::PushUp },
        { "pushdown", Transition::PushDown },
        { "irisin", Transition::IrisIn },
        { "irisout", Transition::IrisOut },
    };

    static const std::unordered_map<std::string, Warper> atl_warpers = {
        { "pause", Warper::Pause },
        { "linear", Warper::Linear },
        { "ease", Warper::Ease },
        { "easein", Warper::EaseIn },
        { "easeout", Warper::EaseOut },
    };

    std::string txt_buff;
    while (peek()) {
        if (std::isalpha(*peek()) != 0) {
            txt_buff += consume();
            while (peek() && ((std::isalnum(*peek()) != 0) || *peek() == '_' || *peek() == '.')) {
                txt_buff += consume();
            }
            const unsigned new_col = col - txt_buff.length();
            if (txt_buff == "show") {
                tokens.emplace_back(TokShow{line, new_col, indent_level});
            } else if (txt_buff == "hide") {
                tokens.emplace_back(TokHide{line, new_col, indent_level});
            } else if (txt_buff == "scene") {
                tokens.emplace_back(TokScene{line, new_col, indent_level});
            } else if (txt_buff == "None") {
                tokens.emplace_back(TokNone{line, new_col, indent_level});
            } else if (txt_buff == "menu") {
                tokens.emplace_back(TokMenu{line, new_col, indent_level});
            } else if (txt_buff == "as") {
                tokens.emplace_back(TokAs{line, new_col, indent_level});
            } else if (txt_buff == "at") {
                tokens.emplace_back(TokAt{line, new_col, indent_level});
            } else if (txt_buff == "behind") {
                tokens.emplace_back(TokBehind{line, new_col, indent_level});
            } else if (txt_buff == "onlayer") {
                tokens.emplace_back(TokOnlayer{line, new_col, indent_level});
            } else if (txt_buff == "zorder") {
                tokens.emplace_back(TokZOrder{line, new_col, indent_level});
            } else if (txt_buff == "with") {
                tokens.emplace_back(TokWith{line, new_col, indent_level});
            } else if (txt_buff == "label") {
                tokens.emplace_back(TokLabel{line, new_col, indent_level});
            } else if (txt_buff == "True") {
                tokens.emplace_back(TokBoolLit{line, new_col, indent_level, true});
            } else if (txt_buff == "False") {
                tokens.emplace_back(TokBoolLit{line, new_col, indent_level, false});
            } else if (txt_buff == "in") {
                tokens.emplace_back(TokOp{line, new_col, indent_level, OpType::In});
            } else if (txt_buff == "and") {
                tokens.emplace_back(TokOp{line, new_col, indent_level, OpType::And});
            } else if (txt_buff == "or") {
                tokens.emplace_back(TokOp{line, new_col, indent_level, OpType::Or});
            } else if (txt_buff == "not") {
                tokens.emplace_back(TokOp{line, new_col, indent_level, OpType::Not});
            } else if (txt_buff == "default") {
                tokens.emplace_back(TokDefault{line, new_col, indent_level});
            } else if (txt_buff == "define") {
                tokens.emplace_back(TokDefine{line, new_col, indent_level});
            } else if (txt_buff == "set") {
                tokens.emplace_back(TokSet{line, new_col, indent_level});
            } else if (txt_buff == "play") {
                tokens.emplace_back(TokPlay{line, new_col, indent_level});
            } else if (txt_buff == "music") {
                tokens.emplace_back(TokMusic{line, new_col, indent_level});
            } else if (txt_buff == "sfx") {
                tokens.emplace_back(TokSfx{line, new_col, indent_level});
            } else if (txt_buff == "if") {
                tokens.emplace_back(TokIf{line, new_col, indent_level});
            } else if (txt_buff == "elif") {
                tokens.emplace_back(TokElif{line, new_col, indent_level});
            } else if (txt_buff == "else") {
                tokens.emplace_back(TokElse{line, new_col, indent_level});
            } else if (txt_buff == "while") {
                tokens.emplace_back(TokWhile{line, new_col, indent_level});
            } else if (txt_buff == "return") {
                tokens.emplace_back(TokReturn{line, new_col, indent_level});
            } else if (txt_buff == "pass") {
                tokens.emplace_back(TokPass{line, new_col, indent_level});
            } else if (txt_buff == "call") {
                tokens.emplace_back(TokCall{line, new_col, indent_level});
            } else if (txt_buff == "jump") {
                tokens.emplace_back(TokJump{line, new_col, indent_level});
            } else if (txt_buff == "image") {
                tokens.emplace_back(TokImage{line, new_col, indent_level});
            } else if (txt_buff == "transform") {
                tokens.emplace_back(TokTransform{line, new_col, indent_level});
            } else if (txt_buff == "pause") {
                tokens.emplace_back(TokATLPause{line, new_col, indent_level});
            } else if (txt_buff == "warp") {
                tokens.emplace_back(TokATLWarp{line, new_col, indent_level});
            } else if (txt_buff == "knot") {
                tokens.emplace_back(TokATLKnot{line, new_col, indent_level});
            } else if (txt_buff == "clockwise") {
                tokens.emplace_back(TokATLClockwise{line, new_col, indent_level});
            } else if (txt_buff == "counterclockwise") {
                tokens.emplace_back(TokATLCCWise{line, new_col, indent_level});
            } else if (txt_buff == "circles") {
                tokens.emplace_back(TokATLCircles{line, new_col, indent_level});
            } else if (txt_buff == "repeat") {
                tokens.emplace_back(TokATLRepeat{line, new_col, indent_level});
            } else if (txt_buff == "block") {
                tokens.emplace_back(TokATLBlock{line, new_col, indent_level});
            } else if (txt_buff == "parallel") {
                tokens.emplace_back(TokATLParallel{line, new_col, indent_level});
            } else if (txt_buff == "animation") {
                tokens.emplace_back(TokATLAnimation{line, new_col, indent_level});
            } else if (txt_buff == "on") {
                tokens.emplace_back(TokATLOn{line, new_col, indent_level});
            } else if (txt_buff == "contains") {
                tokens.emplace_back(TokATLContains{line, new_col, indent_level});
            } else if (txt_buff == "function") {
                tokens.emplace_back(TokATLFunction{line, new_col, indent_level});
            } else if (txt_buff == "time") {
                tokens.emplace_back(TokATLTime{line, new_col, indent_level});
            } else if (txt_buff == "event") {
                tokens.emplace_back(TokATLEvent{line, new_col, indent_level});
            } else if (atl_tf_props.contains(txt_buff)) {
                const auto tf_prop = atl_tf_props.at(txt_buff);
                tokens.emplace_back(TokATLProperty{line, new_col, indent_level, tf_prop});
            } else if (atl_trans.contains(txt_buff)) {
                const auto trans = atl_trans.at(txt_buff);
                tokens.emplace_back(TokATLTransition{line, new_col, indent_level, trans});
            } else if (atl_warpers.contains(txt_buff)) {
                const auto warper = atl_warpers.at(txt_buff);
                tokens.emplace_back(TokATLWarper{line, new_col, indent_level, warper});
            } else {
                tokens.emplace_back(TokIdent{line, new_col, indent_level, txt_buff});
            }
            txt_buff.clear();
        } else if (std::isdigit(*peek()) != 0) {
            parse_num();
        } else if (*peek() == '$') {
            tokens.emplace_back(TokDollarSign{line, col, indent_level});
            consume();
        } else if (*peek() == ':') {
            tokens.emplace_back(TokColon{line, col, indent_level});
            consume();
        } else if (*peek() == '(') {
            tokens.emplace_back(TokLParen{line, col, indent_level});
            consume();
        } else if (*peek() == ')') {
            tokens.emplace_back(TokRParen{line, col, indent_level});
            consume();
        } else if (*peek() == ',') {
            tokens.emplace_back(TokComma{line, col, indent_level});
            consume();
        } else if (*peek() == '#') {
            while (peek() && *peek() != '\n') {
                consume();
            }
        } else if (*peek() == '\"') {
            const auto new_str = get_str_lit();
            const unsigned new_col = col - new_str.length() - 2; // 2, one for each quote mark
            tokens.emplace_back(TokStrLit{line, new_col, indent_level, new_str});
            txt_buff.clear();
        } else if (*peek() == '\n') {
            consume();
            tokens.emplace_back(TokNewline{line, col, indent_level});

            indent_level = 0;
            line++;
            col = 1;
        } else if (*peek() == ' ') {
            txt_buff = consume();
            while (peek() && *peek() == ' ' && txt_buff.length() < TAB_WIDTH) {
                txt_buff += consume();
            }

            if (txt_buff.length() == TAB_WIDTH) {
                tokens.emplace_back(TokTab{line, col - TAB_WIDTH, indent_level});
                indent_level++;
            }

            txt_buff.clear();
        } else if (*peek() == '=') {
            consume();
            if (peek() && *peek() == '=') {
                consume();
                tokens.emplace_back(TokOp{line, col - 2, indent_level, OpType::Eq});
            } else {
                tokens.emplace_back(TokOp{line, col - 1, indent_level, OpType::Assign});
            }
        } else if (*peek() == '!') {
            consume();
            if (peek() && *peek() == '=') {
                consume();
                tokens.emplace_back(TokOp{line, col - 2, indent_level, OpType::NotEq});
            } else {
                std::println("warning: syntax error on {}:{}", line, col);
            }
        } else if (*peek() == '<') {
            consume();
            if (peek() && *peek() == '=') {
                consume();
                tokens.emplace_back(TokOp{line, col - 2, indent_level, OpType::LessEq});
            } else {
                tokens.emplace_back(TokOp{line, col - 1, indent_level, OpType::Less});
            }
        } else if (*peek() == '>') {
            consume();
            if (peek() && *peek() == '=') {
                consume();
                tokens.emplace_back(TokOp{line, col - 2, indent_level, OpType::GreaterEq});
            } else {
                tokens.emplace_back(TokOp{line, col - 1, indent_level, OpType::Greater});
            }
        } else if (*peek() == '+') {
            consume();
            if (peek() && *peek() == '=') {
                consume();
                tokens.emplace_back(TokOp{line, col - 2, indent_level, OpType::PlusEq});
            } else {
                tokens.emplace_back(TokOp{line, col - 2, indent_level, OpType::Plus});
            }
        } else if (*peek() == '-') {
            consume();
            if (peek() && *peek() == '=') {
                consume();
                tokens.emplace_back(TokOp{line, col - 2, indent_level, OpType::MinusEq});
            } else if (peek() && (std::isdigit(*peek()) != 0)) {
                parse_num(true);
            } else {
                // std::println("warning: syntax error on {}:{}", line, col);
                tokens.emplace_back(TokOp{line, col - 1, indent_level, OpType::Neg});
            }
        } else if (*peek() == '*') {
            consume();
            if (peek() && *peek() == '=') {
                consume();
                tokens.emplace_back(TokOp{line, col - 2, indent_level, OpType::MultEq});
            } else {
                tokens.emplace_back(TokOp{line, col - 2, indent_level, OpType::Mult});
            }
        } else if (*peek() == '/') {
            consume();
            if (peek() && *peek() == '=') {
                consume();
                tokens.emplace_back(TokOp{line, col - 2, indent_level, OpType::DivEq});
            } else {
                tokens.emplace_back(TokOp{line, col - 2, indent_level, OpType::Div});
            }
        }
        else {
            consume();
        }
    }

    std::println("got {} tokens...", tokens.size());
    print_tokens(5);

    return this->tokens;
}


void Lexer::print_tokens(const unsigned n_lines) const {
    std::vector<unsigned> curr_line;
    curr_line.reserve(10);

    auto print_tok = [](const Token& tok) -> void {
        if (std::holds_alternative<TokNewline>(tok) || std::holds_alternative<TokTab>(tok)) {
            std::print("{}", tok);
        } else {
            std::print("[{}] ", tok);
        }
    };

    if (n_lines > 0) {
        std::println("first {} lines:", n_lines);
        bool is_blank = true;
        unsigned n_newlines = 0;
        unsigned toks = 0;

        while (n_newlines < n_lines && toks < tokens.size()) {
            curr_line.push_back(toks);
            if (const auto &tok = tokens.at(toks); std::holds_alternative<TokNewline>(tok)) {
                if (!is_blank) {
                    n_newlines++;
                    for (const auto &t : curr_line) {
                        print_tok(tokens.at(t));
                    }
                }
                curr_line.clear();
                is_blank = true;
            } else if (!std::holds_alternative<TokTab>(tok)) {
                is_blank = false;
            }
            toks++;
        }

        std::println("--- {} lines / {} tokens omitted. ---", line - n_newlines, tokens.size() - toks);
    } else {
        for (Token const &tok : tokens) {
            print_tok(tok);
        }
    }
}
