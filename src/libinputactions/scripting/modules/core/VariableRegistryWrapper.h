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

#include "StoredVariableWrapper.h"
#include "VariableWrapper.h"
#include <QJSValue>
#include <QObject>
#include <libinputactions/globals.h>

namespace InputActions
{

class ScriptingEngine;
class VariableRegistry;

class VariableRegistryWrapper : public QObject
{
    Q_OBJECT

public:
    VariableRegistryWrapper(VariableRegistry &variableRegistry, ScriptingEngine &engine);

    Q_INVOKABLE bool contains(const QString &name) const;
    Q_INVOKABLE VariableWrapper *get(const QString &name) const;

    Q_INVOKABLE VariableWrapper *registerComputedVariable(const QString &name, VariableType type, QJSValue getter) const;
    Q_INVOKABLE StoredVariableWrapper *registerStoredVariable(const QString &name, VariableType type) const;
    void disableRegistration();

    /**
     * All values received from JS must be passed to this function to apply necessary conversions (e.g. int -> double).
     */
    static std::optional<QVariant> jsValueToVariant(const QJSValue &value, VariableType type);

    static std::optional<QMetaType> variableTypeToMetaType(VariableType type);
    static std::optional<VariableType> metaTypeToVariableType(const QMetaType &metaType);

private:
    static bool isVariableNameValid(const QString &name);

    VariableRegistry &m_variableRegistry;
    ScriptingEngine &m_engine;
    bool m_registrationAllowed = true;
};

}