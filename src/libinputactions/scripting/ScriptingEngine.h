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

#include <QJSEngine>
#include <QMetaEnum>
#include <QObject>
#include <memory>

Q_DECLARE_LOGGING_CATEGORY(INPUTACTIONS_SCRIPTING)

namespace InputActions
{

class CoreModule;
class FulfillablePromise;
class InputBackend;
class Module;
class Promise;
class VariableRegistry;

/**
 * Q_PROPERTIES exposed to JavaScript of non-primitive types (with the exception of QObject *) must be of type QJSValue to prevent object reuse.
 */
class ScriptingEngine : public QObject
{
    Q_OBJECT

public:
    ScriptingEngine(std::shared_ptr<InputBackend> inputBackend, std::shared_ptr<VariableRegistry> variableRegistry);
    ~ScriptingEngine() override;

    Q_INVOKABLE QJSValue require(const QString &module);
    Q_INVOKABLE void unhandledPromiseRejection(const QJSValue &result);

    CoreModule &coreModule() const { return *m_coreModule; }

    /**
     * Same as QJSEngine::evaluate but with error logging.
     */
    QJSValue evaluate(const QString &script);
    /**
     * Same as evaluate, but the result is cached and the script is not evaluated on subsequent calls.
     */
    QJSValue evaluateOnce(const QString &script);
    /**
     * Same as QJSEngine::importModule but with error logging.
     */
    QJSValue importModule(const QString &fileName);

    /**
     * Same as QJSEngine::newQMetaObject, but also adds all invokable methods from TStatic to TInstance.
     */
    template<typename TInstance, typename TStatic>
    QJSValue newQMetaObject()
    {
        QStringList staticMethodNames;
        for (int i = 0; i < TStatic::staticMetaObject.methodCount(); i++) {
            staticMethodNames.push_back(TStatic::staticMetaObject.method(i).name());
        }

        const auto instanceMetaObject = m_engine.newQMetaObject(&TInstance::staticMetaObject);
        const auto staticMetaObject = m_engine.newQMetaObject(&TStatic::staticMetaObject);
        const auto func = evaluateOnce(R"(
            (instanceMetaObject, staticMetaObject, staticMethodNames) => {
                const staticInstance = new staticMetaObject();
                for (let staticMethodName of staticMethodNames) {
                    instanceMetaObject[staticMethodName] = (...args) => staticInstance[staticMethodName](...args);
                }
            }
        )");
        call(func, {instanceMetaObject, staticMetaObject, m_engine.toScriptValue(staticMethodNames)});
        return instanceMetaObject;
    }

    /**
     * Converts a JS object to a default-constructible gadget instance with optional properties.
     */
    template<typename T>
    T objectToGadget(const QJSValue &object)
    {
        T result;

        const QMetaObject &metaObject = T::staticMetaObject;
        for (qsizetype i = 0; i < metaObject.propertyCount(); i++) {
            const auto metaProperty = metaObject.property(i);
            if (object.hasOwnProperty(metaProperty.name())) {
                metaProperty.writeOnGadget(&result, object.property(metaProperty.name()).toVariant());
            }
        }

        return result;
    }

    FulfillablePromise newPromise();
    /**
     * @returns Nullptr if the specified value is not a promise.
     */
    std::shared_ptr<Promise> newPromise(const QJSValue &promise);

    template<typename T>
    QJSValue newEnum()
    {
        return newEnum(QMetaEnum::fromType<T>());
    }
    QJSValue newEnum(const QMetaEnum &metaEnum);

    /**
     * Throws a JS error if the specified value is not a function.
     * @returns Whether the specified value is a function.
     */
    bool validateFunction(const QString &argName, const QJSValue &value);

    QJSEngine &qtEngine() { return m_engine; }

    /**
     * @returns The engine for the specified object or nullptr.
     */
    static ScriptingEngine *engineForObject(const QObject *object);

    /**
     * Same as QJSValue::call but with error logging.
     */
    static QJSValue call(const QJSValue &function, const QJSValueList &args = {});

    static QString errorToString(const QJSValue &error);
    static void logError(const QJSValue &error);

private slots:
    void onWatchdogRestartTimerTick();

private:
    void initialize();
    void initializeWatchdog();

    void registerBuiltinModule(const QString &name, Module *module);

    std::shared_ptr<InputBackend> m_inputBackend;
    std::shared_ptr<VariableRegistry> m_variableRegistry;

    QJSEngine m_engine;
    std::unique_ptr<CoreModule> m_coreModule;
    std::map<QString, QJSValue> m_builtinModules;
    std::map<QString, QJSValue> m_cachedScripts;

    QThread *m_watchdogTimerThread{};
    QTimer *m_watchdogTimer{};
    QTimer m_watchdogRestartTimer;

    inline static std::set<ScriptingEngine *> s_engines;
};

inline std::unique_ptr<ScriptingEngine> g_scriptingEngine;

}