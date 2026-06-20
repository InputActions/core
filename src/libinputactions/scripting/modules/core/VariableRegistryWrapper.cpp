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

#include "VariableRegistryWrapper.h"
#include <QRegularExpression>
#include <libinputactions/scripting/ScriptingEngine.h>
#include <libinputactions/variables/ComputedVariable.h>
#include <libinputactions/variables/StoredVariable.h>
#include <libinputactions/variables/VariableRegistry.h>

namespace InputActions
{

static const std::map<VariableType, QMetaType> VARIABLE_TYPES{
    {VariableType::Boolean, QMetaType::fromType<bool>()},
    {VariableType::CursorShape, QMetaType::fromType<CursorShape>()},
    {VariableType::KeyboardModifiers, QMetaType::fromType<Qt::KeyboardModifiers>()},
    {VariableType::Number, QMetaType::fromType<double>()},
    {VariableType::Point, QMetaType::fromType<PointF>()},
    {VariableType::String, QMetaType::fromType<QString>()},
};

VariableRegistryWrapper::VariableRegistryWrapper(VariableRegistry &variableRegistry, ScriptingEngine &engine)
    : m_variableRegistry(variableRegistry)
    , m_engine(engine)
{
}

bool VariableRegistryWrapper::contains(const QString &name) const
{
    return m_variableRegistry.variable(name);
}

VariableWrapper *VariableRegistryWrapper::get(const QString &name) const
{
    auto *variable = m_variableRegistry.variable(name);
    if (!variable) {
        m_engine.ensureEngine().throwError(QString("Variable '%1' does not exist.").arg(name));
        return {};
    }

    return new VariableWrapper(*variable, m_engine);
}

VariableWrapper *VariableRegistryWrapper::registerComputedVariable(const QString &name, VariableType type, QJSValue getter) const
{
    if (!m_registrationAllowed) {
        m_engine.ensureEngine().throwError(QString("Variables can only be registered before the configuration is loaded."));
        return {};
    } else if (contains(name)) {
        m_engine.ensureEngine().throwError(QString("Variable '%1' already exists.").arg(name));
        return {};
    } else if (!isVariableNameValid(name)) {
        m_engine.ensureEngine().throwError(QString("Invalid variable name."));
        return {};
    } else if (!getter.isCallable()) {
        m_engine.ensureEngine().throwError(QString("Getter is not callable."));
        return {};
    }

    const auto metaType = variableTypeToMetaType(type);
    if (!metaType) {
        m_engine.ensureEngine().throwError(QJSValue::RangeError, "Invalid variable type.");
        return {};
    }

    const auto getterWrapper = [this, name, type, getter = std::move(getter)](QVariant &value) {
        const auto returnedValue = m_engine.call(getter);
        const auto variant = jsValueToVariant(returnedValue, type);

        if (!variant) {
            qWarning(INPUTACTIONS_SCRIPTING).noquote().nospace() << QString("Getter function for variable '%1' returned a value of the wrong type.").arg(name);
            return;
        }
        value = variant.value();
    };
    auto variable = std::make_unique<ComputedVariable>(metaType.value(), getterWrapper);
    auto *variableWrapper = new VariableWrapper(*variable.get(), m_engine);
    m_variableRegistry.registerVariable(name, std::move(variable));
    return variableWrapper;
}

StoredVariableWrapper *VariableRegistryWrapper::registerStoredVariable(const QString &name, VariableType type) const
{
    if (!m_registrationAllowed) {
        m_engine.ensureEngine().throwError(QString("Variables can only be registered before the configuration is loaded."));
        return {};
    } else if (contains(name)) {
        m_engine.ensureEngine().throwError(QString("Variable '%1' already exists.").arg(name));
        return {};
    } else if (!isVariableNameValid(name)) {
        m_engine.ensureEngine().throwError(QString("Invalid variable name."));
        return {};
    }

    const auto metaType = variableTypeToMetaType(type);
    if (!metaType) {
        m_engine.ensureEngine().throwError(QJSValue::RangeError, "Invalid variable type.");
        return {};
    }

    auto variable = std::make_unique<StoredVariable>(metaType.value());
    auto *variableWrapper = new StoredVariableWrapper(*variable.get(), m_engine);
    m_variableRegistry.registerVariable(name, std::move(variable));
    return variableWrapper;
}

std::optional<QVariant> VariableRegistryWrapper::jsValueToVariant(const QJSValue &value, VariableType type)
{
    if (value.isUndefined()) {
        return {};
    }

    auto variant = value.toVariant();
    if (variant.isNull()) {
        return variant;
    }

    if (!variant.convert(variableTypeToMetaType(type).value())) {
        return {};
    }
    return variant;
}

std::optional<QMetaType> VariableRegistryWrapper::variableTypeToMetaType(VariableType type)
{
    if (!VARIABLE_TYPES.contains(type)) {
        return {};
    }
    return VARIABLE_TYPES.at(type);
}

std::optional<VariableType> VariableRegistryWrapper::metaTypeToVariableType(const QMetaType &metaType)
{
    for (const auto &[key, value] : VARIABLE_TYPES) {
        if (value == metaType) {
            return key;
        }
    }
    return {};
}

bool VariableRegistryWrapper::isVariableNameValid(const QString &name)
{
    static const QRegularExpression validationRegex("^[a-zA-Z_]+[a-zA-Z0-9_]*$");
    return validationRegex.match(name).hasMatch();
}

void VariableRegistryWrapper::disableRegistration()
{
    m_registrationAllowed = false;
}

}