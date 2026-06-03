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

#include "ScriptingEngine.h"
#include <QJSValue>
#include <libinputactions/helpers/QThread.h>

namespace InputActions
{

class ScriptingEngine;

/**
 * Wraps a JavaScript Promise.
 */
class Promise
{
public:
    Promise(ScriptingEngine *engine, QJSValue promise, QJSValue fulfill, QJSValue reject);

    /**
     * The JavaScript Promise object.
     */
    const QJSValue &promise() const { return m_promise; }

    /**
     * Thread-safe.
     */
    void fulfill() const;

    /**
     * Thread-safe.
     */
    template<typename T>
    void fulfill(T value) const
    {
        QThreadHelpers::runOnThread(
            QThreadHelpers::mainThread(),
            [this, value = std::move(value)]() {
                if (m_fulfill.isCallable()) {
                    m_engine->call(m_fulfill, {m_engine->ensureEngine().toScriptValue(value)});
                }
            },
            true);
    }

    void reject(const QString &errorMessage) const;

private:
    ScriptingEngine *m_engine;

    QJSValue m_promise;
    QJSValue m_fulfill;
    QJSValue m_reject;
};

}