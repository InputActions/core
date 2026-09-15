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

#include "StoredVariableWrapper.h"
#include "VariableRegistryWrapper.h"
#include <libinputactions/scripting/ScriptingEngine.h>
#include <libinputactions/variables/StoredVariable.h>

namespace InputActions
{

StoredVariableWrapper::StoredVariableWrapper(StoredVariable &variable, ScriptingEngine &engine)
    : VariableWrapper(variable, engine)
    , m_variable(variable)
    , m_engine(engine)
{
}

void StoredVariableWrapper::setValue(const QJSValue &value) const
{
    const auto variant = VariableRegistryWrapper::jsValueToVariant(value, type());
    if (!variant) {
        m_engine.ensureEngine().throwError(QString("New value does not match the variable's type."));
        return;
    }

    m_variable.setValue(variant.value());
}

}