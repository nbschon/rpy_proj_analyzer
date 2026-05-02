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

auto ATL::warper_str(const Warper& warper) -> std::string {
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

auto ATL::prop_str(const TFProp& type) -> std::string {
    switch (type) {
        case TFProp::Pos:
            return "pos";
        case TFProp::XPos:
            return "xpos";
        case TFProp::YPos:
            return "ypos";
        case TFProp::Anchor:
            return "anchor";
        case TFProp::XAnchor:
            return "xanchor";
        case TFProp::YAnchor:
            return "yanchor";
        case TFProp::Align:
            return "align";
        case TFProp::XAlign:
            return "xalign";
        case TFProp::YAlign:
            return "yalign";
        case TFProp::Offset:
            return "offset";
        case TFProp::XOffset:
            return "xoffset";
        case TFProp::YOffset:
            return "yoffset";
        case TFProp::XYCenter:
            return "xycenter";
        case TFProp::XCenter:
            return "xcenter";
        case TFProp::YCenter:
            return "ycenter";
        case TFProp::SubPixel:
            return "subpixel";
        case TFProp::Rotate:
            return "rotate";
        case TFProp::Rotate_Pad:
            return "rotate_pad";
        case TFProp::TF_Anchor:
            return "transform_anchor";
        case TFProp::Zoom:
            return "zoom";
        case TFProp::XZoom:
            return "xzoom";
        case TFProp::YZoom:
            return "yzoom";
        case TFProp::Nearest:
            return "nearest";
        case TFProp::Alpha:
            return "alpha";
        case TFProp::Additive:
            return "additive";
        case TFProp::MatrixColor:
            return "matrixcolor";
        case TFProp::Blur:
            return "blur";
        case TFProp::Around:
            return "around";
        case TFProp::Angle:
            return "angle";
        case TFProp::Radius:
            return "radius";
        case TFProp::AnchorAround:
            return "anchoraround";
        case TFProp::AnchorAngle:
            return "anchorangle";
        case TFProp::AnchorRadius:
            return "anchorradius";
        case TFProp::Crop:
            return "crop";
        case TFProp::Corner1:
            return "corner1";
        case TFProp::Corner2:
            return "corner2";
        case TFProp::XYSize:
            return "xysize";
        case TFProp::XSize:
            return "xsize";
        case TFProp::YSize:
            return "ysize";
        case TFProp::Fit:
            return "fit";
        case TFProp::XPan:
            return "xpan";
        case TFProp::YPan:
            return "ypan";
        case TFProp::XTile:
            return "xtile";
        case TFProp::YTile:
            return "ytile";
        case TFProp::Delay:
            return "delay";
        case TFProp::Events:
            return "events";
        case TFProp::FPS:
            return "fps";
        case TFProp::Show_Cancels_Hide:
            return "show_cancels_hide";
    }
    std::println(std::cerr, "invalid transformation property");
    std::unreachable();
}

auto ATL::event_str(const Event &event) -> std::string {
    switch (event) {
        case Event::Start:
            return "start";
        case Event::Show:
            return "show";
        case Event::Replace:
            return "replace";
        case Event::Hide:
            return "hide";
        case Event::Replaced:
            return "replaced";
        case Event::Hover:
            return "hover";
        case Event::Idle:
            return "idle";
        case Event::SelectedHover:
            return "selected_hover";
        case Event::SelectedIdle:
            return "selected_idle";
        case Event::Insensitive:
            return "insensitive";
        case Event::SelectedInsensitive:
            return "selected_insensitive";
    }
    std::println(std::cerr, "invalid event name");
    std::unreachable();
}
