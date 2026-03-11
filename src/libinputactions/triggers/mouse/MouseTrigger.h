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

#include <libinputactions/triggers/Trigger.h>

namespace InputActions
{

class MouseButton;
class TriggerCore;

class MouseTrigger : public Trigger
{
public:
    ~MouseTrigger() override;

    /**
     * Mouse buttons that must be pressed before and during the trigger.
     */
    const std::vector<MouseButton> &mouseButtons() const { return m_mouseButtons; }
    void setMouseButtons(std::vector<MouseButton> value) { m_mouseButtons = std::move(value); }

    /**
     * Whether mouse buttons must be pressed in order as specified.
     */
    bool mouseButtonsExactOrder() const { return m_mouseButtonsExactOrder; }
    void setMouseButtonsExactOrder(bool value) { m_mouseButtonsExactOrder = value; }

    bool canActivate(const TriggerActivationEvent &event) const override;

protected:
    MouseTrigger(TriggerType type, std::unique_ptr<TriggerCore> core);

private:
    std::vector<MouseButton> m_mouseButtons;
    bool m_mouseButtonsExactOrder{};
};

}