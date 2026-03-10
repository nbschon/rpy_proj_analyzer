//
// Created by Noah Schonhorn on 3/6/26.
//

#include "Panel.hpp"

#include "TextHelper.hpp"

FileTreePanel::FileTreePanel(const std::filesystem::path &path)
    : Panel({0.0, 0.0, 200, 1000}), path(path), tree(build_dir_tree(path)) {}

void FileTreePanel::update(const raylib::Window &win) {

}

void FileTreePanel::draw(const raylib::Window &win) {
    rect.Draw(raylib::Color::White());
    float y = 10.0;

    std::function<void(const DirTree&, int)> rec_draw = [&](const DirTree &t, const int lvl) -> void {
        for (const auto &child : t.children) {
            if (child.is_dir) {
                dir_icon.Draw(raylib::Vector2{0.0f, y});
                TextHelper::draw_text(child.name, {static_cast<float>(dir_icon.width), y});
            } else {
                doc_icon.Draw(raylib::Vector2{0.0f, y});
                TextHelper::draw_text(child.name, {static_cast<float>(doc_icon.width), y});
            }
            y += 20.0f;
            if (child.is_dir) {
                rec_draw(child, lvl + 1);
            }
        }
    };

    rec_draw(tree, 0);
}
