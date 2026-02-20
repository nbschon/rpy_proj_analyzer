//
// Created by Noah on 10/9/2025.
//

#include "Lexer.hpp"
#include "Node.hpp"
#include "Token.hpp"

#include <algorithm>
#include <format>
#include <list>
#include <print>
#include <ranges>
#include <sstream>
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
    tokens.emplace_back(TokNumLit{line, new_col, indent_level, std::stod(num_buff)});
    if (pt_count > 1) {
        std::println(std::cerr, "warning: incorrect number format on line {}", line);
    }
}

auto Lexer::tokenize() -> std::vector<Token> {
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
            } else if (txt_buff == "menu") {
                tokens.emplace_back(TokMenu{line, new_col, indent_level});
            } else if (txt_buff == "with") {
                tokens.emplace_back(TokTransition{line, new_col, indent_level});
            } else if (txt_buff == "at") {
                tokens.emplace_back(TokPos{line, new_col, indent_level});
            } else if (txt_buff == "label") {
                tokens.emplace_back(TokLabel{line, new_col, indent_level});
            } else if (txt_buff == "scene") {
                tokens.emplace_back(TokScene{line, new_col, indent_level});
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
            } else if (txt_buff == "call") {
                tokens.emplace_back(TokCall{line, new_col, indent_level});
            } else if (txt_buff == "jump") {
                tokens.emplace_back(TokJump{line, new_col, indent_level});
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
        } else if (*peek() == '#') {
            while (peek() && *peek() != '\n') {
                consume();
            }
        } else if (*peek() == '\"') {
            consume();
            while (peek() && *peek() != '\"') {
                txt_buff += consume();
            }
            consume();
            const unsigned new_col = col - txt_buff.length() - 2; // 2, one for each quote mark
            tokens.emplace_back(TokStrLit{line, new_col, indent_level, txt_buff});
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
        }
        else {
            consume();
        }
    }

    std::println("got {} tokens...", tokens.size());
    print_tokens(5);

    return this->tokens;
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


Lexer::Lexer(const std::filesystem::path &path) {
    this->input_file = std::ifstream(path);
    std::stringstream buff;
    buff << input_file.rdbuf();
    this->input_str = buff.str();

    if (input_str.empty()) {
        std::println(std::cerr, "Could not open file: {}", path.string());
    }
}
