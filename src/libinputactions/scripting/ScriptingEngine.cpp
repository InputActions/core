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
#include "modules/Module.h"
#include "modules/core/CoreModule.h"
#include "modules/desktop/generic/DesktopGenericModule.h"
#include "modules/fs/FSModule.h"
#include "modules/main/MainModule.h"
#include <libinputactions/InputActionsMain.h>
#include <libinputactions/globals.h>
#include <libinputactions/helpers/QString.h>
#include <libinputactions/helpers/QThread.h>
#include <libinputactions/interfaces/NotificationManager.h>

Q_LOGGING_CATEGORY(INPUTACTIONS_SCRIPTING, "inputactions.scripting", QtWarningMsg)

namespace InputActions
{

static const std::chrono::milliseconds WATCHDOG_TIMER_TIMEOUT{2000};
static const std::chrono::milliseconds WATCHDOG_TIMER_RESET_INTERVAL{1000};

ScriptingEngine::ScriptingEngine(InputBackend &inputBackend, VariableRegistry &variableRegistry)
    : m_inputBackend(inputBackend)
    , m_variableRegistry(variableRegistry)
{
    QJSEngine::setObjectOwnership(this, QJSEngine::CppOwnership);
    s_engines.insert(this);
    initialize();
}

ScriptingEngine::~ScriptingEngine()
{
    s_engines.erase(this);

    QMetaObject::invokeMethod(m_watchdogTimer, "stop", Qt::BlockingQueuedConnection);
    m_watchdogTimerThread->quit();
    m_watchdogTimerThread->wait();

    m_watchdogTimer->deleteLater();
    m_watchdogTimerThread->deleteLater();
}

void ScriptingEngine::initialize()
{
    m_engine.installExtensions(QJSEngine::ConsoleExtension);

    initializeWatchdog();

    m_coreModule = std::make_unique<CoreModule>(*this, m_inputBackend, m_variableRegistry);
    QJSEngine::setObjectOwnership(m_coreModule.get(), QJSEngine::CppOwnership);
    registerBuiltinModule("inputactions/core", m_coreModule.get());

    registerBuiltinModule("inputactions", new MainModule(*this));
    registerBuiltinModule("inputactions/desktop/generic", new DesktopGenericModule(*this));
    registerBuiltinModule("inputactions/fs", new FSModule(*this));

    // TODO Maybe perform the unhandled promise check after garbage collection if possible
    const auto initFunc = evaluate(R"(
        engine => {
            require = engine.require;

            const { delay } = require("inputactions");

            const patch = (promise) => {
                promise.__then = promise.then;
                promise.then = function(onFulfilled, onRejected) {
                    this.__handled = true;
                    return patch(this.__then(onFulfilled, onRejected));
                };

                promise.__then(undefined, x => {
                    delay(100).__then(() => {
                        if (!promise.__handled) {
                            engine.unhandledPromiseRejection(x);
                        }
                    })
                });

                return promise;
            }

            const _Promise = Promise;
            Promise = function(executor) {
                return patch(new _Promise((resolve, reject) => {
                    executor(resolve, reject);
                }));
            }
            Promise.all = _Promise.all;
            Promise.race = _Promise.race;
            Promise.reject = _Promise.reject;
            Promise.resolve = _Promise.resolve;
        }
    )");
    call(initFunc, {m_engine.newQObject(this)});
}

void ScriptingEngine::initializeWatchdog()
{
    m_watchdogTimerThread = new QThread;
    m_watchdogTimer = new QTimer;

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

    connect(&m_watchdogRestartTimer, &QTimer::timeout, this, &ScriptingEngine::onWatchdogRestartTimerTick);
    m_watchdogRestartTimer.setInterval(WATCHDOG_TIMER_RESET_INTERVAL);
    m_watchdogRestartTimer.start();
    onWatchdogRestartTimerTick();
}

QJSValue ScriptingEngine::require(const QString &module)
{
    if (m_builtinModules.contains(module)) {
        return m_builtinModules[module];
    }

    return m_engine.importModule(module);
}

void ScriptingEngine::unhandledPromiseRejection(const QJSValue &result)
{
    if (result.isError()) {
        qCCritical(INPUTACTIONS_SCRIPTING).nospace().noquote() << "Uncaught (in promise) script error\n" << errorToString(result);
    } else {
        qCCritical(INPUTACTIONS_SCRIPTING).nospace().noquote() << "Uncaught (in promise) " << result.toString();
    }
}

void ScriptingEngine::registerBuiltinModule(const QString &name, Module *module)
{
    auto object = m_engine.newQObject(module);
    module->initialize(object);
    m_engine.registerModule(name, object);
    m_builtinModules[name] = std::move(object);
}

QJSValue ScriptingEngine::newEnum(const QMetaEnum &metaEnum)
{
    auto object = m_engine.newObject();
    for (int i = 0; i < metaEnum.keyCount(); i++) {
        object.setProperty(metaEnum.key(i), metaEnum.value(i));
    }
    return object;
}

void ScriptingEngine::disableWatchdog()
{
    QMetaObject::invokeMethod(m_watchdogTimer, "stop", Qt::BlockingQueuedConnection);
    m_watchdogRestartTimer.stop();
}

QJSValue ScriptingEngine::evaluate(const QString &script)
{
    const auto result = m_engine.evaluate(script);
    if (result.isError()) {
        logError(result);
    }

    return result;
}

QJSValue ScriptingEngine::evaluateOnce(const QString &script)
{
    if (m_cachedScripts.contains(script)) {
        return m_cachedScripts.at(script);
    }
    return m_cachedScripts[script] = evaluate(script);
}

QJSValue ScriptingEngine::importModule(const QString &fileName)
{
    const auto result = m_engine.importModule(fileName);
    if (result.isError()) {
        logError(result);
    }

    return result;
}

QJSValue ScriptingEngine::call(const QJSValue &function, const QJSValueList &args)
{
    const auto result = function.call(args);
    if (result.isError()) {
        logError(result);
    }

    return result;
}

QString ScriptingEngine::errorToString(const QJSValue &error)
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

void ScriptingEngine::logError(const QJSValue &error)
{
    qCCritical(INPUTACTIONS_SCRIPTING).nospace().noquote() << "Uncaught script error\n" << errorToString(error);
}

Promise ScriptingEngine::newPromise()
{
    const auto factory = evaluateOnce(R"(
        holder => {
            return new Promise((fulfill, reject) => {
                holder.fulfill = fulfill;
                holder.reject = reject;
            });
        }
    )");
    const auto holder = m_engine.newObject();
    const auto promise = factory.call({holder});
    return {this, promise, holder.property("fulfill"), holder.property("reject")};
}

ScriptingEngine *ScriptingEngine::engineForObject(const QObject *object)
{
    for (auto *engine : s_engines) {
        if (qjsEngine(object) == &engine->m_engine) {
            return engine;
        }
    }
    return {};
}

void ScriptingEngine::onWatchdogRestartTimerTick()
{
    QMetaObject::invokeMethod(m_watchdogTimer, "start", Qt::QueuedConnection);
}

}