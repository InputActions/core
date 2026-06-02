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

#include "VariableRegistry.h"
#include "Variable.h"
#include <QLoggingCategory>
#include <QRegularExpression>

Q_LOGGING_CATEGORY(INPUTACTIONS_VARIABLE_REGISTRY, "inputactions.variableregistry", QtWarningMsg)

namespace InputActions
{

VariableRegistry::VariableRegistry() = default;
VariableRegistry::~VariableRegistry() = default;

bool VariableRegistry::contains(const QString &name) const
{
    return m_variables.contains(name) || (m_variableAliases.contains(name) && m_variables.contains(m_variableAliases.at(name)));
}

Variable *VariableRegistry::variable(const QString &name) const
{
    Variable *variable{};
    if (m_variables.contains(name)) {
        variable = m_variables.at(name).get();
    } else if (m_variableAliases.contains(name) && m_variables.contains(m_variableAliases.at(name))) {
        variable = m_variables.at(m_variableAliases.at(name)).get();
    }

    if (!variable) {
        qCDebug(INPUTACTIONS_VARIABLE_REGISTRY).noquote() << QString("Variable %1 not found").arg(name);
    }
    return variable;
}

Variable *VariableRegistry::registerVariable(const QString &name, std::unique_ptr<Variable> variable, bool hidden)
{
    variable->setHidden(hidden);
    m_variables[name] = std::move(variable);

    if (m_variables[name]->type().id() == qMetaTypeId<PointF>()) {
        registerComputed<qreal>(
            name + "_x",
            [this, name](auto &value) {
                if (const auto point = this->variable<PointF>(name)->value()) {
                    value = point->x();
                }
            },
            true);
        registerComputed<qreal>(
            name + "_y",
            [this, name](auto &value) {
                if (const auto point = this->variable<PointF>(name)->value()) {
                    value = point->y();
                }
            },
            true);
    }

    return m_variables[name].get();
}

void VariableRegistry::registerAlias(const QString &name, const QString &alias)
{
    m_variableAliases[name] = alias;
}

std::map<QString, QString> VariableRegistry::extraProcessEnvironment(const QString &command) const
{
    static const QRegularExpression variableReferenceRegex("\\$([a-zA-Z0-9_])+");

    std::map<QString, QString> result;
    auto it = variableReferenceRegex.globalMatch(command);
    while (it.hasNext()) {
        const auto match = it.next();
        const auto variableName = match.captured(0).mid(1);

        if (const auto *variable = this->variable(variableName)) {
            const auto value = variable->value();
            if (value.isNull()) {
                continue;
            }

            if (variable->type().id() == qMetaTypeId<bool>()) {
                if (value.toBool()) {
                    result[variableName] = "1";
                }
                continue;
            }

            result[variableName] = variable->operations()->toString(value);
        }
    }

    return result;
}

std::map<QString, const Variable *> VariableRegistry::variables() const
{
    std::map<QString, const Variable *> variables;
    for (const auto &[name, variable] : m_variables) {
        variables[name] = variable.get();
    }
    return variables;
}

}