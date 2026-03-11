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

#include "MouseTrigger.h"
#include <libinputactions/input/MouseButton.h>
#include <libinputactions/triggers/core/TriggerCore.h>

namespace InputActions
{

MouseTrigger::MouseTrigger(TriggerType type, std::unique_ptr<TriggerCore> core)
    : Trigger(type, std::move(core))
{
}

MouseTrigger::~MouseTrigger() = default;

bool MouseTrigger::canActivate(const TriggerActivationEvent &event) const
{
    if (!m_mouseButtons.empty() && event.mouseButtons().has_value()) {
        if (m_mouseButtons.size() != event.mouseButtons()->size()
            || (m_mouseButtonsExactOrder && !std::ranges::equal(m_mouseButtons, event.mouseButtons().value()))
            || (!m_mouseButtonsExactOrder && !std::ranges::all_of(m_mouseButtons, [event](auto &button) {
                   return std::ranges::contains(event.mouseButtons().value(), button);
               }))) {
            return false;
        }
    }

    return Trigger::canActivate(event);
}

}