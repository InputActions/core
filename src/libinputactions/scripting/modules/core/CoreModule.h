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

#include "Config.h"
#include "input/InputBackendWrapper.h"
#include "variables/VariableRegistryWrapper.h"
#include <libinputactions/scripting/modules/Module.h>

namespace InputActions
{

class InputBackend;

class CoreModule : public Module
{
    Q_OBJECT

    Q_PROPERTY(Config *config READ config)
    Q_PROPERTY(InputBackendWrapper *input READ input)
    Q_PROPERTY(VariableRegistryWrapper *variableRegistry READ variableRegistry)

public:
    CoreModule(std::shared_ptr<InputBackend> inputBackend, std::shared_ptr<VariableRegistry> variableRegistry, ScriptingEngine &engine);

    Config *config() { return &m_config; }
    InputBackendWrapper *input() { return &m_inputBackend; }
    VariableRegistryWrapper *variableRegistry() { return &m_variableRegistry; }

    void initialize(QJSValue &self) override;

private:
    Config m_config;
    InputBackendWrapper m_inputBackend;
    VariableRegistryWrapper m_variableRegistry;
};

}