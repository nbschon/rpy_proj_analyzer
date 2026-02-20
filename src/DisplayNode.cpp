//
// Created by Noah Schonhorn on 1/4/26.
//

#include "DisplayNode.hpp"

#include <algorithm>

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

        auto offset = TextHelper::draw_text(title, tex_box.width, {title_x, 0});

        if (!fields.empty()) {
            for (const auto &field : fields) {
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

    const auto text_width = TextHelper::text_width(this->title);
    const float title_x = padding_box.x + (padding_box.width / 2 - text_width / 2);
    title_coords = raylib::Vector2(title_x, padding_box.y);

    // default_color = raylib::Color(0xe2e2e2ff);
}

// auto DisplayNode::args_to_spacing(const std::span<const float> args) -> Spacing {
//     switch (args.size()) {
//         case 0:
//             return {0.0f, 0.0f, 0.0f, 0.0f};
//         case 1: {
//             const auto all = args[0];
//             return {all, all, all, all};
//         }
//         case 2: {
//             const auto v = args[0];
//             const auto h = args[1];
//             return {v, h, v, h};
//         }
//         case 3: {
//             const auto t = args[0];
//             const auto sides = args[1];
//             const auto b = args[2];
//             return {t, sides, b, sides};
//         }
//         default: {
//             const auto t = args[0];
//             const auto r = args[1];
//             const auto b = args[2];
//             const auto l = args[3];
//             return {t, r, b, l};
//         }
//     }
// }

DisplayNode::DisplayNode(const Node* node, const raylib::Rectangle rect, std::string title)
    : title(std::move(title)), fields({}), underlying(node), margin_box(rect) {

    // main_box = margin_box;
    // main_box.x += margin;
    // main_box.y += margin;
    // main_box.width -= (margin * 2);
    // main_box.height -= (margin * 2);
    // padding_box = main_box;
    // padding_box.x += padding;
    // padding_box.y += padding;
    // padding_box.width -= (padding * 2);
    // padding_box.height -= (padding * 2);
    //
    // const auto text_width = TextHelper::text_width(this->title);
    // const float title_x = padding_box.x + (padding_box.width / 2 - text_width / 2);
    // title_coords = raylib::Vector2(title_x, padding_box.y);
    // color = raylib::Color(0xe2e2e2ff);
    // texture = raylib::RenderTexture2D(width, height);
    // setup_texture();
    setup_dimensions();
}

DisplayNode::DisplayNode(const Node* node, const raylib::Rectangle rect, std::string title, std::vector<std::string> fields)
    : title(std::move(title)), fields(std::move(fields)), underlying(node), margin_box(rect) {
    setup_dimensions();
}

bool DisplayNode::is_mouse_hovering(const raylib::Camera2D &cam) {
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

    // DrawTexture(texture.texture, box.x, box.y, raylib::Color::White());
    // DrawTexturePro(texture.texture, {0, 0, width, -height}, box, {0, 0}, 0, raylib::Color::White());
    auto offset = TextHelper::draw_text(title, static_cast<int>(padding_box.width), title_coords);
    if (!fields.empty()) {
        for (const auto &field : fields) {
            const auto [lines, did_chop] = TextHelper::draw_text_constrained(field, padding_box, offset, "...\"");
            offset += lines;
            if (did_chop) {
                break;
            }
        }
    }

    if (hover_start && mouse_pos) {
        if (std::chrono::steady_clock::now() - *hover_start > std::chrono::seconds(1)) {
            constexpr auto box_height = 60.0f;
            auto [line, col] = underlying->line_and_col();
            auto text = std::format("Line:     {:>4}\nColumn:   {:>4}", line, col);
            return text;
            const auto text_width = TextHelper::text_width(text);
            auto x = mouse_pos->x;
            auto y = mouse_pos->y - box_height;
            const raylib::Rectangle hover_box(x, y, text_width + 2, box_height);
            // raylib::RenderTexture2D render_texture(text_width + 2, box_height);
            // BeginTextureMode(render_texture);
            {
                hover_box.Draw(raylib::Color(0xE2, 0xE2, 0xE2));
                hover_box.DrawLines(raylib::Color::Black());
                TextHelper::draw_text(text, hover_box.width, {x, y});
            }
            // EndTextureMode();
            // return HoverBox(hover_box, std::move(render_texture));
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
