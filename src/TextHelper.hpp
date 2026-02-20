//
// Created by Noah Schonhorn on 1/6/26.
//

#ifndef RPY_PROJ_ANALYZER_TEXTHELPER_HPP
#define RPY_PROJ_ANALYZER_TEXTHELPER_HPP

#include "raylib-cpp.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <print>
#include <span>
#include <string_view>
#include <tuple>
#include <utility>
#include <variant>

class TextHelper {
public:
    static inline raylib::Color default_color;

private:
    static constexpr std::uint8_t BOLD      = 0b1;
    static constexpr std::uint8_t ITALIC    = 0b10;

    template<class... Ts>
    struct Overload : Ts... {
        using Ts::operator()...;
    };

    template<class... Ts>
    Overload(Ts...) -> Overload<Ts...>;

    struct Style {
        raylib::Color fg = default_color;
        std::uint8_t font = 0;

        void reset() {
            fg = default_color;
            font = 0;
        }
    };

    struct DispChar {
        int codepoint = 0;
        Style style{};
    };

    struct DispGlyph {
        int codepoint = 0;
        Style style{};
        float advance = 0;
    };

    enum class WSpaceType {
        Space,
        Tab,
        Newline,
    };

    using Displayable = std::variant<WSpaceType, std::vector<DispGlyph>>;

    static inline std::array<std::unique_ptr<raylib::Font>, ((BOLD | ITALIC) + 1)> fonts{};
    static inline DispGlyph space_glyph{0, {raylib::Color::Blank(), 0}, 0};
    static inline bool loaded_fonts = false;
    static inline float font_spacing = 2.0f;

    static auto is_space(const int &i) -> bool {
        return (i == ' ' || i == '\t' || i == '\n');
    };

    static auto font_ptr (const std::uint8_t &font) -> raylib::Font* {
        raylib::Font* ptr = fonts.at(font).get();
        if (ptr->texture.id == 0) {
            *ptr = GetFontDefault();
        }
        return ptr;
    };

    static auto color_from_hex(std::string_view hex_str, std::uint8_t alpha_mod = 0xFF) -> std::optional<raylib::Color>;

    static auto adv_glyph(const DispChar& dg) -> DispGlyph;

    static auto consume_tag(std::string_view s, std::size_t i, Style& style) -> int;

    static auto codepoint(std::string_view s, std::size_t i) -> std::pair<int, int>;

    static auto apply_formatting(std::string_view input) -> std::vector<DispChar>;

    static auto into_displayables(std::string_view input) -> std::pair<std::vector<Displayable>, float>;

    static auto dgs_to_wstring(const std::vector<DispGlyph> &disp) -> std::wstring;

    static auto split_displayable(const std::vector<DispGlyph> &disp, float max_width) -> std::vector<std::span<const DispGlyph>>;

    static auto chop_displayable(const std::vector<DispGlyph> &disp, float max_width, float cursor_x) -> std::span<const DispGlyph>;

public:
    static constexpr float font_size = 18.0f;

    /**
     * @brief Loads fonts needed to render text with custom formatting.
     *
     * @warning Must be called after raylib has been initialized, otherwise fonts will not be loaded.
     */
    static void load_fonts();
    static void unload_fonts();

    /*
     * All text drawing functions and the style
     * parsing are adapted from the following:
     * https://www.raylib.com/examples/text/loader.html?name=text_inline_styling
     * https://www.raylib.com/examples/text/loader.html?name=text_rectangle_bounds
     */
    static auto draw_text(std::string_view text, int width, raylib::Vector2 pos, int rel_line = 0.0f) -> int;

    static auto draw_text_constrained(std::string_view text, raylib::Rectangle bounds, int rel_line = 0.0f, std::string_view cont = "(...)")
        -> std::pair<int, bool>;

    // static auto draw_displayable_g_by_g(const Displayable &disp, raylib::Vector2 pos, float max_width = 0.0f) -> std::tuple<float, bool, int>;

    static auto text_width(std::string_view text) -> float;

    static auto text_height(std::string_view text, int width, float offset = 0.0f) -> float;
};


#endif //RPY_PROJ_ANALYZER_TEXTHELPER_HPP
