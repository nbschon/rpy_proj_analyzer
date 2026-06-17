//
// Created by Noah Schonhorn on 3/17/26.
//

#include "ATL.hpp"

#include <iostream>
#include <print>
#include <utility>

#include "Lexer.hpp"

auto ATL::make_inline_interp(Lexer& lexer, std::optional<Warper> warper)
    -> std::expected<std::pair<std::vector<ATLProperty>, std::vector<std::unique_ptr<Expr>>>, std::string> {
    std::unique_ptr<Expr> expr = nullptr;

    std::vector<ATLProperty> properties;
    while (lexer.curr_is<TokATLProperty>()) {
        const auto prop = lexer.expect<TokATLProperty>();
        if (auto e = try_get_expr(lexer)) {
            properties.emplace_back(ATLProperty{.prop = prop->type, .value = std::move(*e)});
        } else {
            return std::unexpected(std::move(e.error()));
        }
    }

    std::string err_msg;
    std::vector<std::unique_ptr<Expr>> knots;
    auto type = RotationType::None;
    std::unique_ptr<Expr> circles;

    while (lexer.curr_is_not<TokNewline>()) {
        const auto &token = lexer.curr();
        std::visit(Overload {
            [&](const TokATLKnot &t) -> void {
                while (lexer.curr_is<TokATLKnot>()) {
                    ++lexer;
                    if (auto e = try_get_expr(lexer)) {
                        knots.emplace_back(std::move(*e));
                    } else {
                        err_msg = std::move(e.error());
                    }
                }
            },
            [&](const TokATLClockwise &) -> void {
                ++lexer;
                type = RotationType::Clockwise;
            },
            [&](const TokATLCCWise &) -> void {
                ++lexer;
                type = RotationType::CCWise;
            },
            [&](const TokATLCircles &t) -> void {
                ++lexer;
                if (auto e = try_get_expr(lexer)) {
                    circles = std::move(*e);
                } else {
                    err_msg = std::move(e.error());
                }
            },
            [&]<typename U>(U&& other) -> void {
                using To = std::decay_t<U>;
                err_msg = std::format("unexpected token {} at {}", tok_name<To>(), tok_pos(other));
            },
        }, token);

        if (!err_msg.empty()) {
            return std::unexpected(std::move(err_msg));
        }
        ++lexer;
    }

    return std::make_pair(std::move(properties), std::move(knots));
}

auto ATL::make_interp_block(Lexer& lexer, std::optional<Warper> warper)
    -> std::expected<std::vector<ATLProperty>, std::string> {
    std::vector<ATLProperty> properties;
    while (lexer.curr_is<TokATLProperty>()) {
        const auto prop = lexer.expect<TokATLProperty>();
        if (auto expr = try_get_expr(lexer)) {
            properties.emplace_back(prop->type, std::move(*expr));
        } else {
            return std::unexpected(std::move(expr.error()));
        }

        while (lexer.curr_is<TokNewline, TokTab>()) {
            ++lexer;
        }
    }
    return properties;
}

auto ATL::make_atl_block(Lexer& lexer, unsigned indent) -> std::vector<ATLStmt> {
    std::vector<ATLStmt> statements;

    while (tok_indent(lexer.curr()) > indent && lexer.has_more()) {
        const auto &token = lexer.curr();
        std::visit(Overload {
            [&](const TokATLProperty& t) {
                ++lexer;
                if (auto expr = try_get_expr(lexer)) {
                    statements.emplace_back(ATLProperty{.prop=t.type, .value=std::move(*expr)});
                } else {
                    std::println(std::cerr, "{}", expr.error());
                }
            },
            [&](const TokFloatLit& t) {
                ++lexer;
                statements.emplace_back(ATLNumber{std::make_unique<ExprLit>(t.value)});
            },
            [&](const TokIntLit& t) {
                ++lexer;
                statements.emplace_back(ATLNumber{std::make_unique<ExprLit>(t.value)});
            },
            [&](const TokATLPause& t) {
                ++lexer;
                if (auto expr = try_get_expr(lexer)) {
                    statements.emplace_back(ATLNumber{std::move(*expr)});
                } else {
                    std::println(std::cerr, "{}", expr.error());
                    return;
                }
            },
            [&](const TokATLWarper& t) {
                ++lexer;
                std::unique_ptr<Expr> expr = nullptr;
                if (auto e = try_get_expr(lexer)) {
                    expr = std::move(*e);
                } else {
                    std::println(std::cerr, "{}", e.error());
                    return;
                }

                if (lexer.curr_is<TokColon>()) {
                    if (auto props_knots = make_inline_interp(lexer, t.warper)) {
                        statements.emplace_back(ATLInterp{
                            .warper = std::make_unique<ExprLit>(warper_str(t.warper)),
                            .value = std::move(expr),
                            .properties = std::move(props_knots->first),
                            .knots = std::move(props_knots->second),
                        });
                    } else {
                        std::println(std::cerr, "{}", props_knots.error());
                    }
                } else if (lexer.curr_is<TokATLProperty>()) {
                    if (auto prop_block = make_interp_block(lexer, t.warper)) {
                        statements.emplace_back(ATLInterp{
                            .warper = std::make_unique<ExprLit>(warper_str(t.warper)),
                            .value = std::move(expr),
                            .properties = std::move(*prop_block),
                            .knots = {}
                        });
                    } else {
                        std::println(std::cerr, "{}", prop_block.error());
                    }
                } else {
                    std::println(std::cerr, "invalid ATL Interpolation statement on line {}", t.line);
                }
            },
            [&](const TokATLWarp& t) {
                ++lexer;
                std::unique_ptr<Expr> warper_func = nullptr;
                if (auto expr = try_get_expr(lexer)) {
                    warper_func = std::move(*expr);
                } else {
                    std::println(std::cerr, "{}", expr.error());
                    return;
                }

                std::unique_ptr<Expr> expr = nullptr;
                if (auto e = try_get_expr(lexer)) {
                    expr = std::move(*e);
                } else {
                    std::println(std::cerr, "{}", e.error());
                    return;
                }

                if (auto props_knots = make_inline_interp(lexer)) {
                    statements.emplace_back(ATLInterp{
                        .warper = std::move(warper_func),
                        .value = std::move(expr),
                        .properties = std::move(props_knots->first),
                        .knots = std::move(props_knots->second),
                    });
                } else {
                    std::println(std::cerr, "{}", props_knots.error());
                }
            },
            [&](const TokPass& t) {
                ++lexer;
                statements.emplace_back(ATLPass{});
            },
            [&](const TokATLRepeat& t) {
                ++lexer;
                if (auto expr = try_get_expr(lexer)) {
                    statements.emplace_back(ATLRepeat{std::move(*expr)});
                } else {
                    std::println(std::cerr, "{}", expr.error());
                }
            },
            [&](const TokATLBlock& t) {
                ++lexer;
                auto block = make_atl_block(lexer, indent + 1);
                statements.emplace_back(ATLBlock{std::move(block)});
            },
            [&](const TokATLParallel& t) {
                ++lexer;
                auto block = make_atl_block(lexer, indent + 1);
                statements.emplace_back(ATLParallel{std::move(block)});
            },
            [&](const TokATLChoice &t) {
                ++lexer;
                std::unique_ptr<Expr> weight = nullptr;
                if (auto e = try_get_expr(lexer)) {
                    weight = std::move(*e);
                    if (lexer.curr_is<TokColon>()) {
                        ++lexer;
                    } else {
                        std::println(std::cerr, "missing colon in choice statement at {}", tok_pos(t));
                        return;
                    }
                }
                auto block = make_atl_block(lexer, indent + 1);
                statements.emplace_back(ATLChoice{.weight=std::move(weight), .block=std::move(block)});
            },
            [&](const TokATLAnimation &t) {
                ++lexer;
                if (statements.empty()) {
                    statements.emplace_back(ATLAnimation{});
                } else {
                    std::println(std::cerr, "animation statement not first in ATL block at {}", tok_pos(t));
                }
            },
            [&](const TokATLOn &t) {
                ++lexer;
                std::vector<Event> events;
                while (lexer.curr_is_not<TokColon>()) {
                    if (lexer.expect<TokShow>()) {
                        events.emplace_back(Event::Show);
                    } else if (lexer.expect<TokHide>()) {
                        events.emplace_back(Event::Hide);
                    } else if (auto event = lexer.expect<TokATLEvent>()) {
                        events.emplace_back(event->event);
                    } else if (lexer.expect<TokNewline>()) {
                        std::println(std::cerr, "on statement missing colon in ATL block at {}", tok_pos(t));
                    }

                    if (lexer.curr_is<TokComma>()) {
                        ++lexer;
                    } else if (lexer.curr_is<TokColon>()) {
                        ++lexer;
                        break;
                    }
                }
                auto block = make_atl_block(lexer, indent + 1);
                statements.emplace_back(ATLOn{.events=std::move(events), .block=std::move(block)});
            },
            // TODO: displayable
            // TODO: transform
            [&](const TokATLContains) {
                ++lexer;
                if (lexer.expect<TokColon>()) {
                    auto block = make_atl_block(lexer, indent + 1);
                    statements.emplace_back(ATLContainsBlock{std::move(block)});
                } else if (auto expr = try_get_expr(lexer)) {
                    statements.emplace_back(ATLContainsInline{std::move(*expr)});
                } else {
                    std::println(std::cerr, "{}", lexer.multi_tok_error<TokColon>({"valid Expression"}));
                }
            },
            [&](const TokATLFunction) {
                ++lexer;
                if (auto expr = try_get_expr(lexer)) {
                    statements.emplace_back(ATLFunction{std::move(*expr)});
                } else {
                    std::println(std::cerr, "{}", expr.error());
                }
            },
            [&]<typename U>(U&& other) {
                if (auto expr = try_get_expr(lexer)) {
                    statements.emplace_back(ATLNumber{std::move(*expr)});
                    return;
                }
                using To = std::decay_t<U>;
                static_assert(std::is_base_of_v<Tok, To>, "expected derived from base Tok");
                std::string msg = std::format("unexpected token {} at {}", tok_name<To>(), tok_pos(other));
                std::println(std::cerr, "{}", msg);
            },
        }, token);
    }

    return statements;
}

auto ATL::get_trans(const std::string_view& str) -> Transition {
    if (str == "dissolve") {
        return Transition::Dissolve;
    }
    if (str == "fade") {
        return Transition::Fade;
    }
    if (str == "pixellate") {
        return Transition::Pixellate;
    }
    if (str == "move") {
        return Transition::Move;
    }
    if (str == "moveinright") {
        return Transition::MoveInRight;
    }
    if (str == "moveinleft") {
        return Transition::MoveInLeft;
    }
    if (str == "moveintop") {
        return Transition::MoveInTop;
    }
    if (str == "moveinbottom") {
        return Transition::MoveInBottom;
    }
    if (str == "moveoutright") {
        return Transition::MoveOutRight;
    }
    if (str == "moveoutleft") {
        return Transition::MoveOutLeft;
    }
    if (str == "moveouttop") {
        return Transition::MoveOutTop;
    }
    if (str == "moveoutbottom") {
        return Transition::MoveOutBottom;
    }
    if (str == "ease") {
        return Transition::Ease;
    }
    if (str == "easeinright") {
        return Transition::EaseInRight;
    }
    if (str == "easeinleft") {
        return Transition::EaseInLeft;
    }
    if (str == "easeintop") {
        return Transition::EaseInTop;
    }
    if (str == "easeinbottom") {
        return Transition::EaseInBottom;
    }
    if (str == "easeoutright") {
        return Transition::EaseOutRight;
    }
    if (str == "easeoutleft") {
        return Transition::EaseOutLeft;
    }
    if (str == "easeouttop") {
        return Transition::EaseOutTop;
    }
    if (str == "easeoutbottom") {
        return Transition::EaseOutBottom;
    }
    if (str == "zoomin") {
        return Transition::ZoomIn;
    }
    if (str == "zoomout") {
        return Transition::ZoomOut;
    }
    if (str == "zoominout") {
        return Transition::ZoomInOut;
    }
    if (str == "vpunch") {
        return Transition::VPunch;
    }
    if (str == "hpunch") {
        return Transition::HPunch;
    }
    if (str == "blinds") {
        return Transition::Blinds;
    }
    if (str == "squares") {
        return Transition::Squares;
    }
    if (str == "wipeleft") {
        return Transition::WipeLeft;
    }
    if (str == "wiperight") {
        return Transition::WipeRight;
    }
    if (str == "wipeup") {
        return Transition::WipeUp;
    }
    if (str == "wipedown") {
        return Transition::WipeDown;
    }
    if (str == "slideleft") {
        return Transition::SlideLeft;
    }
    if (str == "slideright") {
        return Transition::SlideRight;
    }
    if (str == "slideup") {
        return Transition::SlideUp;
    }
    if (str == "slidedown") {
        return Transition::SlideDown;
    }
    if (str == "slideawayleft") {
        return Transition::SlideAwayLeft;
    }
    if (str == "slideawayright") {
        return Transition::SlideAwayRight;
    }
    if (str == "slideawayup") {
        return Transition::SlideAwayUp;
    }
    if (str == "slideawaydown") {
        return Transition::SlideAwayDown;
    }
    if (str == "pushright") {
        return Transition::PushRight;
    }
    if (str == "pushleft") {
        return Transition::PushLeft;
    }
    if (str == "pushup") {
        return Transition::PushUp;
    }
    if (str == "pushdown") {
        return Transition::PushDown;
    }
    if (str == "irisin") {
        return Transition::IrisIn;
    }
    if (str == "irisout") {
        return Transition::IrisOut;
    }
    std::println(std::cerr, "invalid transition type");
    std::unreachable();
}

auto ATL::get_prop(const std::string_view& str) -> TFProp {
    if (str == "pos") {
        return TFProp::Pos;
    }
    if (str == "xpos") {
        return TFProp::XPos;
    }
    if (str == "ypos") {
        return TFProp::YPos;
    }
    if (str == "anchor") {
        return TFProp::Anchor;
    }
    if (str == "xanchor") {
        return TFProp::XAnchor;
    }
    if (str == "yanchor") {
        return TFProp::YAnchor;
    }
    if (str == "align") {
        return TFProp::Align;
    }
    if (str == "xalign") {
        return TFProp::XAlign;
    }
    if (str == "yalign") {
        return TFProp::YAlign;
    }
    if (str == "offset") {
        return TFProp::Offset;
    }
    if (str == "xoffset") {
        return TFProp::XOffset;
    }
    if (str == "yoffset") {
        return TFProp::YOffset;
    }
    if (str == "xycenter") {
        return TFProp::XYCenter;
    }
    if (str == "xcenter") {
        return TFProp::XCenter;
    }
    if (str == "ycenter") {
        return TFProp::YCenter;
    }
    if (str == "subpixel") {
        return TFProp::SubPixel;
    }
    if (str == "rotate") {
        return TFProp::Rotate;
    }
    if (str == "rotate_pad") {
        return TFProp::Rotate_Pad;
    }
    if (str == "transform_anchor") {
        return TFProp::TF_Anchor;
    }
    if (str == "zoom") {
        return TFProp::Zoom;
    }
    if (str == "xzoom") {
        return TFProp::XZoom;
    }
    if (str == "yzoom") {
        return TFProp::YZoom;
    }
    if (str == "nearest") {
        return TFProp::Nearest;
    }
    if (str == "alpha") {
        return TFProp::Alpha;
    }
    if (str == "additive") {
        return TFProp::Additive;
    }
    if (str == "matrixcolor") {
        return TFProp::MatrixColor;
    }
    if (str == "blur") {
        return TFProp::Blur;
    }
    if (str == "around") {
        return TFProp::Around;
    }
    if (str == "angle") {
        return TFProp::Angle;
    }
    if (str == "radius") {
        return TFProp::Radius;
    }
    if (str == "anchoraround") {
        return TFProp::AnchorAround;
    }
    if (str == "anchorangle") {
        return TFProp::AnchorAngle;
    }
    if (str == "anchorradius") {
        return TFProp::AnchorRadius;
    }
    if (str == "crop") {
        return TFProp::Crop;
    }
    if (str == "corner1") {
        return TFProp::Corner1;
    }
    if (str == "corner2") {
        return TFProp::Corner2;
    }
    if (str == "xysize") {
        return TFProp::XYSize;
    }
    if (str == "xsize") {
        return TFProp::XSize;
    }
    if (str == "ysize") {
        return TFProp::YSize;
    }
    if (str == "fit") {
        return TFProp::Fit;
    }
    if (str == "xpan") {
        return TFProp::XPan;
    }
    if (str == "ypan") {
        return TFProp::YPan;
    }
    if (str == "xtile") {
        return TFProp::XTile;
    }
    if (str == "ytile") {
        return TFProp::YTile;
    }
    if (str == "delay") {
        return TFProp::Delay;
    }
    if (str == "events") {
        return TFProp::Events;
    }
    if (str == "fps") {
        return TFProp::FPS;
    }
    if (str == "show_cancels_hide") {
        return TFProp::Show_Cancels_Hide;
    }
    std::println(std::cerr, "unknown property type");
    std::unreachable();
}

auto ATL::get_warper(const std::string_view& str) -> Warper {
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

auto ATL::get_str(const ATLStmt& stmt) -> std::string {
    return std::visit(Overload {
        [&](const ATLProperty &p) {
            return std::format("property: {}, value:", prop_str(p.prop));
        },
        [&](const ATLNumber &p) {
            return std::format("number: ");
        },
        [&](const ATLInterp &p) {
            return std::format("warper: , value: ,");
        },
        [&](const ATLPass &p) {
            return std::format("pass");
        },
        [&](const ATLRepeat &p) {
            return std::format("repeat");
        },
        [&](const ATLBlock &p) {
            return std::format("block");
        },
        [&](const ATLParallel &p) {
            return std::format("parallel");
        },
        [&](const ATLChoice &p) {
            return std::format("choice");
        },
        [&](const ATLAnimation &p) {
            return std::format("animation");
        },
        [&](const ATLOn &p) {
            return std::format("on");
        },
        [&](const ATLDisplayable &p) {
            return std::format("displayable");
        },
        [&](const ATLTransform &p) {
            return std::format("transform");
        },
        [&](const ATLContainsInline &p) {
            return std::format("contains (inline)");
        },
        [&](const ATLContainsBlock &p) {
            return std::format("contains (block)");
        },
        [&](const ATLFunction &p) {
            return std::format("function");
        },
        [&](const ATLTime &p) {
            return std::format("time");
        },
        [&](const ATLEvent &p) {
            return std::format("event");
        },
    }, stmt);
}

auto ATL::trans_str(const Transition& trans) -> std::string {
    switch (trans) {
        using enum Transition;
        case Dissolve:
            return "dissolve";
        case Fade:
            return "fade";
        case Pixellate:
            return "pixellate";
        case Move:
            return "move";
        case MoveInRight:
            return "moveinright";
        case MoveInLeft:
            return "moveinleft";
        case MoveInTop:
            return "moveintop";
        case MoveInBottom:
            return "moveinbottom";
        case MoveOutRight:
            return "moveoutright";
        case MoveOutLeft:
            return "moveoutleft";
        case MoveOutTop:
            return "moveouttop";
        case MoveOutBottom:
            return "moveoutbottom";
        case Ease:
            return "ease";
        case EaseInRight:
            return "easeinright";
        case EaseInLeft:
            return "easeinleft";
        case EaseInTop:
            return "easeintop";
        case EaseInBottom:
            return "easeinbottom";
        case EaseOutRight:
            return "easeoutright";
        case EaseOutLeft:
            return "easeoutleft";
        case EaseOutTop:
            return "easeouttop";
        case EaseOutBottom:
            return "easeoutbottom";
        case ZoomIn:
            return "zoomin";
        case ZoomOut:
            return "zoomout";
        case ZoomInOut:
            return "zoominout";
        case VPunch:
            return "vpunch";
        case HPunch:
            return "hpunch";
        case Blinds:
            return "blinds";
        case Squares:
            return "squares";
        case WipeLeft:
            return "wipeleft";
        case WipeRight:
            return "wiperight";
        case WipeUp:
            return "wipeup";
        case WipeDown:
            return "wipedown";
        case SlideLeft:
            return "slideleft";
        case SlideRight:
            return "slideright";
        case SlideUp:
            return "slideup";
        case SlideDown:
            return "slidedown";
        case SlideAwayLeft:
            return "slideawayleft";
        case SlideAwayRight:
            return "slideawayright";
        case SlideAwayUp:
            return "slideawayup";
        case SlideAwayDown:
            return "slideawaydown";
        case PushRight:
            return "pushright";
        case PushLeft:
            return "pushleft";
        case PushUp:
            return "pushup";
        case PushDown:
            return "pushdown";
        case IrisIn:
            return "irisin";
        case IrisOut:
            return "irisout";
    }
    std::println(std::cerr, "invalid transition type");
    std::unreachable();
}

auto ATL::warper_str(const Warper& warper) -> std::string {
    switch (warper) {
        using enum Warper;
        case Pause:
            return "pause";
        case Linear:
            return "linear";
        case Ease:
            return "ease";
        case EaseIn:
            return "easein";
        case EaseOut:
            return "easeout";
    }
    std::println(std::cerr, "invalid warper type");
    std::unreachable();
}

auto ATL::prop_str(const TFProp& type) -> std::string {
    switch (type) {
        using enum TFProp;
        case Pos:
            return "pos";
        case XPos:
            return "xpos";
        case YPos:
            return "ypos";
        case Anchor:
            return "anchor";
        case XAnchor:
            return "xanchor";
        case YAnchor:
            return "yanchor";
        case Align:
            return "align";
        case XAlign:
            return "xalign";
        case YAlign:
            return "yalign";
        case Offset:
            return "offset";
        case XOffset:
            return "xoffset";
        case YOffset:
            return "yoffset";
        case XYCenter:
            return "xycenter";
        case XCenter:
            return "xcenter";
        case YCenter:
            return "ycenter";
        case SubPixel:
            return "subpixel";
        case Rotate:
            return "rotate";
        case Rotate_Pad:
            return "rotate_pad";
        case TF_Anchor:
            return "transform_anchor";
        case Zoom:
            return "zoom";
        case XZoom:
            return "xzoom";
        case YZoom:
            return "yzoom";
        case Nearest:
            return "nearest";
        case Alpha:
            return "alpha";
        case Additive:
            return "additive";
        case MatrixColor:
            return "matrixcolor";
        case Blur:
            return "blur";
        case Around:
            return "around";
        case Angle:
            return "angle";
        case Radius:
            return "radius";
        case AnchorAround:
            return "anchoraround";
        case AnchorAngle:
            return "anchorangle";
        case AnchorRadius:
            return "anchorradius";
        case Crop:
            return "crop";
        case Corner1:
            return "corner1";
        case Corner2:
            return "corner2";
        case XYSize:
            return "xysize";
        case XSize:
            return "xsize";
        case YSize:
            return "ysize";
        case Fit:
            return "fit";
        case XPan:
            return "xpan";
        case YPan:
            return "ypan";
        case XTile:
            return "xtile";
        case YTile:
            return "ytile";
        case Delay:
            return "delay";
        case Events:
            return "events";
        case FPS:
            return "fps";
        case Show_Cancels_Hide:
            return "show_cancels_hide";
    }
    std::println(std::cerr, "invalid transformation property");
    std::unreachable();
}

auto ATL::event_str(const Event &event) -> std::string {
    switch (event) {
        using enum Event;
        case Start:
            return "start";
        case Show:
            return "show";
        case Replace:
            return "replace";
        case Hide:
            return "hide";
        case Replaced:
            return "replaced";
        case Hover:
            return "hover";
        case Idle:
            return "idle";
        case SelectedHover:
            return "selected_hover";
        case SelectedIdle:
            return "selected_idle";
        case Insensitive:
            return "insensitive";
        case SelectedInsensitive:
            return "selected_insensitive";
    }
    std::println(std::cerr, "invalid event name");
    std::unreachable();
}
