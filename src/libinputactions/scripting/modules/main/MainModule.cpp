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

#include "MainModule.h"
#include <libinputactions/scripting/Promise.h>

namespace InputActions
{

MainModule::MainModule(ScriptingEngine &engine)
    : m_engine(engine)
    , m_globalObject(engine.ensureEngine().globalObject())
{
}

QJSValue MainModule::delay(double duration)
{
    auto promise = g_scriptingEngine->newPromise();
    if (duration < 1 || duration > INT32_MAX) {
        promise.reject(m_engine.ensureEngine().newErrorObject(QJSValue::RangeError, QString("Value %1 is out of range.").arg(QString::number(duration))));
        return promise.promise();
    }

    QTimer::singleShot(std::floor(duration), Qt::PreciseTimer, this, [promise]() {
        promise.fulfill();
    });
    return promise.promise();
}

}