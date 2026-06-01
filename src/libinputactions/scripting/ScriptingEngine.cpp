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

#include "ScriptingEngine.h"
#include <libinputactions/InputActionsMain.h>
#include <libinputactions/helpers/QThread.h>
#include <libinputactions/interfaces/NotificationManager.h>

namespace InputActions
{

static const std::chrono::milliseconds WATCHDOG_TIMER_TIMEOUT{2000};
static const std::chrono::milliseconds WATCHDOG_TIMER_RESET_INTERVAL{1000};

ScriptingEngine::ScriptingEngine()
    : m_watchdogTimerThread(new QThread)
    , m_watchdogTimer(new QTimer)
{
    m_engine.installExtensions(QJSEngine::ConsoleExtension);

    m_watchdogTimer->setInterval(WATCHDOG_TIMER_TIMEOUT);
    m_watchdogTimer->moveToThread(m_watchdogTimerThread);
    connect(m_watchdogTimer, &QTimer::timeout, [this]() {
        m_engine.setInterrupted(true);
        QThreadHelpers::runOnThread(QThreadHelpers::mainThread(), []() {
            g_notificationManager
                ->sendNotification("Infinite loop detected",
                                   "A script has likely entered an infinite loop and frozen the main thread. InputActions has been suspended.");
            g_inputActions->suspend();
        });
    });
    m_watchdogTimerThread->start();

    connect(&m_watchdogRestartTimer, &QTimer::timeout, this, &ScriptingEngine::onWatchdogValueRestartTimerTick);
    m_watchdogRestartTimer.setInterval(WATCHDOG_TIMER_RESET_INTERVAL);
    m_watchdogRestartTimer.start();

    registerApi();
}

ScriptingEngine::~ScriptingEngine()
{
    QMetaObject::invokeMethod(m_watchdogTimer, "stop", Qt::BlockingQueuedConnection);
    m_watchdogTimerThread->quit();
    m_watchdogTimerThread->wait();

    m_watchdogTimer->deleteLater();
    m_watchdogTimerThread->deleteLater();
}

QJSValue ScriptingEngine::evaluate(const QString &script)
{
    return m_engine.evaluate(script);
}

void ScriptingEngine::registerApi()
{
    m_globalObject.emplace();
    m_engine.globalObject().setProperty("ia", m_engine.newQObject(&m_globalObject.value()));
}

void ScriptingEngine::onWatchdogValueRestartTimerTick()
{
    QMetaObject::invokeMethod(m_watchdogTimer, "start", Qt::QueuedConnection);
}

}