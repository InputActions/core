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

#include "EmittableJSSignal.h"
#include <QJSValueList>
#include <libinputactions/scripting/ScriptingEngine.h>

namespace InputActions
{

template<typename... TArgs>
class Signal
{
public:
    /**
     * Constructs a signal for the specified engine.
     */
    explicit Signal(ScriptingEngine &engine)
        : m_base(engine)
    {
    }
    /**
     * Constructs a signal for the specified object that was constructed from JavaScript. It is okay if the object is still being constructed and does not have
     * an engine associated with it yet, however emitting the signal will fail at that point.
     */
    explicit Signal(const QObject &object)
        : m_base(object)
    {
    }

    void emit(TArgs... args)
    {
        if (!m_base.hasHandlers() || !m_base.getEngine()) {
            return;
        }

        m_base.emit(toJsArgs(args...));
    }

    /**
     * A future that is finished when all Promise objects (if any) returned by handlers are fulfilled or rejected. If failOnError is false, the future never
     * fails. Otherwise, the future fails with a PromiseException when a handler throws an error or the Promise returned by it is rejected.
     */
    QFuture<void> emitAsync(TArgs... args, bool failOnError)
    {
        if (!m_base.hasHandlers() || !m_base.getEngine()) {
            QtFuture::makeReadyVoidFuture();
        }

        return m_base.emitAsync(toJsArgs(args...), failOnError);
    }

    /**
     * The object that can be exposed to JavaScript. Do not call while the object containing signals is being constructed.
     */
    JSSignal *jsSignal()
    {
        if (!m_nonEmittableBase) {
            m_nonEmittableBase.emplace(m_base);
        }
        return &m_nonEmittableBase.value();
    }

private:
    QJSValueList toJsArgs(TArgs... args)
    {
        QJSValueList jsArgs;
        (
            [&] {
                jsArgs.push_back(m_base.getEngine()->qtEngine().toScriptValue(args));
            }(),
            ...);
        return jsArgs;
    }

    EmittableJSSignal m_base;
    std::optional<JSSignal> m_nonEmittableBase;
};

}