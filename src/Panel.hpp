//
// Created by Noah Schonhorn on 3/6/26.
//

#ifndef RPY_PROJ_ANALYZER_PANEL_HPP
#define RPY_PROJ_ANALYZER_PANEL_HPP

#include <algorithm>
#include <filesystem>
#include <format>
#include <functional>
#include <string>
#include <unordered_set>
#include <vector>

#include "raylib-cpp.hpp"

class Panel {
protected:
    raylib::Rectangle rect;

public:
    explicit Panel(const raylib::Rectangle rect) : rect(rect) {};
    virtual ~Panel() = default;
    virtual void update(const raylib::Window &win) = 0;
    virtual void draw(const raylib::Window &win) = 0;
};

struct DirTree {
    std::string name;
    std::filesystem::path path;
    std::vector<DirTree> children;
    bool is_dir;

    explicit DirTree(const std::filesystem::path &path) : path(path) {
        is_dir = is_directory(path);
        name = path.filename().string();
    }

    auto operator<(const DirTree &other) const -> bool {
        if (other.is_dir && !is_dir) {
            return false;
        }
        if (!other.is_dir && is_dir) {
            return true;
        }
        return name < other.name;
    }
};

inline auto flat_files(const DirTree &tree) -> std::vector<std::string>  {
    std::vector<std::string> files;

    std::function<void(const DirTree&, int)> rec_add = [&](const DirTree &t, const int lvl) -> void {
        for (const auto &child : t.children) {
            std::string spaces(lvl, '\t');
            files.push_back(std::format("{}{}", spaces, child.name));
            if (child.is_dir) {
                rec_add(child, lvl + 1);
            }
        }
    };

    rec_add(tree, 0);

    return files;
}

inline auto build_dir_tree(const std::filesystem::path &path) -> DirTree {
    DirTree root(path);

    static const std::unordered_set<std::filesystem::path> valid_exts = {".rpy"};

    for (const auto &dir : std::filesystem::directory_iterator(path)) {
        if (std::filesystem::is_directory(dir)) {
            if (auto sub_dir = build_dir_tree(dir.path()); !sub_dir.children.empty()) {
                root.children.push_back(sub_dir);
            }
        } else if (std::filesystem::is_regular_file(dir) && valid_exts.contains(dir.path().extension())) {
            root.children.emplace_back(dir.path());
        }
    }

    std::ranges::sort(root.children, [](const DirTree &a, const DirTree &b) -> bool {
        return a < b;
    });

    return root;
}

class FileTreePanel final : public Panel {
    std::filesystem::path path;
    DirTree tree;

public:
    static inline raylib::Texture2D doc_icon;
    static inline raylib::Texture2D dir_icon;
    explicit FileTreePanel(const std::filesystem::path &path);
    void update(const raylib::Window &win) override;
    void draw(const raylib::Window &win) override;
};


#endif //RPY_PROJ_ANALYZER_PANEL_HPP