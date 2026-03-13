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

#include "DirectionalMotionTriggerCore.h"

namespace InputActions
{

DirectionalMotionTriggerCore::DirectionalMotionTriggerCore(TriggerDirection direction)
    : m_direction(direction)
{
}

bool DirectionalMotionTriggerCore::canUpdate(const TriggerUpdateEvent &event) const
{
    if (!MotionTriggerCore::canUpdate(event)) {
        return false;
    }

    const auto &castedEvent = dynamic_cast<const DirectionalMotionTriggerUpdateEvent &>(event);
    return m_direction & castedEvent.direction();
}

void DirectionalMotionTriggerCore::updateActions(const std::vector<std::unique_ptr<TriggerAction>> &actions, const TriggerUpdateEvent &event) const
{
    auto newEvent = dynamic_cast<const DirectionalMotionTriggerUpdateEvent &>(event);

    // Ensure delta is always positive for single-directional gestures, it makes intervals easier to use.
    static std::vector<TriggerDirection> negativeDirections = {
        static_cast<TriggerDirection>(PinchDirection::In),
        static_cast<TriggerDirection>(RotateDirection::Counterclockwise),
        static_cast<TriggerDirection>(SimpleSwipeDirection::Left),
        static_cast<TriggerDirection>(SimpleSwipeDirection::Up),
    };
    if ((m_direction & (m_direction - 1)) == 0 && std::find(negativeDirections.begin(), negativeDirections.end(), m_direction) != negativeDirections.end()) {
        auto delta = event.delta();
        delta = {delta.accelerated() * -1, delta.unaccelerated() * -1};
        newEvent.setDelta(delta);
    }

    MotionTriggerCore::updateActions(actions, newEvent);
}

PinchTriggerCore::PinchTriggerCore(PinchDirection direction)
    : DirectionalMotionTriggerCore(static_cast<TriggerDirection>(direction))
{
}

RotateTriggerCore::RotateTriggerCore(RotateDirection direction)
    : DirectionalMotionTriggerCore(static_cast<TriggerDirection>(direction))
{
}

SimpleSwipeTriggerCore::SimpleSwipeTriggerCore(SimpleSwipeDirection direction)
    : DirectionalMotionTriggerCore(static_cast<TriggerDirection>(direction))
{
}

}