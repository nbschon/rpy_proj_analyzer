//
// Created by Noah Schonhorn on 3/17/26.
//

#ifndef RPY_PROJ_ANALYZER_ATL_HPP
#define RPY_PROJ_ANALYZER_ATL_HPP

#include <string_view>

enum class Transition : std::uint8_t {
    Dissolve,
    Fade,
    Pixellate,
    Move,
    MoveInRight,
    MoveInLeft,
    MoveInTop,
    MoveInBottom,
    MoveOutRight,
    MoveOutLeft,
    MoveOutTop,
    MoveOutBottom,
    Ease,
    EaseInRight,
    EaseInLeft,
    EaseInTop,
    EaseInBottom,
    EaseOutRight,
    EaseOutLeft,
    EaseOutTop,
    EaseOutBottom,
    ZoomIn,
    ZoomOut,
    ZoomInOut,
    VPunch,
    HPunch,
    Blinds,
    Squares,
    WipeLeft,
    WipeRight,
    WipeUp,
    WipeDown,
    SlideLeft,
    SlideRight,
    SlideUp,
    SlideDown,
    SlideAwayLeft,
    SlideAwayRight,
    SlideAwayUp,
    SlideAwayDown,
    PushRight,
    PushLeft,
    PushUp,
    PushDown,
    IrisIn,
    IrisOut,
};

enum class Warper : std::uint8_t {
    Pause,
    Linear,
    Ease,
    EaseIn,
    EaseOut,
};

class ATL {

public:
    static auto get_trans(const std::string_view &str) -> Transition;
    static auto get_warper(const std::string_view &str) -> Warper;

    static auto trans_str(const Transition &trans) -> std::string;
    static auto warper_str(const Warper &warper) -> std::string;
};


#endif //RPY_PROJ_ANALYZER_ATL_HPP