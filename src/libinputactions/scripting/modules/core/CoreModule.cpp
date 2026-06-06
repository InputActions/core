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
#include <QJSEngine>
#include <libinputactions/input/backends/InputBackend.h>
#include <libinputactions/variables/VariableRegistry.h>

namespace InputActions
{

CoreModule::CoreModule()
    : m_config(std::make_unique<Config>())
{
    QJSEngine::setObjectOwnership(m_config.get(), QJSEngine::CppOwnership);
}

CoreModule::~CoreModule() = default;

Config *CoreModule::config() const
{
    return m_config.get();
}

InputBackend *CoreModule::input() const
{
    return g_inputBackend.get();
}

VariableRegistry *CoreModule::variableRegistry() const
{
    return g_variableRegistry.get();
}

}