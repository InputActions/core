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

#include "Promise.h"
#include "PromiseException.h"
#include <libinputactions/scripting/ScriptingEngine.h>

namespace InputActions
{

Promise::Promise(QJSValue promise, ScriptingEngine &engine)
    : m_jsPromise(std::move(promise))
{
    QJSEngine::setObjectOwnership(this, QJSEngine::CppOwnership);

    m_qtPromise.start();

    const auto self = engine.qtEngine().newQObject(this);
    const auto initFunc = engine.evaluateOnce(R"(
        (promise, self) => {
            promise.__then(self.onFulfilled, self.onRejected);
        }
    )");
    ScriptingEngine::call(initFunc, {m_jsPromise, self});
}

QFuture<QJSValue> Promise::future()
{
    return m_qtPromise.future();
}

void Promise::onFulfilled(const QJSValue &value)
{
    m_qtPromise.addResult(value);
    auto self = shared_from_this(); // The lambdas passed to QFuture::then and QFuture::onFailed get deleted before QPromise::finish finishes execution
    m_qtPromise.finish();
}

void Promise::onRejected(const QJSValue &value)
{
    m_qtPromise.setException(std::make_exception_ptr(PromiseException(value)));
    auto self = shared_from_this();
    m_qtPromise.finish();
}

}