//
// Created by Noah Schonhorn on 12/16/25.
//

#include "Expr.hpp"

#include "Lexer.hpp"

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
    return std::visit(Overload {
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

auto expr_slice(Lexer &lexer) -> std::expected<std::span<const Token>, std::string> {
    const auto start_idx = lexer.get_idx();

    if (!lexer.has_more()) {
        return std::unexpected("reached end of Tokens");
    }

    bool get_toks = true;
    while (get_toks && lexer.has_more()) {
        const auto& token = lexer.curr();
        std::visit(Overload {
            [&](const TokIdent&) -> void {
                ++lexer;
            },
            [&](const TokStrLit&) -> void {
                ++lexer;
            },
            [&](const TokIntLit&) -> void {
                ++lexer;
            },
            [&](const TokFloatLit&) -> void {
                ++lexer;
            },
            [&](const TokBoolLit&) -> void {
                ++lexer;
            },
            [&](const TokOp&) -> void {
                ++lexer;
            },
            [&](const TokLParen&) -> void {
                ++lexer;
            },
            [&](const TokRParen&) -> void {
                ++lexer;
            },
            [&](const TokComma&) -> void {
                ++lexer;
            },
            [&](const TokNone) -> void {
                ++lexer;
            },
            [&]<typename U>(U&&) -> void {
                get_toks = false;
            },
        }, token);
    }

    const auto count = lexer.get_idx() - start_idx;
    if (count == 0) {
        return std::unexpected(
            std::format("failed to make span of valid Tokens at {}", tok_pos(
                lexer.get_tokens().at(start_idx))));
    }

    const std::span all(lexer.get_tokens().data(), lexer.get_tokens().size());
    return all.subspan(start_idx, count);
}

auto split_inside_parens(std::span<const Token> toks, unsigned& start_idx)
-> std::expected<std::unique_ptr<ExprCall>, std::string> {
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
                    auto expr = fold_into_expr(a, e_start_idx);
                    if (expr) {
                        fn_kwargs.emplace_back(name, std::move(*expr));
                        continue;
                    }
                    return std::unexpected(std::move(expr.error()));
                }

                return std::unexpected("invalid kwarg format");
            }
        }

        if (a.empty()) {
            return std::unexpected("empty arg in expression");
        }

        auto new_arg = fold_into_expr(a);
        if (!new_arg) {
            return std::unexpected(std::move(new_arg.error()));
        }
        fn_args.emplace_back(std::move(*new_arg));
    }

    return std::make_unique<ExprCall>(std::move(fn_args), std::move(fn_kwargs));
}

/*
 * Adapted from here:
 * https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html
 */
auto fold_into_expr(std::span<const Token> toks, unsigned idx, const float min_prec)
-> std::expected<std::unique_ptr<Expr>, std::string> {
    auto peek = [&]() -> std::optional<const Token> {
        if (idx < toks.size()) {
            return toks[idx];
        }
        return std::nullopt;
    };
    auto consume = [&]() -> Token {
        return toks[idx++];
    };

    using Result = std::expected<std::unique_ptr<Expr>, std::string>;

    // TODO: graceful error reporting instead of crashing
    const auto lhs_tok = consume();
    auto lhs = std::visit(Overload {
        [&](const TokIdent& t) -> Result {
            return std::make_unique<ExprVar>(t.name);
        },
        [&](const TokStrLit& t) -> Result {
            return std::make_unique<ExprLit>(t.text);
        },
        [&](const TokIntLit& t) -> Result {
            return std::make_unique<ExprLit>(t.value);
        },
        [&](const TokFloatLit& t) -> Result {
            return std::make_unique<ExprLit>(t.value);
        },
        [&](const TokBoolLit& t) -> Result {
            return std::make_unique<ExprLit>(t.value);
        },
        [&](const TokLParen&) -> Result {
            auto expr = fold_into_expr(toks, idx, 0.0f);

            if (expr) {
                if (std::holds_alternative<TokRParen>(*peek())) {
                    return std::unexpected(std::format("expected RParen at {}", tok_pos(*peek())));
                }
                consume();
                return expr;
            }

            return std::unexpected(expr.error());
        },
        [&](const TokOp& t) -> Result {
            auto [l_prec, r_prec] = precedence(t.type);
            auto rhs = fold_into_expr(toks, idx, r_prec);
            return std::make_unique<ExprUnary>(t.type, std::move(*rhs));
        },
        [&](auto&&) -> Result {
            return std::unexpected("bad token in expression");
        },
    }, lhs_tok);

    while (true) {
        if (!peek()) {
            break;
        }

        if (std::holds_alternative<TokLParen>(*peek())) {
            if (auto call = split_inside_parens(toks, idx)) {
                (*call)->callee = std::move(*lhs);
                lhs = std::move(call);
            } else {
                return std::unexpected(std::move(call.error()));
            }
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
            auto res = fold_into_expr(toks, idx, r_prec);
            if (!res) {
                return std::unexpected(res.error());
            }
            lhs = std::make_unique<ExprUnary>(op, std::move(*res));
        } else {
            auto rhs = fold_into_expr(toks, idx, r_prec);
            if (!rhs) {
                return std::unexpected(rhs.error());
            }
            lhs = std::make_unique<ExprBinary>(std::move(*lhs), op, std::move(*rhs));
        }
    }

    return lhs;
}

auto try_get_expr(Lexer& lexer) -> std::expected<std::unique_ptr<Expr>, std::string> {
    auto slice = expr_slice(lexer);
    if (slice) {
        auto expr = fold_into_expr(*slice);
        if (expr) {
            return expr;
        }
        return std::unexpected(std::move(expr.error()));
    }
    return std::unexpected(std::move(slice.error()));
}

auto is_valid_assign(const Expr* expr) -> bool {
    if (const auto* bin_expr = dynamic_cast<const ExprBinary*>(expr)) {
        const bool lhs_is_var = dynamic_cast<ExprVar *>(bin_expr->lhs.get()) != nullptr;
        const bool op_is_assign = bin_expr->op == OpType::Assign;
        return lhs_is_var && op_is_assign;
    }
    return false;
}
