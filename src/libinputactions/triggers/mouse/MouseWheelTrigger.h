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

#include "MouseMotionTrigger.h"

namespace InputActions
{

class SimpleSwipeTriggerCore;

/**
 * Wheel triggers are continuous when an update action is present and a mouse button or a keyboard modifier is present. Continuous triggers begin on a scroll
 * event and end on modifier or button release. Non-continuous triggers begin and end on the same scroll event.
 */
class MouseWheelTrigger : public MouseMotionTrigger
{
public:
    MouseWheelTrigger(std::unique_ptr<SimpleSwipeTriggerCore> core);

    bool continuous() const { return m_continuous; }

protected:
    void actionAdded(TriggerAction *action) override;

private:
    bool m_continuous = false;
};

}