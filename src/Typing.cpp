//
// Created by Noah Schonhorn on 6/8/26.
//

#include "Typing.hpp"

#include <ranges>

auto Typing::lit_type(const ExprLit* lit) -> Type {
    const auto type = std::visit(Overload {
        [&](const int &) -> Type {
            return Type::Int;
        },
        [&](const double &) -> Type {
            return Type::Float;
        },
        [&](const bool &) -> Type {
            return Type::Boolean;
        },
        [&](const std::string &) -> Type {
            return Type::String;
        },
        [&]<typename U>(U&& other) -> Type {
            // FIXME: make better
            return Type::None;
        }
    }, lit->value);
    return type;
}

auto Typing::deduce_type(const std::unique_ptr<Expr>& expr) -> std::optional<Type> {
    if (const auto* lit = dynamic_cast<ExprLit*>(expr.get())) {
        return lit_type(lit);
    }
    if (const auto* var = dynamic_cast<ExprVar*>(expr.get())) {
        // TODO: add actual typing
        return std::nullopt;
    }
    if (const auto* unary = dynamic_cast<ExprUnary*>(expr.get())) {
        if (const auto type = deduce_type(unary->rhs)) {
            return type;
        }
        return std::nullopt;
    }
    if (const auto *binary = dynamic_cast<ExprBinary*>(expr.get())) {
        const auto rhs = deduce_type(binary->rhs);
        if (dynamic_cast<ExprVar*>(binary->lhs.get()) != nullptr) {
            return rhs;
        }
        if (const auto lhs = deduce_type(binary->lhs); lhs && rhs) {
            using enum Type;
            if (*lhs == Float || *rhs == Float) {
                return Float;
            }
            if (*lhs == Int || *rhs == Int) {
                return Int;
            }
            if (*lhs == Boolean || *rhs == Boolean) {
                return Boolean;
            }
        }
    }
    if (const auto *call = dynamic_cast<ExprCall*>(expr.get())) {
        std::println("fn call");
        std::vector<Type> types;
        types.reserve(call->args.size() + call->kwargs.size());
        for (const auto &arg : call->args) {
            if (const auto type = deduce_type(arg)) {
                types.push_back(*type);
            }
        }
        for (const auto& val : call->kwargs | std::views::values) {
            if (const auto type = deduce_type(val)) {
                types.push_back(*type);
            }
        }
    }
    if (const auto *tuple = dynamic_cast<ExprTuple*>(expr.get())) {
        std::println("tuple");
        std::vector<Type> types;
        types.reserve(tuple->elems.size());
        for (const auto &elem : tuple->elems) {
            if (const auto type = deduce_type(elem)) {
                types.push_back(*type);
            }
        }
    }
    return std::nullopt;
}
