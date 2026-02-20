//
// Created by Noah Schonhorn on 1/4/26.
//

#ifndef RPY_PROJ_ANALYZER_DISPLAYNODE_HPP
#define RPY_PROJ_ANALYZER_DISPLAYNODE_HPP

#include <array>
#include <chrono>
#include <format>
#include <string>
#include <vector>

#include "raylib-cpp.hpp"
#include "TextHelper.hpp"

class Node;

struct HoverBox {
    raylib::Rectangle rect;
    raylib::RenderTexture2D texture;
};

class DisplayNode {
    struct Spacing {
        float t{};
        float r{};
        float b{};
        float l{};

        template<typename... Ts>
            requires (sizeof...(Ts) <= 4 && (std::same_as<Ts, float> && ...))
        explicit Spacing(const Ts&... p_args) {
            std::array<float, sizeof...(p_args)> args{p_args...};
            switch (args.size()) {
                case 0:
                    t = 0.0f, r = 0.0f, b = 0.0f, l = 0.0f;
                    break;
                case 1: {
                    t = args[0], r = args[0], b = args[0], l = args[0];
                    break;
                }
                case 2: {
                    t = args[0], b = args[0];
                    r = args[1], l = args[1];
                    break;
                }
                case 3: {
                    t = args[0];
                    r = args[1], l = args[1];
                    b = args[2];
                    break;
                }
                case 4: {
                    t = args[0];
                    r = args[1];
                    b = args[2];
                    l = args[3];
                    break;
                }
                default:
                    // having <= 4 items is a compile-time given
                    std::unreachable();
            }
        }
    };

    static constexpr float line_height = 20;
    std::string title;
    std::vector<std::string> fields;

    bool hovered = false;
    std::optional<std::chrono::steady_clock::time_point> hover_start;
    std::optional<Vector2> mouse_pos;

    const Node* underlying = nullptr;

    raylib::Vector2 title_coords;
    raylib::RenderTexture2D texture;

    void setup_texture() const;

    void setup_dimensions();

    static constexpr float width = 400.0f;
    static constexpr float height = TextHelper::font_size * 5.0f;

    // static auto args_to_spacing(std::span<const float> args) -> Spacing;

public:
    static inline raylib::Color default_color;
    static inline raylib::Color line_color;
    /*
     * These follow the CSS declaration rules for margin / padding
     * See here:
     * https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Properties/margin
     */
    static inline Spacing margin{10.0f};
    static inline Spacing padding{10.0f};
    // static constexpr unsigned padding = 10;
    // static constexpr unsigned margin = 10;

    raylib::Rectangle margin_box{};
    raylib::Rectangle padding_box{};
    raylib::Rectangle main_box{};

    explicit DisplayNode(const Node* node, raylib::Rectangle rect, std::string title);

    DisplayNode(const Node* node, raylib::Rectangle rect, std::string title, std::vector<std::string> fields);

    auto is_mouse_hovering(const raylib::Camera2D& cam) -> bool;

    [[nodiscard]] auto draw() const -> std::optional<std::string>;

    [[nodiscard]] auto to_string() const -> std::string;

    static auto get_width() -> float;

    static auto get_height() -> float;
};

template<>
struct std::formatter<DisplayNode> {
    constexpr auto parse(const std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const DisplayNode& dn, std::format_context& ctx) {
        return std::format_to(ctx.out(), "{}", dn.to_string());
    }
};


#endif //RPY_PROJ_ANALYZER_DISPLAYNODE_HPP
