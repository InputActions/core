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
#include "Promise.h"
#include "modules/core/CoreModule.h"
#include "modules/fs/File.h"
#include <libinputactions/InputActionsMain.h>
#include <libinputactions/PointF.h>
#include <libinputactions/helpers/QString.h>
#include <libinputactions/helpers/QThread.h>
#include <libinputactions/interfaces/NotificationManager.h>

Q_LOGGING_CATEGORY(INPUTACTIONS_SCRIPTING, "inputactions.scripting", QtWarningMsg)

namespace InputActions
{

static const std::chrono::milliseconds WATCHDOG_TIMER_TIMEOUT{2000};
static const std::chrono::milliseconds WATCHDOG_TIMER_RESET_INTERVAL{1000};

ScriptingEngine::~ScriptingEngine()
{
    if (!m_engine) {
        return;
    }

    QMetaObject::invokeMethod(m_watchdogTimer, "stop", Qt::BlockingQueuedConnection);
    m_watchdogTimerThread->quit();
    m_watchdogTimerThread->wait();

    m_watchdogTimer->deleteLater();
    m_watchdogTimerThread->deleteLater();
}

void ScriptingEngine::initialize()
{
    m_engine.emplace();
    m_engine->installExtensions(QJSEngine::ConsoleExtension);

    initializeWatchdog();

    m_promiseFactory = m_engine->evaluate(R"(
        holder => {
            return new Promise((fulfill, reject) => {
                holder.fulfill = fulfill;
                holder.reject = reject;
            });
        }
    )");

    auto coreModule = m_engine->newQObject(new CoreModule);
    coreModule.setProperty("Point", m_engine->newQMetaObject(&PointF::staticMetaObject));
    registerBuiltinModule("inputactions/core", coreModule);

    auto fsModule = m_engine->newObject();
    fsModule.setProperty("File", m_engine->newQMetaObject(&File::staticMetaObject));
    auto file = fsModule.property("File");
    file.setProperty("readAllTextAsync", newFunction<QJSValue, QString>(&File::readAllTextAsync));
    file.setProperty("writeAllTextAsync", newFunction<QJSValue, QString, QString>(&File::writeAllTextAsync));
    registerBuiltinModule("inputactions/fs", fsModule);

    auto globalObject = m_engine->globalObject();
    globalObject.setProperty("require", newFunction<QJSValue, QString>([this](QString module) {
                                 if (m_builtinModules.contains(module)) {
                                     return m_builtinModules[module];
                                 }

                                 return m_engine->importModule(module);
                             }));
}

void ScriptingEngine::initializeWatchdog()
{
    m_watchdogTimerThread = new QThread;
    m_watchdogTimer = new QTimer;

    m_watchdogTimer->setInterval(WATCHDOG_TIMER_TIMEOUT);
    m_watchdogTimer->moveToThread(m_watchdogTimerThread);
    connect(m_watchdogTimer, &QTimer::timeout, [this]() {
        m_engine->setInterrupted(true);
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
}

void ScriptingEngine::registerBuiltinModule(const QString &name, QJSValue value)
{
    ensureEngine().registerModule(name, value);
    m_builtinModules[name] = std::move(value);
}

QJSValue ScriptingEngine::evaluate(const QString &script)
{
    const auto result = ensureEngine().evaluate(script);
    if (result.isError()) {
        logError(result);
    }

    return result;
}

QJSValue ScriptingEngine::call(const QJSValue &function, const QJSValueList &args) const
{
    const auto result = function.call(args);
    if (result.isError()) {
        logError(result);
    }

    return result;
}

QString ScriptingEngine::errorToString(const QJSValue &error) const
{
    const auto name = error.property("name").toString();
    const auto message = error.property("message").toString();
    auto file = error.property("fileName").toString();
    if (file.isEmpty()) {
        file = "None (defined in a YAML file)";
    }
    const auto lineNumber = error.property("lineNumber").toUInt();
    const auto stack = error.property("stack").toString();

    return QString("%1: %2\nFile: %3\nLine: %4\nStack:\n%5\n").arg(name, message, file, QString::number(lineNumber), QStringHelpers::indented(stack, 4));
}

void ScriptingEngine::logError(const QJSValue &error) const
{
    qCCritical(INPUTACTIONS_SCRIPTING).nospace().noquote() << "Uncaught script error\n" << errorToString(error);
}

Promise ScriptingEngine::newPromise()
{
    const auto holder = ensureEngine().newObject();
    const auto promise = m_promiseFactory.call({holder});
    return {this, promise, holder.property("fulfill"), holder.property("reject")};
}

QJSEngine &ScriptingEngine::ensureEngine()
{
    if (!m_engine) {
        initialize();
    }
    return m_engine.value();
}

void ScriptingEngine::onWatchdogValueRestartTimerTick()
{
    QMetaObject::invokeMethod(m_watchdogTimer, "start", Qt::QueuedConnection);
}

}