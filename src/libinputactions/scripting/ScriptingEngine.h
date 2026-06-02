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

#include "FunctionWrapper.h"
#include <QJSEngine>
#include <QObject>
#include <memory>

namespace InputActions
{

/**
 * Lazily initializated.
 */
class ScriptingEngine : public QObject
{
    Q_OBJECT

public:
    ScriptingEngine() = default;
    ~ScriptingEngine() override;

    /**
     * Same as QJSEngine::evaluate but with error logging.
     */
    QJSValue evaluate(const QString &script);

    /**
     * Same as QJSValue::call but with error logging.
     */
    QJSValue call(const QJSValue &function, const QJSValueList &args = {}) const;

    QString errorToString(const QJSValue &error) const;
    void logError(const QJSValue &error) const;

    template<typename TReturn, typename... TArgs, typename TFunction>
    QJSValue newFunction(TFunction &&function)
    {
        auto &engine = ensureEngine();
        auto *wrapper = FunctionWrapper::create<TReturn, TArgs...>(&engine, std::forward<TFunction>(function));
        return evaluate(QString("obj => (...args) => obj.call(args);")).call({engine.newQObject(wrapper)});
    }

private slots:
    void onWatchdogValueRestartTimerTick();

private:
    /**
     * Returns an instance of the engine. Initializes the engine if it has not been initialized yet.
     */
    QJSEngine &ensureEngine();
    void initialize();
    void initializeWatchdog();

    void registerBuiltinModule(const QString &name, QJSValue value);

    std::optional<QJSEngine> m_engine;
    std::map<QString, QJSValue> m_builtinModules;

    QThread *m_watchdogTimerThread{};
    QTimer *m_watchdogTimer{};
    QTimer m_watchdogRestartTimer;
};

/**
 * Do not use this instance during the creation of a configuration, use ConfigLoader::futureScriptingEngine instead.
 *
 * A new instance is created on each config activation.
 */
inline std::unique_ptr<ScriptingEngine> g_scriptingEngine;

}