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

#include "VirtualMouseWrapper.h"
#include <libinputactions/input/backends/InputBackend.h>
#include <libinputactions/scripting/ScriptingEngine.h>

namespace InputActions
{

VirtualMouseWrapper::VirtualMouseWrapper(std::shared_ptr<InputBackend> inputBackend, ScriptingEngine &engine)
    : m_inputBackend(std::move(inputBackend))
    , m_engine(engine)
{
}

void VirtualMouseWrapper::mouseMotion(const PointF &pos)
{
    if (auto *device = virtualMouse()) {
        device->mouseMotion(pos);
    }
}

void VirtualMouseWrapper::mouseWheel(const PointF &delta)
{
    if (auto *device = virtualMouse()) {
        device->mouseWheel(delta);
    }
}

VirtualMouse *VirtualMouseWrapper::virtualMouse() const
{
    if (m_inputBackend->initialized()) {
        return m_inputBackend->virtualMouse();
    }

    m_engine.qtEngine().throwError(QString("The method can only be called after the configuration is activated."));
    return {};
}

}