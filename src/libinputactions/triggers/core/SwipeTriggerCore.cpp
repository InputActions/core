/*
    Input Actions - Input handler that executes user-defined actions
    Copyright (C) 2024-2026 Marcin Woźniak

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "SwipeTriggerCore.h"
#include <libinputactions/helpers/Math.h>
#include <libinputactions/input/devices/InputDevice.h>
#include <libinputactions/input/devices/InputDeviceProperties.h>

namespace InputActions
{

SwipeTriggerCore::SwipeTriggerCore(qreal minAngle, qreal maxAngle)
    : m_minAngle(minAngle)
    , m_maxAngle(maxAngle)
{
}

SwipeTriggerCore::SwipeTriggerCore(SwipeDirection direction)
    : m_direction(direction)
{
    switch (direction) {
        case SwipeDirection::LeftRight:
        case SwipeDirection::UpDown:
        case SwipeDirection::LeftUpRightDown:
        case SwipeDirection::LeftDownRightUp:
            m_bidirectional = true;
            break;
    }
}

bool SwipeTriggerCore::canUpdate(const TriggerUpdateEvent &event) const
{
    if (!MotionTriggerCore::canUpdate(event)) {
        return false;
    }

    const auto &castedEvent = dynamic_cast<const SwipeTriggerUpdateEvent &>(event);
    const auto angle = castedEvent.averageAngle(); // Use the average so that the trigger is not cancelled on jitter

    const auto range = angleRange(castedEvent.sender());
    return matchesAngleRange(angle, range.min, range.max) || (m_bidirectional && matchesOppositeAngleRange(angle, range.min, range.max));
}

void SwipeTriggerCore::updateActions(const std::vector<std::unique_ptr<TriggerAction>> &actions, const TriggerUpdateEvent &event) const
{
    auto newEvent = dynamic_cast<const SwipeTriggerUpdateEvent &>(event);
    const auto angle = newEvent.angle();

    // Ensure delta is always positive for normal angle range, and negative for opposite. Normal range takes priority over the opposite one in case of
    // overlapping.
    const auto range = angleRange(newEvent.sender());
    if (!matchesAngleRange(angle, range.min, range.max) && matchesOppositeAngleRange(angle, range.min, range.max)) {
        auto delta = event.delta();
        delta = {delta.accelerated() * -1, delta.unaccelerated() * -1};
        newEvent.setDelta(delta);
    }

    MotionTriggerCore::updateActions(actions, newEvent);
}

SwipeTriggerCore::AngleRange SwipeTriggerCore::angleRange(const InputDevice *device) const
{
    if (!m_direction) {
        return {m_minAngle, m_maxAngle};
    }

    const auto tolerance = device ? device->properties().swipeAngleTolerance() : DEFAULT_SWIPE_ANGLE_TOLERANGE;
    switch (m_direction.value()) {
        case SwipeDirection::Left:
            return {180 - tolerance, 180 + tolerance};
        case SwipeDirection::Right:
            return {360 - tolerance, tolerance};
        case SwipeDirection::Up:
            return {90 - tolerance, 90 + tolerance};
        case SwipeDirection::Down:
            return {270 - tolerance, 270 + tolerance};

        case SwipeDirection::LeftUp:
            return {90 + tolerance, 180 - tolerance};
        case SwipeDirection::LeftDown:
            return {180 + tolerance, 270 - tolerance};
        case SwipeDirection::RightUp:
            return {tolerance, 90 - tolerance};
        case SwipeDirection::RightDown:
            return {270 + tolerance, 360 - tolerance};

        case SwipeDirection::LeftRight:
            return {360 - tolerance, tolerance};
        case SwipeDirection::UpDown:
            return {270 - tolerance, 270 + tolerance};

        case SwipeDirection::LeftUpRightDown:
            return {270 + tolerance, 360 - tolerance};
        case SwipeDirection::LeftDownRightUp:
            return {tolerance, 90 - tolerance};

        case SwipeDirection::Any:
            return {0, 360};

        default:
            return {0, 0};
    }
}

bool SwipeTriggerCore::matchesAngleRange(qreal angle, qreal min, qreal max)
{
    if (min <= max) {
        return angle >= min && angle <= max;
    }
    return angle >= min || angle <= max;
}

bool SwipeTriggerCore::matchesOppositeAngleRange(qreal angle, qreal min, qreal max)
{
    min -= 180;
    if (min < 0) {
        min += 360;
    }

    max -= 180;
    if (max < 0) {
        max += 360;
    }

    if (min <= max) {
        return angle >= min && angle <= max;
    }
    return angle >= min || angle <= max;
}

SwipeTriggerCore::AngleRange::AngleRange(qreal min, qreal max)
    : min(min)
    , max(max)
{
}

}