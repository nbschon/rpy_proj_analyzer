//
// Created by Noah Schonhorn on 6/8/26.
//

#ifndef RPY_PROJ_ANALYZER_TYPING_HPP
#define RPY_PROJ_ANALYZER_TYPING_HPP

#include <cstdint>
#include <optional>

#include "Expr.hpp"

enum class Type : std::uint8_t {
    None,
    Int,
    Float,
    Boolean,
    String,
    List,
    Tuple,
    Set,
    Dict,
};

class Typing {
    template<class... Ts>
    struct Overload : Ts... {
        using Ts::operator()...;
    };

    template<class... Ts>
    Overload(Ts...) -> Overload<Ts...>;

    [[nodiscard]] static auto lit_type(const ExprLit *lit) -> Type;

public:
    [[nodiscard]] static auto deduce_type(const std::unique_ptr<Expr> &expr) -> std::optional<Type>;
};

template<>
struct std::formatter<Type> {
    constexpr auto parse(const std::format_parse_context &ctx) {
        return ctx.begin();
    }

    auto format(const Type &type, std::format_context &ctx) const {
        const std::string str = [](const Type t) -> std::string {
            switch (t) {
                using enum Type;
                case None:
                    return "None";
                case Int:
                    return "Int";
                case Float:
                    return "Float";
                case Boolean:
                    return "Boolean";
                case String:
                    return "String";
                case List:
                    return "List";
                case Tuple:
                    return "Tuple";
                case Set:
                    return "Set";
                case Dict:
                    return "Dict";
                default:
                    std::unreachable();
            }
        }(type);
        return std::format_to(ctx.out(), "{}", str);
    }
};

#endif //RPY_PROJ_ANALYZER_TYPING_HPP
