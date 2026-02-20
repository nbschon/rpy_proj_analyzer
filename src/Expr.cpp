//
// Created by Noah Schonhorn on 12/16/25.
//

#include "Expr.hpp"

#include <format>
#include <optional>
#include <queue>
#include <unordered_map>

ExprLit::ExprLit(Literal value) : value(std::move(value)) {
}

ExprVar::ExprVar(std::string name) : name(std::move(name)) {
}

ExprUnary::ExprUnary(const OpType& op, std::unique_ptr<Expr> rhs) : rhs(std::move(rhs)), op(op) {
}

ExprBinary::ExprBinary(std::unique_ptr<Expr> lhs, const OpType& op, std::unique_ptr<Expr> rhs)
    : lhs(std::move(lhs)), rhs(std::move(rhs)), op(op) {
}

auto ExprLit::to_string() const -> std::string {
    return std::visit(
        Overload{
            [&](const double& d) -> std::string {
                return std::format("[Numeric Literal: {}]", d);
            },
            [&](const bool& b) -> std::string {
                return std::format("[Boolean Literal: {}]", b);
            },
            [&](const std::string& s) -> std::string {
                return std::format("[String Literal: {}]", s);
            }
        }, value);
}

auto ExprVar::to_string() const -> std::string {
    return std::format("[Variable: {}]", name);
}

auto ExprUnary::to_string() const -> std::string {
    auto rhs_str = rhs != nullptr ? rhs->to_string() : "Null";
    return std::format("[Unary Op: {}, Arg: {}]", rhs_str, op);
}

auto ExprBinary::to_string() const -> std::string {
    auto lhs_str = lhs != nullptr ? lhs->to_string() : "Null";
    auto rhs_str = rhs != nullptr ? rhs->to_string() : "Null";
    return std::format("[Binary Arg 1: {}, Op: {}, Arg 2: {}]", lhs_str, op, rhs_str);
}

/*
 * Adapted from here:
 * https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html
 */
auto fold_into_expr(std::span<const Token> toks, unsigned& idx, const float min_prec) -> std::unique_ptr<Expr> {
    auto peek = [&]() -> std::optional<const Token> {
        if (idx < toks.size()) {
            return toks[idx];
        }
        return std::nullopt;
    };
    auto consume = [&]() -> Token {
        return toks[idx++];
    };

    const auto lhs_tok = consume();
    auto lhs = std::visit(
        Overload{
            [&](const TokIdent& t) -> std::unique_ptr<Expr> {
                return std::make_unique<ExprVar>(t.name);
            },
            [&](const TokStrLit& t) -> std::unique_ptr<Expr> {
                return std::make_unique<ExprLit>(t.text);
            },
            [&](const TokNumLit& t) -> std::unique_ptr<Expr> {
                return std::make_unique<ExprLit>(t.value);
            },
            [&](const TokBoolLit& t) -> std::unique_ptr<Expr> {
                return std::make_unique<ExprLit>(t.value);
            },
            [&](const TokOp& t) -> std::unique_ptr<Expr> {
                auto [l_prec, r_prec] = precedence(t.type);
                auto rhs = fold_into_expr(toks, idx, r_prec);
                return std::make_unique<ExprUnary>(t.type, std::move(rhs));
            },
            [&](auto&&) -> std::unique_ptr<Expr> {
                std::println(std::cerr, "bad token");
                std::unreachable();
            },
        }, lhs_tok);

    while (true) {
        if (!peek()) break;

        OpType op = std::get<TokOp>(*peek()).type;

        auto [l_prec, r_prec] = precedence(op);
        if (l_prec < min_prec) {
            break;
        }

        consume();

        if (const auto n_args = args(op); n_args == 1) {
            lhs = std::make_unique<ExprUnary>(op, fold_into_expr(toks, idx, r_prec));
        } else {
            lhs = std::make_unique<ExprBinary>(std::move(lhs), op, fold_into_expr(toks, idx, r_prec));
        }
    }

    return lhs;
}

auto is_valid_assign(const Expr* expr) -> bool {
    if (const auto bin_expr = dynamic_cast<const ExprBinary*>(expr)) {
        const bool lhs_is_var = dynamic_cast<ExprVar *>(bin_expr->lhs.get()) != nullptr;
        const bool op_is_assign = bin_expr->op == OpType::Assign;
        return lhs_is_var && op_is_assign;
    }
    return false;
}
