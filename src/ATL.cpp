//
// Created by Noah Schonhorn on 3/17/26.
//

#include "ATL.hpp"

#include <iostream>
#include <print>
#include <utility>

auto ATL::get_trans(const std::string_view &str) -> Transition {
    if (str == "dissolve") {
        return Transition::Dissolve;
    }
    if (str == "fade") {
        return  Transition::Fade;
    }
    if (str == "pixellate") {
        return  Transition::Pixellate;
    }
    if (str == "move") {
        return  Transition::Move;
    }
    if (str == "moveinright") {
        return  Transition::MoveInRight;
    }
    if (str == "moveinleft") {
        return  Transition::MoveInLeft;
    }
    if (str == "moveintop") {
        return  Transition::MoveInTop;
    }
    if (str == "moveinbottom") {
        return  Transition::MoveInBottom;
    }
    if (str == "moveoutright") {
        return  Transition::MoveOutRight;
    }
    if (str == "moveoutleft") {
        return  Transition::MoveOutLeft;
    }
    if (str == "moveouttop") {
        return  Transition::MoveOutTop;
    }
    if (str == "moveoutbottom") {
        return  Transition::MoveOutBottom;
    }
    if (str == "ease") {
        return  Transition::Ease;
    }
    if (str == "easeinright") {
        return  Transition::EaseInRight;
    }
    if (str == "easeinleft") {
        return  Transition::EaseInLeft;
    }
    if (str == "easeintop") {
        return  Transition::EaseInTop;
    }
    if (str == "easeinbottom") {
        return  Transition::EaseInBottom;
    }
    if (str == "easeoutright") {
        return  Transition::EaseOutRight;
    }
    if (str == "easeoutleft") {
        return  Transition::EaseOutLeft;
    }
    if (str == "easeouttop") {
        return  Transition::EaseOutTop;
    }
    if (str == "easeoutbottom") {
        return  Transition::EaseOutBottom;
    }
    if (str == "zoomin") {
        return  Transition::ZoomIn;
    }
    if (str == "zoomout") {
        return  Transition::ZoomOut;
    }
    if (str == "zoominout") {
        return  Transition::ZoomInOut;
    }
    if (str == "vpunch") {
        return  Transition::VPunch;
    }
    if (str == "hpunch") {
        return  Transition::HPunch;
    }
    if (str == "blinds") {
        return  Transition::Blinds;
    }
    if (str == "squares") {
        return  Transition::Squares;
    }
    if (str == "wipeleft") {
        return  Transition::WipeLeft;
    }
    if (str == "wiperight") {
        return  Transition::WipeRight;
    }
    if (str == "wipeup") {
        return  Transition::WipeUp;
    }
    if (str == "wipedown") {
        return  Transition::WipeDown;
    }
    if (str == "slideleft") {
        return  Transition::SlideLeft;
    }
    if (str == "slideright") {
        return  Transition::SlideRight;
    }
    if (str == "slideup") {
        return  Transition::SlideUp;
    }
    if (str == "slidedown") {
        return  Transition::SlideDown;
    }
    if (str == "slideawayleft") {
        return  Transition::SlideAwayLeft;
    }
    if (str == "slideawayright") {
        return  Transition::SlideAwayRight;
    }
    if (str == "slideawayup") {
        return  Transition::SlideAwayUp;
    }
    if (str == "slideawaydown") {
        return  Transition::SlideAwayDown;
    }
    if (str == "pushright") {
        return  Transition::PushRight;
    }
    if (str == "pushleft") {
        return  Transition::PushLeft;
    }
    if (str == "pushup") {
        return  Transition::PushUp;
    }
    if (str == "pushdown") {
        return  Transition::PushDown;
    }
    if (str == "irisin") {
        return  Transition::IrisIn;
    }
    if (str == "irisout") {
        return  Transition::IrisOut;
    }
    std::println(std::cerr, "invalid transition type");
    std::unreachable();
}

auto ATL::get_warper(const std::string_view &str) -> Warper {
    if (str == "pause") {
        return Warper::Pause;
    }
    if (str == "linear") {
        return Warper::Linear;
    }
    if (str == "ease") {
        return Warper::Ease;
    }
    if (str == "easein") {
        return Warper::EaseIn;
    }
    if (str == "easeout") {
        return Warper::EaseOut;
    }

    std::println(std::cerr, "invalid warper type");
    std::unreachable();
}

auto ATL::trans_str(const Transition &trans) -> std::string {
    switch (trans) {
        case Transition::Dissolve:
            return "dissolve";
        case Transition::Fade:
            return "fade";
        case Transition::Pixellate:
            return "pixellate";
        case Transition::Move:
            return "move";
        case Transition::MoveInRight:
            return "moveinright";
        case Transition::MoveInLeft:
            return "moveinleft";
        case Transition::MoveInTop:
            return "moveintop";
        case Transition::MoveInBottom:
            return "moveinbottom";
        case Transition::MoveOutRight:
            return "moveoutright";
        case Transition::MoveOutLeft:
            return "moveoutleft";
        case Transition::MoveOutTop:
            return "moveouttop";
        case Transition::MoveOutBottom:
            return "moveoutbottom";
        case Transition::Ease:
            return "ease";
        case Transition::EaseInRight:
            return "easeinright";
        case Transition::EaseInLeft:
            return "easeinleft";
        case Transition::EaseInTop:
            return "easeintop";
        case Transition::EaseInBottom:
            return "easeinbottom";
        case Transition::EaseOutRight:
            return "easeoutright";
        case Transition::EaseOutLeft:
            return "easeoutleft";
        case Transition::EaseOutTop:
            return "easeouttop";
        case Transition::EaseOutBottom:
            return "easeoutbottom";
        case Transition::ZoomIn:
            return "zoomin";
        case Transition::ZoomOut:
            return "zoomout";
        case Transition::ZoomInOut:
            return "zoominout";
        case Transition::VPunch:
            return "vpunch";
        case Transition::HPunch:
            return "hpunch";
        case Transition::Blinds:
            return "blinds";
        case Transition::Squares:
            return "squares";
        case Transition::WipeLeft:
            return "wipeleft";
        case Transition::WipeRight:
            return "wiperight";
        case Transition::WipeUp:
            return "wipeup";
        case Transition::WipeDown:
            return "wipedown";
        case Transition::SlideLeft:
            return "slideleft";
        case Transition::SlideRight:
            return "slideright";
        case Transition::SlideUp:
            return "slideup";
        case Transition::SlideDown:
            return "slidedown";
        case Transition::SlideAwayLeft:
            return "slideawayleft";
        case Transition::SlideAwayRight:
            return "slideawayright";
        case Transition::SlideAwayUp:
            return "slideawayup";
        case Transition::SlideAwayDown:
            return "slideawaydown";
        case Transition::PushRight:
            return "pushright";
        case Transition::PushLeft:
            return "pushleft";
        case Transition::PushUp:
            return "pushup";
        case Transition::PushDown:
            return "pushdown";
        case Transition::IrisIn:
            return "irisin";
        case Transition::IrisOut:
            return "irisout";
    }
    std::println(std::cerr, "invalid transition type");
    std::unreachable();
}

auto ATL::warper_str(const Warper &warper) -> std::string {
    switch (warper) {
        case Warper::Pause:
            return "pause";
        case Warper::Linear:
            return "linear";
        case Warper::Ease:
            return "ease";
        case Warper::EaseIn:
            return "easein";
        case Warper::EaseOut:
            return "easeout";
    }
    std::println(std::cerr, "invalid warper type");
    std::unreachable();
}