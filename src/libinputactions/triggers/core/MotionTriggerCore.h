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

#pragma once

#include "TriggerCore.h"
#include <libinputactions/triggers/Trigger.h>

namespace InputActions
{

class MotionTriggerUpdateEvent : public TriggerUpdateEvent
{
public:
    MotionTriggerUpdateEvent() = default;

    TriggerSpeed speed() const { return m_speed; }
    void setSpeed(TriggerSpeed value) { m_speed = value; }

    /**
     * Multiplied by the delta multiplier specified in MotionTriggerHandler.
     */
    PointDelta pointDelta() const { return m_pointDelta; }
    void setPointDelta(PointDelta value) { m_pointDelta = std::move(value); }

private:
    // Speed should be in a TriggerBeginEvent, but that's not a thing, and adding it would complicate everything.
    // Not worth it for a single property.
    TriggerSpeed m_speed = TriggerSpeed::Any;
    PointDelta m_pointDelta{};
};

/**
 * An input action that involves directionless motion.
 */
class MotionTriggerCore : public TriggerCore
{
public:
    bool hasSpeed() const;
    TriggerSpeed speed() const { return m_speed; }
    void setSpeed(TriggerSpeed value) { m_speed = value; }

    bool canUpdate(const TriggerUpdateEvent &event) const override;

private:
    TriggerSpeed m_speed = TriggerSpeed::Any;
};

}