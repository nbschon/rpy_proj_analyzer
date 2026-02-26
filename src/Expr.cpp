//
// Created by Noah Schonhorn on 12/16/25.
//

#include "Expr.hpp"

#include <cassert>
#include <format>
#include <optional>
#include <string>

ExprLit::ExprLit(Literal value) 
    : value(std::move(value)) {
}

ExprVar::ExprVar(std::string name) 
    : name(std::move(name)) {
}

ExprUnary::ExprUnary(const OpType& op, std::unique_ptr<Expr> rhs) 
    : rhs(std::move(rhs)), op(op) {
}

ExprBinary::ExprBinary(std::unique_ptr<Expr> lhs, const OpType& op, std::unique_ptr<Expr> rhs)
    : lhs(std::move(lhs)), rhs(std::move(rhs)), op(op) {
}

ExprCall::ExprCall(std::unique_ptr<Expr> callee, 
            std::vector<std::unique_ptr<Expr>> args,
            std::vector<std::pair<std::string, std::unique_ptr<Expr>>> kwargs) 
    : callee(std::move(callee)), args(std::move(args)), kwargs(std::move(kwargs)) {
}

auto ExprLit::to_string() const -> std::string {
    return std::visit(
        Overload{
            [&](const double& val) -> std::string {
                return std::format("[Numeric Literal: {}]", val);
            },
            [&](const bool& val) -> std::string {
                return std::format("[Boolean Literal: {}]", val);
            },
            [&](const std::string& val) -> std::string {
                return std::format("[String Literal: {}]", val);
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

auto ExprCall::to_string() const -> std::string {
    auto callee_str = callee != nullptr ? callee->to_string() : "Null";
    std::string arg_str = "Args: ";
    for (const auto &arg : this->args) {
        if (arg != nullptr) {
            arg_str += std::format("({})", arg->to_string());
        }
    }
    std::string kwarg_str = "KWArgs: ";
    for (const auto &kwarg : this->kwargs) {
        const auto &[name, arg] = kwarg;
        if (arg != nullptr) {
            kwarg_str += std::format("({}: {})", name, arg->to_string());
        }
    }

    if (!args.empty() && !kwargs.empty()) {
        // no args, no kwargs
        return std::format("[Callee: {}]", callee_str);
    }
    if (!args.empty() && kwargs.empty()) {
        // has args, no kwargs
        return std::format("[Callee: {}, {}]", callee_str, arg_str);
    }
    if (args.empty() && !kwargs.empty()) {
        // no args, has kwargs
        return std::format("[Callee: {}, {}]", callee_str, kwarg_str);
    }

    // has args & kwargs
    return std::format("[Callee: {}, {}, {}]", callee_str, arg_str, kwarg_str);
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
            [&](const TokLParen& t) -> std::unique_ptr<Expr> {
                auto expr = fold_into_expr(toks, idx, 0.0f);
                assert(std::holds_alternative<TokRParen>(*peek()));
                consume();
                return std::move(expr);
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
        if (!peek()) {
            break;
        }

        if (std::holds_alternative<TokRParen>(*peek())) {
            break;
        }
        if (!std::holds_alternative<TokOp>(*peek())) {
            break;
        }

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
    if (const auto* bin_expr = dynamic_cast<const ExprBinary*>(expr)) {
        const bool lhs_is_var = dynamic_cast<ExprVar *>(bin_expr->lhs.get()) != nullptr;
        const bool op_is_assign = bin_expr->op == OpType::Assign;
        return lhs_is_var && op_is_assign;
    }
    return false;
}
