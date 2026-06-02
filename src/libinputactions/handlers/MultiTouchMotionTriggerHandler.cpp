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

#include "MultiTouchMotionTriggerHandler.h"
#include <libinputactions/input/devices/InputDevice.h>
#include <libinputactions/input/devices/InputDeviceProperties.h>
#include <libinputactions/input/events.h>
#include <libinputactions/variables/VariableRegistry.h>

namespace InputActions
{

bool MultiTouchMotionTriggerHandler::handlePinch(qreal scale, qreal angleDelta)
{
    if (!hasActiveTriggers(TriggerType::PinchRotate)) {
        return false;
    }

    const auto scaleDelta = -(m_previousPinchScale - scale);
    m_previousPinchScale = scale;

    m_accumulatedRotateDelta += std::abs(angleDelta);
    if (m_pinchType == PinchType::Unknown) {
        if (m_accumulatedRotateDelta >= 10) {
            m_pinchType = PinchType::Rotate;
            cancelTriggers(TriggerType::Pinch);
        } else if (std::abs(1.0 - scale) >= 0.2) {
            m_pinchType = PinchType::Pinch;
            cancelTriggers(TriggerType::Rotate);
        } else {
            return true;
        }
    }

    TriggerType type{};
    uint32_t direction{};
    qreal delta{};

    switch (m_pinchType) {
        case PinchType::Pinch:
            direction = static_cast<TriggerDirection>(scale < 1 ? PinchDirection::In : PinchDirection::Out);
            delta = scaleDelta;
            type = TriggerType::Pinch;
            break;
        case PinchType::Rotate:
            direction = static_cast<TriggerDirection>(angleDelta > 0 ? RotateDirection::Clockwise : RotateDirection::Counterclockwise);
            delta = angleDelta;
            type = TriggerType::Rotate;
            break;
        case PinchType::Unknown:
            break;
    }

    TriggerSpeed speed{};
    if (!determineSpeed(type, delta, speed, direction)) {
        return true;
    }

    DirectionalMotionTriggerUpdateEvent event;
    event.setDelta(delta);
    event.setDirection(direction);
    event.setSpeed(speed);
    const auto result = updateTriggers(type, event);

    return result.block;
}

void MultiTouchMotionTriggerHandler::reset()
{
    MotionTriggerHandler::reset();
    m_previousPinchScale = 1;
    m_pinchType = PinchType::Unknown;
    m_accumulatedRotateDelta = 0;
}

}