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
#include <utility>
#include <variant>

class TextHelper {
public:
    static inline std::unique_ptr<raylib::Color> default_color;

    struct Style {
        raylib::Color fg = *default_color;
        std::uint8_t font = 0;

        void reset() {
            fg = *default_color;
            font = 0;
        }
    };

    struct DispGlyph {
        int codepoint = 0;
        Style style{};
        float advance = 0.0f;

        DispGlyph(const int codepoint, const Style style) {
            this->codepoint = codepoint;
            this->style = style;
            advance = 0.0f;
        }
        DispGlyph(const int codepoint, const Style style, const float advance) {
            this->codepoint = codepoint;
            this->style = style;
            this->advance = advance;
        }
    };

    enum class WSpaceType : std::uint8_t {
        Space,
        Tab,
        Newline,
    };

    using Displayable = std::variant<WSpaceType, std::vector<DispGlyph>>;

    struct DispText {
        std::vector<Displayable> text;
        float width;
    };

private:
    static constexpr std::uint8_t BOLD      = 0b1;
    static constexpr std::uint8_t ITALIC    = 0b10;

    template<class... Ts>
    struct Overload : Ts... {
        using Ts::operator()...;
    };

    template<class... Ts>
    Overload(Ts...) -> Overload<Ts...>;

    static inline std::array<std::unique_ptr<raylib::Font>, ((BOLD | ITALIC) + 1)> fonts{};
    static inline std::unique_ptr<DispGlyph> space_glyph;
    static inline std::unique_ptr<DispText> cont_text;
    static inline bool loaded_fonts = false;
    static inline float font_spacing = 2.0f;

    static auto is_space(const int &i) -> bool {
        return (i == ' ' || i == '\t' || i == '\n');
    };

    static auto font_ptr(const std::uint8_t &font) -> raylib::Font* {
        raylib::Font* ptr = fonts.at(font).get();
        if (ptr->texture.id == 0) {
            *ptr = GetFontDefault();
        }
        return ptr;
    };

    static auto color_from_hex(std::string_view hex_str, std::uint8_t alpha_mod = 0xFF) -> std::optional<raylib::Color>;

    static auto adv_glyph(DispGlyph& glyph) -> void;

    static auto consume_tag(std::string_view str, std::size_t idx, Style& style) -> int;

    /**
     * @brief returns the font codepoint for an index in a given string
     * @return the codepoint and its size in bytes
     */
    static auto codepoint(std::string_view str, std::size_t idx) -> std::pair<int, int>;

    static auto apply_formatting(std::string_view input) -> std::vector<DispGlyph>;

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

    static auto into_disp_text(std::string_view input) -> DispText;

    /*
     * All text drawing functions and the style
     * parsing are adapted from the following:
     * https://www.raylib.com/examples/text/loader.html?name=text_inline_styling
     * https://www.raylib.com/examples/text/loader.html?name=text_rectangle_bounds
     */
    static auto draw_text(std::string_view text, raylib::Vector2 pos, int width = 0, int rel_line = 0) -> int;
    static auto draw_text(const DispText &text, raylib::Vector2 pos, int width = 0, int rel_line = 0) -> int;

    static auto draw_text_constrained(std::string_view text, raylib::Rectangle bounds, int rel_line = 0)
        -> std::pair<int, bool>;
    static auto draw_text_constrained(const DispText &text, raylib::Rectangle bounds, int rel_line = 0)
        -> std::pair<int, bool>;

    static auto text_width(std::string_view text) -> float;
    static auto text_width(const DispText &text) -> float;

    static auto text_height(std::string_view text, int width, float offset = 0.0f) -> float;
    static auto text_height(const DispText &text, int width, float offset = 0.0f) -> float;
};


#endif //RPY_PROJ_ANALYZER_TEXTHELPER_HPP
