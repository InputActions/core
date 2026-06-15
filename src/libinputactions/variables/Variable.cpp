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

#include "Variable.h"
#include <libinputactions/scripting/ScriptingEngine.h>

namespace InputActions
{

Variable::Variable(QMetaType type)
    : m_type(std::move(type))
    , m_operations(VariableOperationsBase::create(this))
{
    QJSEngine::setObjectOwnership(this, QJSEngine::CppOwnership);
}

const VariableOperationsBase *Variable::operations() const
{
    return m_operations.get();
}

const QMetaType &Variable::type() const
{
    return m_type;
}

QJSValue Variable::scriptValue() const
{
    return g_scriptingEngine->ensureEngine().toScriptValue(value());
}

}