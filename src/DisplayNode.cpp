//
// Created by Noah Schonhorn on 1/4/26.
//

#include "DisplayNode.hpp"

#include <algorithm>
#include <ranges>

#include "raylib-cpp.hpp"

#include "ArgVParser.hpp"
#include "Node.hpp"
#include "TextHelper.hpp"

void DisplayNode::setup_texture() const {
    BeginTextureMode(texture);
    {
        ClearBackground(raylib::Color::Blank());
        const raylib::Rectangle tex_box{0, 0, width, height};
        const auto text_width = TextHelper::text_width(this->title);
        const float title_x = (padding_box.width / 2 - text_width / 2);

        tex_box.Draw(raylib::Color(0xE2, 0xE2, 0xE2));
        tex_box.DrawLines(raylib::Color::Black());

        auto offset = TextHelper::draw_text(title_text, {title_x, 0}, tex_box.width);

        if (!fields.empty()) {
            for (const auto &field : fields_text) {
                const auto [lines, chop] = TextHelper::draw_text_constrained(field, tex_box, offset);
                offset += lines;
                if (chop) {
                    break;
                }
            }
        }
    }

    EndTextureMode();
    SetTextureFilter(texture.texture, TEXTURE_FILTER_BILINEAR);
}

void DisplayNode::setup_dimensions() {
    // TODO: this isn't quite right. needs tweaking
    main_box = margin_box;
    main_box.x += margin.l;
    main_box.y += margin.t;
    main_box.width -= (margin.t + margin.b);
    main_box.height -= (margin.l + margin.r);
    padding_box = main_box;
    padding_box.x += padding.l;
    padding_box.y += padding.t;
    padding_box.width -= (padding.t + padding.b);
    padding_box.height -= (padding.l + padding.r);

    const auto text_width = TextHelper::text_width(this->title_text);
    const float title_x = padding_box.x + (padding_box.width / 2 - text_width / 2);
    title_coords = raylib::Vector2(title_x, padding_box.y);
}

DisplayNode::DisplayNode(const Node* node, const raylib::Rectangle rect, std::string title)
    : title(std::move(title)), fields({}), underlying(node), margin_box(rect),
    title_text(TextHelper::into_disp_text(this->title)), fields_text({}) {
    setup_dimensions();
}

DisplayNode::DisplayNode(const Node* node, const raylib::Rectangle rect, std::string title, std::vector<std::string> fields)
    : title(std::move(title)), title_text(TextHelper::into_disp_text(this->title)), fields(std::move(fields)),
    fields_text(this->fields | std::views::transform([&](const std::string &txt) -> TextHelper::DispText {
          return TextHelper::into_disp_text(txt);
      }) | std::ranges::to<std::vector<TextHelper::DispText>>()),
    underlying(node), margin_box(rect) {
    setup_dimensions();
}

auto DisplayNode::is_mouse_hovering(const raylib::Camera2D &cam) -> bool {
    Vector2 curr_mouse_pos = GetScreenToWorld2D(GetMousePosition(), cam);
    hovered = padding_box.CheckCollision(curr_mouse_pos);
    if (hovered) {
        if (!hover_start) {
            hover_start = std::chrono::steady_clock::now();
        }
        mouse_pos = curr_mouse_pos;
    } else {
        hover_start.reset();
        mouse_pos.reset();
    }
    return hovered;
}

auto DisplayNode::draw() const -> std::optional<std::string> {
    main_box.Draw(default_color);

    if (hovered) {
        DrawRectangleLinesEx(main_box, 2.0, line_color);
    } else {
        main_box.DrawLines(line_color);
    }

    auto offset = TextHelper::draw_text(title_text, title_coords, static_cast<int>(padding_box.width));
    if (!fields.empty()) {
        for (const auto &field : fields_text) {
            const auto [lines, did_chop] = TextHelper::draw_text_constrained(field, padding_box, offset);
            offset += lines;
            if (did_chop) {
                break;
            }
        }
    }

    if (hover_start && mouse_pos) {
        if (std::chrono::steady_clock::now() - *hover_start > std::chrono::seconds(1)) {
            auto [line, col] = underlying->line_and_col();
            auto text = std::format("Line:     {:>4}\nColumn:   {:>4}", line, col);
            return text;
        }
    }

    return std::nullopt;
}

auto DisplayNode::to_string() const -> std::string {
    return std::format("\"{}\", n fields: {}", title, fields.size());
}

auto DisplayNode::get_width() -> float {
    return width + (margin.l + margin.r) + (padding.l + padding.r);
}

auto DisplayNode::get_height() -> float {
    return height + (margin.t + margin.b) + (padding.t + padding.b);
}
