//
// Created by Noah Schonhorn on 3/6/26.
//

#include "Panel.hpp"

#include "TextHelper.hpp"

FileTreePanel::FileTreePanel(const std::filesystem::path &path)
    : Panel({0.0, 0.0, 200, 1000}), path(path), tree(build_dir_tree(path)) {
    float y = 0.0f;
    for (auto &child : tree.children) {
        files.emplace_back(raylib::Rectangle{0.0f, y, 200, 20}, &child, 0);
        y += 20.0f;
    }
}

void FileTreePanel::update(const raylib::Window &win) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && rect.CheckCollision(GetMousePosition())) {
        const auto last_size = files.size();
        for (int i = 0; i < files.size(); ++i) {
            auto &[file_rect, node, depth, open] = files.at(i);
            if (file_rect.CheckCollision(GetMousePosition())) {
                if (node->is_dir && !open) {
                    open = true;
                    std::vector<FileEntry> new_files;
                    float new_y = file_rect.y + 20.0f;
                    new_files.reserve(node->children.size());
                    for (auto &child : node->children) {
                        raylib::Rectangle new_rect{file_rect.x + 10.0f, new_y, file_rect.width -  10.0f, file_rect.height};
                        new_files.emplace_back(new_rect, &child, depth + 1);
                        new_y += 20.0f;
                    }
                    files.insert(files.begin() + i + 1, new_files.begin(), new_files.end());
                    break;
                }
                if (node->is_dir && open) {
                    int j = i + 1;
                    while (j < files.size()) {
                        if (files.at(j).depth > depth) {
                            j++;
                        } else {
                            break;
                        }
                    }
                    files.erase(files.begin() + i + 1, files.begin() + j);
                    open = false;
                    break;
                }
                if (!node->is_dir) {
                    curr_script = node->path;
                }
            }
        }

        if (files.size() != last_size) {
            float y = 0.0f;
            for (auto &[rect, n, depth, o] : files) {
                rect.x = 0.0f + (static_cast<float>(depth) * 10.0f);
                rect.y = y;
                rect.width = 200.0f - (static_cast<float>(depth) * 10.0f);
                y += 20.0f;
            }
        }
    }
}

void FileTreePanel::draw(const raylib::Window &win) {
    rect.Draw(raylib::Color::White());

    const std::function<void(const DirTree&, int)> rec_draw = [&](const DirTree &t, const int lvl) -> void {
        for (const auto & [rect, node, d, o] : files) {
            if (node->is_dir) {
                rect.DrawLines(raylib::Color::Black());
                dir_icon->Draw(raylib::Vector2{rect.x, rect.y});
                auto draw_at = rect;
                draw_at.x += dir_icon->width;
                draw_at.width -= dir_icon->width;
                TextHelper::draw_text(node->name, {draw_at.x, rect.y});
            } else {
                rect.DrawLines(raylib::Color::Black());
                doc_icon->Draw(raylib::Vector2{rect.x, rect.y});
                auto draw_at = rect;
                draw_at.x += doc_icon->width;
                draw_at.width -= doc_icon->width;
                TextHelper::draw_text(node->name, {draw_at.x, rect.y});
            }
        }
    };

    rec_draw(tree, 0);
}

void FileTreePanel::unload_textures() {
    doc_icon.reset();
    dir_icon.reset();
}
