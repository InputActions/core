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

#include "SwipeTrigger.h"
#include <libinputactions/helpers/Math.h>
#include <libinputactions/input/devices/InputDevice.h>
#include <libinputactions/input/devices/InputDeviceProperties.h>

namespace InputActions
{

SwipeTrigger::SwipeTrigger(qreal minAngle, qreal maxAngle)
    : MotionTrigger(TriggerType::Swipe)
    , m_minAngle(minAngle)
    , m_maxAngle(maxAngle)
{
}

SwipeTrigger::SwipeTrigger(SwipeTriggerDirection direction)
    : MotionTrigger(TriggerType::Swipe)
    , m_direction(direction)
{
    switch (direction) {
        case SwipeTriggerDirection::LeftRight:
        case SwipeTriggerDirection::UpDown:
        case SwipeTriggerDirection::LeftUpRightDown:
        case SwipeTriggerDirection::LeftDownRightUp:
            m_bidirectional = true;
            break;
    }
}

bool SwipeTrigger::canUpdate(const TriggerUpdateEvent &event) const
{
    if (!MotionTrigger::canUpdate(event)) {
        return false;
    }

    const auto &castedEvent = dynamic_cast<const SwipeTriggerUpdateEvent &>(event);
    const auto angle = castedEvent.averageAngle(); // Use the average so that the trigger is not cancelled on jitter

    const auto range = angleRange(castedEvent.sender());
    return matchesAngleRange(angle, range.min, range.max) || (m_bidirectional && matchesOppositeAngleRange(angle, range.min, range.max));
}

void SwipeTrigger::updateActions(const TriggerUpdateEvent &event)
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

    MotionTrigger::updateActions(newEvent);
}

SwipeTrigger::AngleRange SwipeTrigger::angleRange(const InputDevice *device) const
{
    if (!m_direction) {
        return {m_minAngle, m_maxAngle};
    }

    const auto tolerance = device ? device->properties().swipeAngleTolerance() : DEFAULT_SWIPE_ANGLE_TOLERANGE;
    switch (m_direction.value()) {
        case SwipeTriggerDirection::Left:
            return {180 - tolerance, 180 + tolerance};
        case SwipeTriggerDirection::Right:
            return {360 - tolerance, tolerance};
        case SwipeTriggerDirection::Up:
            return {90 - tolerance, 90 + tolerance};
        case SwipeTriggerDirection::Down:
            return {270 - tolerance, 270 + tolerance};

        case SwipeTriggerDirection::LeftUp:
            return {90 + tolerance, 180 - tolerance};
        case SwipeTriggerDirection::LeftDown:
            return {180 + tolerance, 270 - tolerance};
        case SwipeTriggerDirection::RightUp:
            return {tolerance, 90 - tolerance};
        case SwipeTriggerDirection::RightDown:
            return {270 + tolerance, 360 - tolerance};

        case SwipeTriggerDirection::LeftRight:
            return {360 - tolerance, tolerance};
        case SwipeTriggerDirection::UpDown:
            return {270 - tolerance, 270 + tolerance};

        case SwipeTriggerDirection::LeftUpRightDown:
            return {270 + tolerance, 360 - tolerance};
        case SwipeTriggerDirection::LeftDownRightUp:
            return {tolerance, 90 - tolerance};

        case SwipeTriggerDirection::Any:
            return {0, 360};

        default:
            return {0, 0};
    }
}

bool SwipeTrigger::matchesAngleRange(qreal angle, qreal min, qreal max)
{
    if (min <= max) {
        return angle >= min && angle <= max;
    }
    return angle >= min || angle <= max;
}

bool SwipeTrigger::matchesOppositeAngleRange(qreal angle, qreal min, qreal max)
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

SwipeTrigger::AngleRange::AngleRange(qreal min, qreal max)
    : min(min)
    , max(max)
{
}

}