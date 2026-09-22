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

#include "EmittableJSSignal.h"
#include <libinputactions/helpers/QThread.h>
#include <libinputactions/scripting/ScriptingEngine.h>
#include <libinputactions/scripting/promises/Promise.h>
#include <libinputactions/scripting/promises/PromiseException.h>

namespace InputActions
{

EmittableJSSignal::EmittableJSSignal(ScriptingEngine &engine)
    : JSSignal(engine)
{
}

EmittableJSSignal::EmittableJSSignal(const QObject &object)
    : JSSignal(object)
{
}

JSSignal *EmittableJSSignal::toNonEmittableSignal()
{
    return new JSSignal(*this);
}

void EmittableJSSignal::emit(const QJSValueList &args)
{
    const auto handlers = *m_handlers;
    for (const auto &handler : handlers) {
        ScriptingEngine::call(handler, args);
    }
}

QFuture<void> EmittableJSSignal::emitAsync(const QJSValueList &args, bool failOnError)
{
    auto qtPromise = std::make_shared<QPromise<void>>();
    qtPromise->start();

    std::vector<std::shared_ptr<Promise>> promises;

    const auto handlers = *m_handlers;
    for (const auto &handler : handlers) {
        const auto result = ScriptingEngine::call(handler, args);
        if (result.isError()) {
            if (failOnError) {
                qtPromise->setException(std::make_exception_ptr(PromiseException(result)));
                qtPromise->finish();
                return qtPromise->future();
            } else {
                continue;
            }
        }

        if (const auto promise = getEngine()->newPromise(result)) {
            promises.push_back(promise);
        }
    }

    size_t promiseCount = promises.size();
    if (!promiseCount) {
        qtPromise->finish();
        return qtPromise->future();
    }

    auto completedPromises = std::make_shared<size_t>(0);
    const auto handleCompletedPromise = [qtPromise, promiseCount, completedPromises]() {
        if (++(*completedPromises) == promiseCount) {
            qtPromise->finish();
        }
    };

    for (const auto &promise : promises) {
        promise->future()
            .then([promise, handleCompletedPromise](const auto &) {
                handleCompletedPromise();
            })
            .onFailed([this, qtPromise, failOnError, handleCompletedPromise](const PromiseException &error) {
                if (failOnError) {
                    qtPromise->setException(std::make_exception_ptr(error));
                    qtPromise->finish();
                } else {
                    getEngine()->unhandledPromiseRejection(error.value());
                    handleCompletedPromise();
                }
            });
    }

    return qtPromise->future();
}

void EmittableJSSignal::jsEmit(const QJSValueList &args)
{
    emit(args);
}

}