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

#include "ComputedVariable.h"
#include "StoredVariable.h"
#include "VariableWrapper.h"
#include <QPointF>
#include <QProcess>
#include <QString>
#include <map>
#include <memory>

Q_DECLARE_LOGGING_CATEGORY(INPUTACTIONS_VARIABLE_REGISTRY)

namespace InputActions
{

static const uint8_t FINGER_VARIABLE_COUNT = 5;
static const uint8_t REGEX_MATCH_VARIABLE_COUNT = 5;

class PointerPositionGetter;
class Variable;
class WindowProvider;

template<typename T>
struct VariableInfo
{
    QString name;

    operator QString() const { return name; }
};

struct BuiltinVariables
{
    inline static const VariableInfo<QString> DeviceName{QStringLiteral("device_name")};
    inline static const VariableInfo<qreal> Fingers{QStringLiteral("fingers")};
    inline static const VariableInfo<Qt::KeyboardModifiers> KeyboardModifiers{QStringLiteral("keyboard_modifiers")};
    inline static const VariableInfo<QString> LastTriggerId{QStringLiteral("last_trigger_id")};
    inline static const VariableInfo<qreal> LastTriggerTimestamp{QStringLiteral("last_trigger_timestamp")};
    inline static const VariableInfo<QPointF> ThumbInitialPositionPercentage{QStringLiteral("thumb_initial_position_percentage")};
    inline static const VariableInfo<QPointF> ThumbPositionPercentage{QStringLiteral("thumb_position_percentage")};
    inline static const VariableInfo<bool> ThumbPresent{QStringLiteral("thumb_present")};
};

class VariableRegistry
{
public:
    VariableRegistry();
    ~VariableRegistry();

    template<typename T>
    std::optional<VariableWrapper<T>> variable(const VariableInfo<T> &info) const
    {
        return variable<T>(info.name);
    }

    /**
     * @return A statically-typed wrapper for the specified variable, nullptr if not found or type doesn't match.
     */
    template<typename T>
    std::optional<VariableWrapper<T>> variable(const QString &name) const
    {
        auto *result = variable(name);
        if (!result) {
            return {};
        } else if (result->type().id() != qMetaTypeId<T>()) {
            qCWarning(INPUTACTIONS_VARIABLE_REGISTRY).noquote()
                << QString("VariableRegistry::variable<T> called with the wrong type (variable: %1, type: %2").arg(result->type().name(), typeid(T).name());
            return {};
        }

        return VariableWrapper<T>(result);
    }

    bool contains(const QString &name) const;
    /**
     * @return The variable with the specified name or nullptr if not found.
     */
    Variable *variable(const QString &name) const;

    Variable *registerVariable(const QString &name, std::unique_ptr<Variable> variable, bool hidden = false);
    template<typename T>
    VariableWrapper<T> registerStored(const QString &name, bool hidden = false)
    {
        return {registerVariable(name, std::make_unique<StoredVariable>(QMetaType::fromType<T>()), hidden)};
    }
    template<typename T>
    void registerStored(const VariableInfo<T> &variable, bool hidden = false)
    {
        registerStored<T>(variable.name, hidden);
    }
    template<typename T>
    void registerComputed(const QString &name, const std::function<void(std::optional<T> &value)> getter, bool hidden = false)
    {
        const std::function<void(QVariant & value)> variantGetter = [getter](QVariant &value) {
            std::optional<T> optValue;
            getter(optValue);
            if (optValue.has_value()) {
                value = QVariant::fromValue(optValue.value());
            }
        };
        registerVariable(name, std::make_unique<ComputedVariable>(QMetaType::fromType<T>(), variantGetter), hidden);
    }

    void registerAlias(const QString &variable, const QString &alias);

    /**
     * Compiles a set of extra environment variables for the specified command.
     *
     * The environment variable will only be set if the variable has a value. For boolean variables, the environment variable will be set to 1 if true.
     */
    std::map<QString, QString> extraProcessEnvironment(const QString &command) const;

    std::map<QString, const Variable *> variables() const;

private:
    std::map<QString, std::unique_ptr<Variable>> m_variables;
    std::map<QString, QString> m_variableAliases;
};

inline std::shared_ptr<VariableRegistry> g_variableRegistry;

}