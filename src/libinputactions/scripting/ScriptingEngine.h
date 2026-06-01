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

#include "GlobalObject.h"
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

    QJSValue evaluate(const QString &script);

private slots:
    void onWatchdogValueRestartTimerTick();

private:
    /**
     * Returns an instance of the engine. Initializes the engine if it has not been initialized yet.
     */
    QJSEngine &engine();
    void initialize();
    void initializeWatchdog();

    std::optional<QJSEngine> m_engine;
    std::optional<GlobalObject> m_globalObject;

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