//
// Created by Noah Schonhorn on 3/17/26.
//

#ifndef RPY_PROJ_ANALYZER_ATL_HPP
#define RPY_PROJ_ANALYZER_ATL_HPP

#include "Expr.hpp"
#include "Token.hpp"

#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

/*
 * Transformation property information borrowed from here:
 * https://www.renpy.org/doc/html/transform_properties.html
 */
enum class TFProp : std::uint8_t {
    /* Name of property    Params */
    // Positioning
    Pos,                // (pos, pos)
    XPos,               // pos
    YPos,               // pos
    Anchor,             // (pos, pos)
    XAnchor,            // pos
    YAnchor,            // pos
    Align,              // (float, float)
    XAlign,             // float
    YAlign,             // float
    Offset,             // (abs, abs)
    XOffset,            // abs
    YOffset,            // abs
    XYCenter,           // (pos, pos)
    XCenter,            // pos
    YCenter,            // pos
    SubPixel,           // bool
    // Rotation
    Rotate,             // float | None
    Rotate_Pad,         // bool
    TF_Anchor,          // bool
    // Zoom & Flip
    Zoom,               // float
    XZoom,              // float
    YZoom,              // float
    // Pixel Effects
    Nearest,            // bool
    Alpha,              // float
    Additive,           // float
    MatrixColor,        // None | Matrix | MatrixColor
    Blur,               // float | None
    // Polar Positioning
    Around,             // (pos, pos)
    Angle,              // float
    Radius,             // pos
    // Polar Positioning of Anchor
    AnchorAround,       // (pos, pos)
    AnchorAngle,        // (float)
    AnchorRadius,       // (pos)
    // Crop & Resize
    Crop,               // None | (pos, pos, pos, pos)
    Corner1,            // None | (pos, pos)
    Corner2,            // None | (pos, pos)
    XYSize,             // None | (pos, pos)
    XSize,              // None | pos
    YSize,              // None | pos
    Fit,                // None | str
    // Pan & Tile
    XPan,               // None | float
    YPan,               // None | float
    XTile,              // int
    YTile,              // int
    // Transitions
    Delay,              // bool
    Events,             // float
    // Other
    FPS,                // None | float
    Show_Cancels_Hide,  // bool
    // not adding deprecated TF properties...
};

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

enum class RotationType : std::uint8_t {
    None,
    Clockwise,
    CCWise,
};

struct ATLProperty;
struct ATLNumber;
struct ATLInterp;
struct ATLPass;
struct ATLRepeat;
struct ATLBlock;
struct ATLParallel;
struct ATLChoice;
struct ATLAnimation;
struct ATLOn;
struct ATLDisplayable;
struct ATLTransform;
struct ATLContainsInline;
struct ATLContainsBlock;
struct ATLFunction;
struct ATLTime;
struct ATLEvent;

using ATLStmt = std::variant<
    ATLProperty,
    ATLNumber,
    ATLInterp,
    ATLPass,
    ATLRepeat,
    ATLBlock,
    ATLParallel,
    ATLChoice,
    ATLAnimation,
    ATLOn,
    ATLDisplayable,
    ATLTransform,
    ATLContainsInline,
    ATLContainsBlock,
    ATLFunction,
    ATLTime,
    ATLEvent>;

struct ATLProperty {
    TFProp prop;
    std::unique_ptr<Expr> value;
};

struct ATLNumber {
    std::unique_ptr<Expr> value;
};

struct ATLInterp {
    std::unique_ptr<Expr> warper;
    std::unique_ptr<Expr> value;
    std::vector<ATLProperty> properties;
    std::vector<std::unique_ptr<Expr>> knots;
    std::optional<int> circles;
    RotationType rot_type;
};

// whoa, empty
struct ATLPass {};

struct ATLRepeat {
    std::unique_ptr<Expr> value;
};

struct ATLBlock {
    std::vector<ATLStmt> block;
};

struct ATLParallel {
    std::vector<ATLStmt> block;
};

struct ATLChoice {
    std::unique_ptr<Expr> weight;
    std::vector<ATLStmt> block;
};

struct ATLAnimation {
    std::string tag;
};

struct ATLOn {
    std::vector<std::string> events;
    std::vector<ATLStmt> block;
};

struct ATLDisplayable {
    std::unique_ptr<Expr> displayable;
    Transition trans;
};

struct ATLTransform {
    ATLProperty property;
};

struct ATLContainsInline {
    std::unique_ptr<Expr> value;
};

struct ATLContainsBlock {
    std::unique_ptr<Expr> displayable;
    std::vector<ATLStmt> block;
};

struct ATLFunction {
    std::unique_ptr<Expr> function;
};

struct ATLTime {
    std::unique_ptr<Expr> value;
};

struct ATLEvent {
    std::unique_ptr<Expr> name;
};

class ATL {
    template<class... Ts>
    struct Overload : Ts... {
        using Ts::operator()...;
    };

    template<class... Ts>
    Overload(Ts...) -> Overload<Ts...>;

    // really ugly return value but that's alright
    static auto make_inline_interp(const std::vector<Token> &tokens, unsigned &idx, std::optional<Warper> warper = std::nullopt)
        -> std::expected<std::pair<std::vector<ATLProperty>, std::vector<std::unique_ptr<Expr>>>, std::string>;

    static auto make_interp_block(const std::vector<Token> &tokens, unsigned &idx, std::optional<Warper> warper = std::nullopt)
        -> std::expected<std::vector<ATLProperty>, std::string>;

public:
    static auto make_atl_block(const std::vector<Token> &tokens, unsigned &idx, unsigned indent) -> std::vector<ATLStmt>;
    static auto get_trans(const std::string_view &str) -> Transition;
    static auto get_warper(const std::string_view &str) -> Warper;
    static auto get_prop(const std::string_view &str) -> TFProp;

    static auto trans_str(const Transition &trans) -> std::string;
    static auto warper_str(const Warper &warper) -> std::string;
    static auto prop_str(const TFProp &type) -> std::string;
};


#endif //RPY_PROJ_ANALYZER_ATL_HPP