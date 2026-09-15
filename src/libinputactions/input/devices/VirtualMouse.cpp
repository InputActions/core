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

#include "VirtualMouse.h"
#include <libinputactions/input/backends/InputBackend.h>

namespace InputActions
{

void VirtualMouse::mouseButton(MouseButton button, bool state)
{
    if ((state && m_pressedButtons.contains(button)) || (!state && !m_pressedButtons.contains(button))) {
        return;
    }

    g_inputBackend->setIgnoreEvents(true);
    doMouseButton(button, state);
    g_inputBackend->setIgnoreEvents(false);
    if (state) {
        m_pressedButtons.insert(button);
    } else {
        m_pressedButtons.erase(button);
    }
}

void VirtualMouse::mouseMotion(const PointF &pos)
{
    g_inputBackend->setIgnoreEvents(true);
    doMouseMotion(pos);
    g_inputBackend->setIgnoreEvents(false);
}

void VirtualMouse::mouseWheel(const PointF &delta)
{
    g_inputBackend->setIgnoreEvents(true);
    doMouseWheel(delta);
    g_inputBackend->setIgnoreEvents(false);
}

void VirtualMouse::reset()
{
    const auto buttons = m_pressedButtons;
    for (const auto button : buttons) {
        mouseButton(button, false);
    }
}

}