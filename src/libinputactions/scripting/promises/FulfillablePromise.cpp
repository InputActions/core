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

#include "FulfillablePromise.h"

namespace InputActions
{

FulfillablePromise::FulfillablePromise(QJSValue promise, QJSValue fulfillFunc, QJSValue rejectFunc, ScriptingEngine &engine)
    : m_jsPromise(std::move(promise))
    , m_fulfillFunc(std::move(fulfillFunc))
    , m_rejectFunc(std::move(rejectFunc))
    , m_engine(engine)
{
}

void FulfillablePromise::fulfill() const
{
    QThreadHelpers::runOnThread(
        QThreadHelpers::mainThread(),
        [this]() {
            if (m_fulfillFunc.isCallable()) {
                ScriptingEngine::call(m_fulfillFunc);
            }
        },
        true);
}

void FulfillablePromise::reject(const QString &errorMessage) const
{
    QThreadHelpers::runOnThread(
        QThreadHelpers::mainThread(),
        [this, errorMessage = std::move(errorMessage)]() {
            reject(m_engine.qtEngine().newErrorObject(QJSValue::GenericError, errorMessage));
        },
        true);
}

void FulfillablePromise::reject(const QJSValue &error) const
{
    QThreadHelpers::runOnThread(
        QThreadHelpers::mainThread(),
        [this, error = std::move(error)]() {
            if (m_rejectFunc.isCallable()) {
                ScriptingEngine::call(m_rejectFunc, {error});
            }
        },
        true);
}

}