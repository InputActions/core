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

#include "CoreModule.h"
#include "Config.h"
#include "VariableRegistryWrapper.h"
#include <QJSEngine>
#include <libinputactions/input/backends/InputBackend.h>
#include <libinputactions/scripting/ScriptingEngine.h>

namespace InputActions
{

CoreModule::CoreModule(ScriptingEngine &engine, VariableRegistry &variableRegistry)
    : Module(engine)
    , m_variableRegistry(variableRegistry, engine)
{
    QJSEngine::setObjectOwnership(&m_config, QJSEngine::CppOwnership);
    QJSEngine::setObjectOwnership(&m_variableRegistry, QJSEngine::CppOwnership);
}

void CoreModule::initialize(QJSValue &self)
{
    self.setProperty("KeyboardModifier", engine().newEnum<KeyboardModifier>());
    self.setProperty("VariableType", engine().newEnum<VariableType>());
}

Config *CoreModule::config()
{
    return &m_config;
}

InputBackend *CoreModule::input() const
{
    return g_inputBackend.get();
}

VariableRegistryWrapper *CoreModule::variableRegistry()
{
    return &m_variableRegistry;
}

}