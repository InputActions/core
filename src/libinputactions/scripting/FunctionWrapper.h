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
#include <QObject>

namespace InputActions
{

/**
 * Wraps a function in a QObject to make it invokable from JavaScript.
 */
class FunctionWrapper : public QObject
{
    Q_OBJECT

public:
    template<typename TReturn, typename... TArgs, typename TFunction>
    static FunctionWrapper *create(QJSEngine *engine, TFunction &&func)
    {
        std::function<TReturn(TArgs...)> function = std::forward<TFunction>(func);
        const auto wrapper = [engine, function = std::move(function)](QJSValueList args) -> QJSValue {
            if (args.size() != sizeof...(TArgs)) {
                engine->throwError(QString("Invalid argument count."));
                return {};
            }

            return call(engine, function, args, std::index_sequence_for<TArgs...>());
        };

        return new FunctionWrapper(engine, wrapper);
    }

    Q_INVOKABLE QJSValue call(QJSValueList args);

private:
    FunctionWrapper(QJSEngine *engine, std::function<QJSValue(QJSValueList args)> function);

    template<typename TReturn, typename... TArgs, size_t... I>
    static QJSValue call(QJSEngine *engine, const std::function<TReturn(TArgs...)> &function, const QJSValueList &args, std::index_sequence<I...>)
    {
        if constexpr (std::is_void_v<TReturn>) {
            function(engine->fromScriptValue<TArgs>(args[I])...);
            return {};
        }

        const auto result = function(engine->fromScriptValue<TArgs>(args[I])...);
        if constexpr (std::is_same_v<TReturn, QJSValue>) {
            return result;
        }

        return engine->toScriptValue(std::move(result));
    }

    QJSEngine *m_engine;
    std::function<QJSValue(QJSValueList args)> m_function;
};

}