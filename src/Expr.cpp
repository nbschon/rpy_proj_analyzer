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

ExprCall::ExprCall(std::vector<std::unique_ptr<Expr>> args,
            std::vector<std::pair<std::string, std::unique_ptr<Expr>>> kwargs)
    : callee(nullptr), args(std::move(args)), kwargs(std::move(kwargs)) {
}

auto ExprLit::to_string() const -> std::string {
    return std::visit(
        Overload{
            [&](const int& val) -> std::string {
                return std::format("[Integer Literal: {}]", val);
            },
            [&](const double& val) -> std::string {
                return std::format("[Float Literal: {}]", val);
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

auto expr_slice(const std::vector<Token> &tokens, unsigned &idx) -> std::expected<std::span<const Token>, std::string> {
    std::string error_msg;
    const std::size_t start_idx = idx;

    bool get_toks = true;
    while (get_toks) {
        const auto& token = tokens.at(idx);
        std::visit(
            Overload {
                [&](const TokIdent&) -> void {
                    idx++;
                },
                [&](const TokStrLit&) -> void {
                    idx++;
                },
                [&](const TokIntLit&) -> void {
                    idx++;
                },
                [&](const TokFloatLit&) -> void {
                    idx++;
                },
                [&](const TokBoolLit&) -> void {
                    idx++;
                },
                [&](const TokOp&) -> void {
                    idx++;
                },
                [&](const TokLParen&) -> void {
                    idx++;
                },
                [&](const TokRParen&) -> void {
                    idx++;
                },
                [&](const TokComma&) -> void {
                    idx++;
                },
                [&](const TokNone) -> void {
                    idx++;
                },
                [&]<typename U>(U&& other) -> void {
                    get_toks = false;
                    // using V = std::decay_t<U>;
                    // static_assert(std::is_base_of_v<Tok, V>, "expected derived from base Tok");
                    // const auto base_tok = static_cast<const Tok &>(other);
                    // error_msg = std::format("expected viable Expr token, got {} at {}", tok_name<V>(),
                    //                         tok_pos(base_tok));
                },
            }, token);
    }

    if (!error_msg.empty()) {
        return std::unexpected(error_msg);
    }

    if (start_idx == idx) {
        return std::unexpected(
            std::format("failed to make span of valid Tokens at {}", tok_pos(tokens.at(idx))));
    }

    const std::span all(tokens.data(), tokens.size());
    return all.subspan(start_idx, idx - start_idx);
}

auto split_inside_parens(std::span<const Token> toks, unsigned& start_idx) -> std::unique_ptr<ExprCall> {
    // if (std::holds_alternative<TokLParen>(toks.front()) && std::holds_alternative<TokRParen>(toks.back())) {
    //     std::println("good args!!!");
    // }
    auto idx = start_idx;
    int n_l = 0;
    while (idx < toks.size()) {
        if (std::holds_alternative<TokLParen>(toks[idx])) {
            n_l++;
        } else if (std::holds_alternative<TokRParen>(toks[idx])) {
            n_l--;
        }

        idx++;

        if (n_l == 0) {
            break;
        }
    }

    // add and subtract one to strip surrounding parenthesis
    const auto inside_parens = toks.subspan(start_idx + 1, idx - start_idx - 1);
    // increment start_idx so we don't repeat tokens once this function is over
    start_idx += idx - 1;

    n_l = 0;
    int left_idx = 0;
    std::vector<std::span<const Token>> arg_spans;
    for (int i = 0; i < inside_parens.size(); ++i) {
        const auto &curr = inside_parens[i];
        if (std::holds_alternative<TokLParen>(curr)) {
            n_l++;
        } else if (std::holds_alternative<TokRParen>(curr)) {
            n_l--;
        }

        if (std::holds_alternative<TokComma>(curr) && n_l == 0) {
            arg_spans.emplace_back(inside_parens.subspan(left_idx, i - left_idx));
            left_idx = i + 1;
        }
    }

    if (left_idx != inside_parens.size() - 1) {
        arg_spans.emplace_back(inside_parens.subspan(left_idx, inside_parens.size() - 1 - left_idx));
    }

    std::vector<std::unique_ptr<Expr>> fn_args;
    std::vector<std::pair<std::string, std::unique_ptr<Expr>>> fn_kwargs;

    for (const auto &a : arg_spans) {
        if (a.size() > 2) {
            if (std::holds_alternative<TokIdent>(a[0]) && std::holds_alternative<TokOp>(a[1])) {
                if (std::get<TokOp>(a[1]).type == OpType::Assign) {
                    unsigned e_start_idx = 2;
                    auto name = std::get<TokIdent>(a[0]).name;
                    fn_kwargs.emplace_back(name, fold_into_expr(a, e_start_idx));
                    continue;
                }

                std::println(std::cerr, "invalid kwarg format");
            }
        }

        if (a.empty()) {
            std::println(std::cerr, "empty arg! bad!");
        } else {
            fn_args.emplace_back(fold_into_expr(a));
        }
    }

    return std::make_unique<ExprCall>(std::move(fn_args), std::move(fn_kwargs));
}

/*
 * Adapted from here:
 * https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html
 */
auto fold_into_expr(std::span<const Token> toks, unsigned idx, const float min_prec) -> std::unique_ptr<Expr> {
    auto peek = [&]() -> std::optional<const Token> {
        if (idx < toks.size()) {
            return toks[idx];
        }
        return std::nullopt;
    };
    auto consume = [&]() -> Token {
        return toks[idx++];
    };

    // TODO: graceful error reporting instead of crashing
    const auto lhs_tok = consume();
    auto lhs = std::visit(
        Overload{
            [&](const TokIdent& t) -> std::unique_ptr<Expr> {
                return std::make_unique<ExprVar>(t.name);
            },
            [&](const TokStrLit& t) -> std::unique_ptr<Expr> {
                return std::make_unique<ExprLit>(t.text);
            },
            [&](const TokIntLit& t) -> std::unique_ptr<Expr> {
                return std::make_unique<ExprLit>(t.value);
            },
            [&](const TokFloatLit& t) -> std::unique_ptr<Expr> {
                return std::make_unique<ExprLit>(t.value);
            },
            [&](const TokBoolLit& t) -> std::unique_ptr<Expr> {
                return std::make_unique<ExprLit>(t.value);
            },
            [&](const TokLParen&) -> std::unique_ptr<Expr> {
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
                std::println(std::cerr, "bad token in expression");
                std::unreachable();
            },
        }, lhs_tok);

    while (true) {
        if (!peek()) {
            break;
        }

        if (std::holds_alternative<TokLParen>(*peek())) {
            auto call = split_inside_parens(toks, idx);
            call->callee = std::move(lhs);
            lhs = std::move(call);
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
