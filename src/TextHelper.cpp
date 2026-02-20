//
// Created by Noah Schonhorn on 1/6/26.
//

#include "TextHelper.hpp"

#include <algorithm>
#include <cassert>
#include <charconv>
#include <format>
#include <iostream>
#include <print>
#include <ranges>
#include <string>

auto TextHelper::color_from_hex(const std::string_view hex_str, const std::uint8_t alpha_mod) -> std::optional<raylib::Color> {
    switch (hex_str.length()) {
        case 3: // #rgb
        case 4: // #rgba
        case 6: // #rrggbb
        case 8: // #rrggbbaa
            break;
        default:
            return std::nullopt;
    }

    std::uint32_t col_val = 0;
    auto [ptr, ec] = std::from_chars(
        hex_str.data(), hex_str.data() + hex_str.length(), col_val, 16);
    if (ec != std::errc() || ptr != hex_str.data() + hex_str.length()) {
        return std::nullopt;
    }

    raylib::Color ret_col = [&]() -> raylib::Color {
        switch (hex_str.length()) {
            case 3: {
                std::uint8_t r = (col_val & 0xF00) >> 8;
                std::uint8_t g = (col_val & 0xF0) >> 4;
                std::uint8_t b = col_val & 0xF;
                r |= r << 4;
                g |= g << 4;
                b |= b << 4;
                return {r, g, b};
            }
            case 4: {
                std::uint8_t r = (col_val & 0xF000) >> 12;
                std::uint8_t g = (col_val & 0xF00) >> 8;
                std::uint8_t b = (col_val & 0xF0) >> 4;
                std::uint8_t a = (col_val & 0xF);
                r |= r << 4;
                g |= g << 4;
                b |= b << 4;
                a |= a << 4;
                return {r, g, b, a};
            }
            case 6: {
                col_val = col_val << 8 | 0xFF;
                return col_val;
            }
            case 8: {
                return col_val;
            }
            default:
                std::unreachable();
        }
    }();

    if (alpha_mod != 0xFF) {
        ret_col.a *= alpha_mod / 255.0f;
    }
    return ret_col;
}

auto TextHelper::adv_glyph(const DispChar& dg) -> DispGlyph {
    const auto font = font_ptr(dg.style.font);

    const float scale = font_size / static_cast<float>(font->baseSize);
    const auto glyph_idx = GetGlyphIndex(*font, dg.codepoint);

    float adv = 0.0f;
    if (font->glyphs[glyph_idx].advanceX == 0) {
        adv = font->recs[glyph_idx].width * scale;
    } else {
        adv = static_cast<float>(font->glyphs[glyph_idx].advanceX) * scale;
    }

    return {dg.codepoint, dg.style, adv};
}

auto TextHelper::consume_tag(const std::string_view s, const std::size_t i, Style& style) -> int {
    if (i >= s.length() || s.at(i) != '{') {
        return 0;
    }
    if (i != 0 && s.at(i - 1) == '\\') {
        return 1;
    }

    int curr_pos = static_cast<int>(i + 1);
    std::string tag_buff;

    while (curr_pos < s.length() && s.at(curr_pos) != '}') {
        tag_buff += s.at(curr_pos);
        curr_pos++;
    }
    if (curr_pos < s.length() && s.at(curr_pos) == '}') {
        curr_pos++;
    }

    const int idx = static_cast<int>(i);
    if (constexpr std::string_view tag = "color=#"; tag_buff.starts_with(tag)) {
        const std::string_view color(tag_buff.data() + tag.length());
        if (const auto rl_col = color_from_hex(color)) {
            style.fg = *rl_col;
            return curr_pos - idx;
        }
    } else if (constexpr std::string_view t = "/color"; t == tag_buff) {
        style.fg = raylib::Color::Black();
    } else if (tag_buff == "b") {
        style.font |= BOLD;
    } else if (tag_buff == "/b") {
        style.font &= ~BOLD;
    } else if (tag_buff == "i") {
        style.font |= ITALIC;
    } else if (tag_buff == "/i") {
        style.font &= ~ITALIC;
    } else {
        // std::println(std::cerr, "Warning: Unknown / unsupported tag \"{{tag_buff}}\"");
        return 0;
    }

    return curr_pos - idx;
}

auto TextHelper::codepoint(const std::string_view s, const std::size_t i) -> std::pair<int, int> {
    int bytes = 0;
    int codepoint = GetCodepointNext(s.data() + i, &bytes);
    if (bytes <= 0) {
        return {'?', 1};
    }
    return {codepoint, bytes};
}

auto TextHelper::apply_formatting(const std::string_view input) -> std::vector<DispChar> {
    Style style{};
    std::vector<DispChar> styled_chars;
    int i = 0;
    while (i < input.length()) {
        if (input.at(i) == '{') {
            if (const int consumed = consume_tag(input, i, style); consumed > 0) {
                i += consumed;
            } else {
                styled_chars.emplace_back('{', style);
                i += 1;
            }
        } else {
            auto [point, bytes] = codepoint(input, i);
            styled_chars.emplace_back(point, style);
            i += bytes;
        }
    }
    style.reset();
    return styled_chars;
}

auto TextHelper::into_displayables(const std::string_view input) -> std::pair<std::vector<Displayable>, float> {
    auto formatted = apply_formatting(input);
    std::vector<Displayable> disp_buff;
    float width = 0.0f;

    int i = 0;
    while (i < formatted.size()) {
        const auto& [codepoint, style] = formatted.at(i);
        if (const auto cp = codepoint; is_space(cp)) {
            switch (cp) {
                case ' ':
                    disp_buff.emplace_back(WSpaceType::Space);
                    width += space_glyph.advance;
                    break;
                case '\t':
                    disp_buff.emplace_back(WSpaceType::Tab);
                    width += space_glyph.advance * 4;
                    break;
                case '\n':
                    disp_buff.emplace_back(WSpaceType::Newline);
                    break;
                default:
                    std::unreachable();
            }
            i++;
        } else {
            std::vector<DispGlyph> dc_word;
            float word_width = 0.0f;
            while (i < formatted.size() && !is_space(formatted.at(i).codepoint)) {
                dc_word.push_back(adv_glyph(formatted.at(i)));
                word_width += dc_word.back().advance;
                i++;
            }
            width += word_width;
            disp_buff.emplace_back(std::move(dc_word));
        }
    }

    return {disp_buff, width};
}

auto TextHelper::dgs_to_wstring(const std::vector<DispGlyph> &dgs) -> std::wstring {
    std::wstring as_chars;

    int i = 0;
    while (i < dgs.size()) {
        const auto& [codepoint, style, adv] = dgs.at(i);
        if (const auto cp = codepoint; is_space(cp)) {
            switch (cp) {
                case ' ':
                case '\t':
                case '\n':
                    as_chars.push_back(static_cast<wchar_t>(codepoint));
                    break;
                default:
                    std::unreachable();
            }
            i++;
        } else {
            while (i < dgs.size() && !is_space(dgs.at(i).codepoint)) {
                as_chars.push_back(static_cast<wchar_t>(dgs.at(i).codepoint));
                i++;
            }
        }
    }

    return as_chars;
}
auto TextHelper::split_displayable(const std::vector<DispGlyph> &disp, const float max_width) -> std::vector<std::span<const DispGlyph>> {
    float curr_width = 0.0f;
    int left = 0;
    const std::span all(disp);
    std::vector<std::span<const DispGlyph>> dgs;

    for (int i = 0; i < disp.size(); ++i) {
        const auto &dg = disp.at(i);
        if (curr_width + dg.advance >= max_width) {
            dgs.emplace_back(all.subspan(left, i - left));
            curr_width = 0.0f;
            left = i;
        }

        curr_width += dg.advance;
    }

    if (left < disp.size()) {
        dgs.emplace_back(all.subspan(left));
    }

    return dgs;
}

auto TextHelper::chop_displayable(const std::vector<DispGlyph>& disp, float max_width, float cursor_x = 0.0f) -> std::span<const DispGlyph> {
    float curr_width = 0.0f;
    int count = 0;
    const std::span all(disp);

    for (int i = 0; i < disp.size(); ++i) {
        const auto &dg = disp.at(i);
        curr_width += dg.advance;
        if (curr_width + cursor_x >= max_width) {
            count = i;
            break;
        }

    }

    // if (count > disp.size()) {
    //     return disp;
    // }

    return all.subspan(0, count);
}

void TextHelper::load_fonts() {
    try {
        fonts.at(0) =
            std::make_unique<raylib::Font>("./fonts/LiberationMono-Regular.ttf", 64, nullptr, 250);
        fonts.at(BOLD) =
            std::make_unique<raylib::Font>("./fonts/LiberationMono-Bold.ttf", 64, nullptr, 250);
        fonts.at(ITALIC) =
            std::make_unique<raylib::Font>("./fonts/LiberationMono-Italic.ttf", 64, nullptr, 250);
        fonts.at(BOLD | ITALIC) =
            std::make_unique<raylib::Font>("./fonts/LiberationMono-BoldItalic.ttf", 64, nullptr, 250);

        for (const auto& font : fonts) {
            SetTextureFilter(font->texture, TEXTURE_FILTER_BILINEAR);
        }

    } catch (raylib::RaylibException &e) {
        std::println(std::cerr, "Could not load fonts. {}", e.what());
        fonts.at(0) = std::make_unique<raylib::Font>(GetFontDefault());
        fonts.at(BOLD) = std::make_unique<raylib::Font>(GetFontDefault());
        fonts.at(ITALIC) = std::make_unique<raylib::Font>(GetFontDefault());
        fonts.at(BOLD | ITALIC) = std::make_unique<raylib::Font>(GetFontDefault());
        font_spacing = 6.0f;
    }

    space_glyph = adv_glyph({codepoint(" ", 0).first, Style{}});
}

void TextHelper::unload_fonts() {
    for (auto& font : fonts) {
        font.reset();
    }
}

auto TextHelper::draw_text(const std::string_view text, const int width, const raylib::Vector2 pos, int rel_line) -> int {
    const auto [ds, _] = into_displayables(text);

    constexpr float line_height = font_size;
    float cursor_x = pos.x;
    float cursor_y = pos.y + (rel_line * line_height);
    bool just_wrapped = false;
    int newlines = 0;

    auto draw_glyph = [&](const DispGlyph& glyph) {
        const auto font = font_ptr(glyph.style.font);
        const auto cp = glyph.codepoint;
        const auto color = glyph.style.fg;
        DrawTextCodepoint(*font, cp, {cursor_x, cursor_y}, font_size, color);
        cursor_x += glyph.advance;
    };

    auto reset_cursor = [&] {
        cursor_x = pos.x;
        cursor_y += line_height;
        just_wrapped = true;
        newlines += 1;
    };

    for (const auto& word : ds) {
        std::visit(
            Overload{
                [&](const std::vector<DispGlyph>& dgs) {
                    const auto word_width = std::ranges::fold_left(
                        dgs, 0.0f,
                        [](const float w, const DispGlyph& dg) {
                            return w + dg.advance;
                        });
                    if (word_width > pos.x + width) {
                        // if whole word is longer than box, wrap it
                        auto split = split_displayable(dgs, width);
                        for (const auto &split_dgs : split) {
                            for (const auto &dg : split_dgs) {
                                draw_glyph(dg);
                            }
                            reset_cursor();
                            just_wrapped = true;
                        }
                    } else if (word_width + cursor_x > pos.x + width) {
                        // if word goes beyond box, push to next line
                        reset_cursor();
                        for (const auto &dg : dgs) {
                            draw_glyph(dg);
                        }
                    } else {
                        // otherwise, draw normally
                        for (const auto &dg : dgs) {
                            draw_glyph(dg);
                        }
                    }

                    just_wrapped = false;
                },
                [&](const WSpaceType& ws) {
                    switch (ws) {
                        case WSpaceType::Space:
                            if (!just_wrapped) {
                                draw_glyph(space_glyph);
                            }
                            break;
                        case WSpaceType::Tab:
                            for (int i = 0; i < 4; ++i) {
                                draw_glyph(space_glyph);
                            }
                            break;
                        case WSpaceType::Newline:
                            reset_cursor();
                            break;
                    }
                }
            }, word);
    }

    return newlines + 1;
}

auto TextHelper::draw_text_constrained(const std::string_view text, const raylib::Rectangle bounds, const int rel_line, const std::string_view cont)
    -> std::pair<int, bool> {
    const auto [ds, _] = into_displayables(text);
    const auto [cont_d, cont_width] = into_displayables(cont);

    bool last_line = false;

    constexpr float line_height = font_size;
    float cursor_x = bounds.x;
    float cursor_y = bounds.y + (rel_line * line_height);
    bool just_wrapped = false;
    float newlines = 0;

    auto draw_glyph = [&](const DispGlyph& glyph) {
        const auto font = font_ptr(glyph.style.font);
        const auto cp = glyph.codepoint;
        const auto color = glyph.style.fg;
        DrawTextCodepoint(*font, cp, {cursor_x, cursor_y}, font_size, color);
        cursor_x += glyph.advance;
    };

    auto can_advance = [&] {
        return cursor_y + line_height * 2 <= bounds.y + bounds.height;
    };

    auto reset_cursor = [&] {
        cursor_x = bounds.x;
        cursor_y += line_height;
        just_wrapped = true;
        newlines += 1;
        if (!can_advance()) {
            last_line = true;
        }
    };

    bool chop = false;
    const auto max = bounds.x + bounds.width - cont_width;

    std::optional<int> continue_idx = std::nullopt;

    for (int i = 0; i < ds.size(); ++i) {
        if (!last_line) {
            std::visit(
                Overload{
                    [&](const std::vector<DispGlyph>& dgs) {
                        auto wchars = dgs_to_wstring(dgs);
                        const auto word_width = std::ranges::fold_left(
                            dgs, 0.0f,
                            [](const float w, const DispGlyph& dg) {
                                return w + dg.advance;
                            });
                        if (word_width > bounds.width) {
                            // if whole word is longer than box, wrap it
                            auto split = split_displayable(dgs, bounds.width);
                            for (const auto &split_dgs : split) {
                                for (const auto &dg : split_dgs) {
                                    draw_glyph(dg);
                                }
                                reset_cursor();
                                just_wrapped = true;
                            }
                        } else if (word_width + cursor_x >= bounds.x + bounds.width) {
                            // if word goes beyond box, push to next line
                            reset_cursor();
                            if (!last_line) {
                                for (const auto &dg : dgs) {
                                    draw_glyph(dg);
                                }
                            }
                        } else {
                            // otherwise, draw normally
                            for (const auto &dg : dgs) {
                                draw_glyph(dg);
                            }
                        }

                        just_wrapped = false;
                    },
                    [&](const WSpaceType& ws) {
                        switch (ws) {
                            case WSpaceType::Space:
                                if (!just_wrapped) {
                                    draw_glyph(space_glyph);
                                }
                                break;
                            case WSpaceType::Tab:
                                for (int i = 0; i < 4; ++i) {
                                    draw_glyph(space_glyph);
                                }
                                break;
                            case WSpaceType::Newline:
                                reset_cursor();
                                break;
                        }
                    }
                }, ds.at(i));
        }
        if (last_line) {
            continue_idx = i;
            break;
        }
    }

    if (continue_idx) {
        std::vector<DispGlyph> dgs;
        for (int j = *continue_idx; j < ds.size(); ++j) {
            std::visit(Overload{
                [&](const std::vector<DispGlyph>& new_dgs) {
                    dgs.insert(dgs.end(), new_dgs.begin(), new_dgs.end());
                },
                [&](const WSpaceType& ws) {
                    switch (ws) {
                        case WSpaceType::Space:
                            dgs.push_back(space_glyph);
                            break;
                        case WSpaceType::Tab:
                            for (int k = 0; k < 4; ++k) {
                                dgs.push_back(space_glyph);
                            }
                            break;
                        case WSpaceType::Newline:
                            break;
                    }
                },
            }, ds.at(j));
        }
        int last_codepoint = 0;
        for (const auto &dg : dgs) {
            if (cursor_x + dg.advance <= max) {
                draw_glyph(dg);
                last_codepoint = dg.codepoint;
            } else {
                if (is_space(last_codepoint)) {
                    cursor_x -= space_glyph.advance;
                }
                chop = true;
                break;
            }
        }

        if (chop) {
            draw_text(cont, bounds.width, {cursor_x, cursor_y});
        }
    }

    return {newlines + 1, chop};
}

// auto TextHelper::draw_displayable_g_by_g(const Displayable& disp, raylib::Vector2 pos, float max_width) -> std::tuple<float, bool, int>{
//     auto cursor_x = pos.x;
//     auto cursor_y = pos.y;
//
//     bool chopped = false;
//     int last_codepoint;
//
//     auto draw_glyph = [&](const DispGlyph& glyph) {
//         const auto font = font_ptr(glyph.style.font);
//         const auto cp = glyph.codepoint;
//         const auto color = glyph.style.fg;
//         DrawTextCodepoint(*font, cp, {cursor_x, cursor_y}, font_size, color);
//         cursor_x += glyph.advance;
//         if (cursor_x >= max_width) {
//             chopped = true;
//         }
//     };
//
//     std::visit(Overload {
//         [&](const std::vector<DispGlyph>& dgs) {
//             for (const auto &dg : dgs) {
//                 if (cursor_x + dg.advance < max_width) {
//                     draw_glyph(dg);
//                     last_codepoint = dg.codepoint;
//                     if (chopped) {
//                         break;
//                     }
//                 }
//             }
//         },
//         [&](const WSpaceType &ws) {
//             switch (ws) {
//                 case WSpaceType::Space:
//                     draw_glyph(space_glyph);
//                     last_codepoint = space_glyph.codepoint;
//                     break;
//                 case WSpaceType::Tab:
//                     for (int i = 0; i < 4; ++i) {
//                         draw_glyph(space_glyph);
//                         if (chopped) {
//                             break;
//                         }
//                     }
//                     last_codepoint = space_glyph.codepoint;
//                     break;
//                 case WSpaceType::Newline:
//                     last_codepoint = '\n';
//                     chopped = true;
//             }
//         }
//     }, disp);
//
//     return {cursor_x, chopped, last_codepoint};
// }

auto TextHelper::text_width(const std::string_view text) -> float {
    const auto [ds, _] = into_displayables(text);
    float cursor_x = 0.0f;
    float cursor_y = 0.0f;
    const float line_height = font_size;
    bool just_wrapped = false;

    float width = 0.0f;
    float highest_width = 0.0f;

    auto dummy_draw = [&](const DispGlyph& glyph) {
        cursor_x += glyph.advance;
        width += glyph.advance;
    };

    auto reset_cursor = [&] {
        cursor_x = 0.0f;
        cursor_y += line_height;
        just_wrapped = true;
        highest_width = std::max(width, highest_width);
        width = 0.0f;
    };

    for (const auto& word : ds) {
        std::visit(
            Overload{
                [&](const std::vector<DispGlyph>& dgs) {
                    for (const auto &dg : dgs) {
                        dummy_draw(dg);
                    }
                    just_wrapped = false;
                },
                [&](const WSpaceType& ws) {
                    switch (ws) {
                        case WSpaceType::Space:
                            if (!just_wrapped) {
                                dummy_draw(space_glyph);
                            }
                            break;
                        case WSpaceType::Tab:
                            for (int i = 0; i < 4; ++i) {
                                dummy_draw(space_glyph);
                            }
                            break;
                        case WSpaceType::Newline:
                            reset_cursor();
                            break;
                    }
                },
                [&](auto &&) {
                    assert(false);
                    std::unreachable();
                }
            }, word);
    }

    return width;
}

auto TextHelper::text_height(const std::string_view text, const int width, const float offset) -> float {
    const auto [ds, _] = into_displayables(text);

    const auto f_width = static_cast<float>(width);
    constexpr float line_height = font_size;
    float cursor_x = 0.0f;
    float cursor_y = offset;
    float newlines = 0;
    bool just_wrapped = false;

    auto dummy_draw = [&](const DispGlyph& glyph) {
        cursor_x += glyph.advance;
    };

    auto reset_cursor = [&] {
        cursor_x = 0.0f;
        cursor_y += line_height;
        just_wrapped = true;
        newlines += 1.0f;
    };

    for (const auto& word : ds) {
        std::visit(
            Overload{
                [&](const std::vector<DispGlyph>& dgs) {
                    const auto word_width = std::ranges::fold_left(
                        dgs, 0.0,
                        [](const float w, const DispGlyph& dg) {
                            return w + dg.advance;
                        });
                    if (word_width + cursor_x > f_width) {
                        reset_cursor();
                    }
                    for (const auto &dg : dgs) {
                        dummy_draw(dg);
                    }
                    just_wrapped = false;
                },
                [&](const WSpaceType& ws) {
                    switch (ws) {
                        case WSpaceType::Space:
                            if (!just_wrapped) {
                                dummy_draw(space_glyph);
                            }
                            break;
                        case WSpaceType::Tab:
                            for (int i = 0; i < 4; ++i) {
                                dummy_draw(space_glyph);
                            }
                            break;
                        case WSpaceType::Newline:
                            reset_cursor();
                            break;
                    }
                },
                [&](auto &&) {
                    assert(false);
                    std::unreachable();
                }
            }, word);
    }

    if (newlines == 0) {
        newlines = 1;
    }
    return (newlines) * line_height;
}
