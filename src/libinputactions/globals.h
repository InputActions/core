#pragma once

#include <QLoggingCategory>
#include <QObject>
#include <QSizeF>
#include <libinputactions/PointF.h>

Q_DECLARE_LOGGING_CATEGORY(INPUTACTIONS)

namespace InputActions
{
Q_NAMESPACE

enum class ComparisonOperator
{
    EqualTo,
    NotEqualTo,

    // List (right only)
    OneOf,

    // Number
    GreaterThan,
    GreaterThanOrEqual,
    LessThan,
    LessThanOrEqual,
    Between,

    // String
    Contains,
    Regex
};

// https://wayland.app/protocols/cursor-shape-v1#wp_cursor_shape_device_v1:enum:shape
enum class CursorShape
{
    Default,
    ContextMenu,
    Help,
    Pointer,
    Progress,
    Wait,
    Cell,
    Crosshair,
    Text,
    VerticalText,
    Alias,
    Copy,
    Move,
    NoDrop,
    NotAllowed,
    Grab,
    Grabbing,
    EResize,
    NResize,
    NEResize,
    NWResize,
    SResize,
    SEResize,
    SWResize,
    WResize,
    EWResize,
    NSResize,
    NESWResize,
    NWSEResize,
    ColResize,
    RowResize,
    AllScroll,
    ZoomIn,
    ZoomOut,
    DndAsk,
    AllResize,

    // KWin https://invent.kde.org/plasma/kwin/-/blob/d36646652272d5793eb07498db2d4e45109536fb/src/cursor.cpp#L585
    UpArrow,
};
Q_ENUM_NS(CursorShape)

enum class InputDeviceType
{
    Unknown = 0,
    Keyboard = 1u << 0,
    Mouse = 1u << 1,
    Touchpad = 1u << 2,
    Touchscreen = 1u << 3,
};
Q_DECLARE_FLAGS(InputDeviceTypes, InputDeviceType)
Q_DECLARE_OPERATORS_FOR_FLAGS(InputDeviceTypes)

// Currently for scripting only
enum class KeyboardModifier
{
    None = Qt::KeyboardModifier::NoModifier,
    Shift = Qt::KeyboardModifier::ShiftModifier,
    Control = Qt::KeyboardModifier::ControlModifier,
    Alt = Qt::KeyboardModifier::AltModifier,
    Meta = Qt::KeyboardModifier::MetaModifier,
};
Q_ENUM_NS(KeyboardModifier)

enum class TriggerSpeed
{
    Any,
    Slow,
    Fast
};
Q_ENUM_NS(TriggerSpeed)

enum class TriggerType : uint32_t
{
    None = 0,
    Click = 1u << 0,
    Pinch = 1u << 1,
    Press = 1u << 2,
    Rotate = 1u << 3,
    Stroke = 1u << 4,
    Swipe = 1u << 5,
    Wheel = 1u << 6,
    KeyboardShortcut = 1u << 7,
    Tap = 1u << 8,
    Hover = 1u << 9,
    Circle = 1u << 10,

    PinchRotate = Pinch | Rotate,
    /**
     * Any trigger that relies on the motion of a single point. This also includes touch device triggers where all fingers are moving in the same direction.
     */
    SinglePointMotion = Circle | Stroke | Swipe,

    All = UINT32_MAX
};
Q_ENUM_NS(TriggerType)
Q_DECLARE_FLAGS(TriggerTypes, TriggerType)
Q_DECLARE_OPERATORS_FOR_FLAGS(TriggerTypes)

enum class VariableType
{
    Boolean,
    CursorShape,
    KeyboardModifiers,
    Number,
    Point,
    String,
};
Q_ENUM_NS(VariableType)

static inline TriggerType operator~(TriggerType value)
{
    return TriggerType(~static_cast<uint32_t>(value));
}

static inline PointF operator/(const PointF &point, const QSizeF &size)
{
    return PointF(point.x() / size.width(), point.y() / size.height());
}

template<class... Ts>
struct overloads : Ts...
{
    using Ts::operator()...;
};

}