//
// Created by Noah Schonhorn on 12/16/25.
//

#ifndef RPY_PROJ_ANALYZER_EXPR_HPP
#define RPY_PROJ_ANALYZER_EXPR_HPP

#include "Token.hpp"

#include <cstdint>
#include <expected>
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <variant>
#include <vector>

class Lexer;

enum class UnaryOp : std::uint8_t {
    Not,
    Neg,
};

enum class BinaryOp : std::uint8_t {
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
};

inline auto args(const OpType &op) -> unsigned {
    switch (op) {
        using enum OpType;
        case Not:
        case Neg:
            return 1;
        default:
            return 2;
    }
}

inline auto precedence(const OpType &op) -> std::pair<float, float> {
    switch (op) {
        using enum OpType;
        case Assign:
        case PlusEq:
        case MinusEq:
        case MultEq:
        case DivEq:
            // right assoc
            return {1.0f, 1.1f};
        case Or:
            // left assoc
            return {2.1f, 2.0f};
        case And:
            // left assoc
            return {3.1f, 3.0f};
        case Not:
            // left assoc (but only one arg)
            return {4.0f, 4.1f};
        case Eq:
        case NotEq:
        case Less:
        case LessEq:
        case Greater:
        case GreaterEq:
        case In:
            // left assoc
            return {5.1f, 5.0f};
        case Plus:
        case Minus:
            // left assoc
            return {6.1f, 6.0f};
        case Mult:
        case Div:
            // left assoc
            return {7.1f, 7.0f};
        case Neg:
            // right assoc (but only one arg)
            return {8.0f, 8.1f};
        default:
            std::println(std::cerr, "unknown precedence in Expr");
            std::unreachable();
    }
}

inline auto unary_str(const UnaryOp &op) -> std::string {
    switch (op) {
        using enum UnaryOp;
        case Not:
            return "!";
        case Neg:
            return "-";
        default:
            std::println(std::cerr, "unknown unary operator in Expr");
            std::unreachable();
    }
}

inline auto binary_str(const BinaryOp &op) -> std::string {
    switch (op) {
        using enum BinaryOp;
        case Plus:
            return "+";
        case Minus:
            return "-";
        case Mult:
            return "*";
        case Div:
            return "/";
        case Assign:
            return "=";
        case PlusEq:
            return "+=";
        case MinusEq:
            return "-=";
        case MultEq:
            return "*=";
        case DivEq:
            return "/=";
        case Eq:
            return "==";
        case NotEq:
            return "!=";
        case Less:
            return "<";
        case LessEq:
            return "<=";
        case Greater:
            return ">";
        case GreaterEq:
            return ">=";
        case In:
            return "in";
        case And:
            return "and";
        case Or:
            return "or";
        default:
            std::println(std::cerr, "unknown binary operator in Expr");
            std::unreachable();
    }
}

using Literal = std::variant<int, double, bool, std::string>;

struct Expr {
    virtual ~Expr() = default;

    [[nodiscard]] virtual auto to_string() const -> std::string = 0;
};

struct ExprLit : Expr {
    explicit ExprLit(Literal value);
    Literal value;
    [[nodiscard]] auto to_string() const -> std::string override;
};

struct ExprVar : Expr {
    explicit ExprVar(std::string name);
    std::string name;
    [[nodiscard]] auto to_string() const -> std::string override;
};

struct ExprUnary : Expr {
    explicit ExprUnary(const OpType &op, std::unique_ptr<Expr> rhs);
    OpType op;
    std::unique_ptr<Expr> rhs;

    [[nodiscard]] auto to_string() const -> std::string override;
};

struct ExprBinary : Expr {
    explicit ExprBinary(std::unique_ptr<Expr> lhs, const OpType &op, std::unique_ptr<Expr> rhs);
    std::unique_ptr<Expr> lhs;
    OpType op;
    std::unique_ptr<Expr> rhs;

    [[nodiscard]] auto to_string() const -> std::string override;
};

struct ExprCall : Expr {
    explicit ExprCall(std::unique_ptr<Expr> callee,
        std::vector<std::unique_ptr<Expr>> args, std::vector<std::pair<std::string, std::unique_ptr<Expr>>> kwargs);
    explicit ExprCall(std::vector<std::unique_ptr<Expr>> args, std::vector<std::pair<std::string, std::unique_ptr<Expr>>> kwargs);
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> args;
    std::vector<std::pair<std::string, std::unique_ptr<Expr>>> kwargs;

    [[nodiscard]] auto to_string() const -> std::string override;
};

struct ExprTuple : Expr {
    explicit ExprTuple(std::vector<std::unique_ptr<Expr>> elems);
    std::vector<std::unique_ptr<Expr>> elems;

    [[nodiscard]] auto to_string() const -> std::string override;
};

[[nodiscard]] auto expr_slice(Lexer &lexer) -> std::expected<std::span<const Token>, std::string>;

[[nodiscard]] auto split_inside_parens(std::span<const Token> toks, unsigned &start_idx)
    -> std::vector<std::span<const Token>>;

[[nodiscard]] auto make_expr_call(std::span<const Token> toks, unsigned &start_idx)
-> std::expected<std::unique_ptr<ExprCall>, std::string>;

[[nodiscard]] auto fold_into_expr(std::span<const Token> toks, unsigned idx = 0, float min_prec = 0.0)
-> std::expected<std::unique_ptr<Expr>, std::string>;

[[nodiscard]] auto try_get_expr(Lexer &lexer) -> std::expected<std::unique_ptr<Expr>, std::string>;
[[nodiscard]] auto is_valid_assign(const Expr *expr) -> bool;


#endif //RPY_PROJ_ANALYZER_EXPR_HPP
