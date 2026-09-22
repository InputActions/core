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
#include "Timer.h"
#include <libinputactions/PointF.h>
#include <libinputactions/scripting/promises/FulfillablePromise.h>
#include <libinputactions/scripting/signals/EmittableJSSignal.h>

namespace InputActions
{

MainModule::MainModule(ScriptingEngine &engine)
    : Module(engine)
    , m_globalObject(engine.qtEngine().globalObject())
{
}

void MainModule::initialize(QJSValue &self)
{
    const auto emittableSignal = engine().qtEngine().newQMetaObject(&EmittableJSSignal::staticMetaObject);
    const auto initFunc = engine().evaluate(R"(
        emittableSignalMetaObject => {
            emittableSignalMetaObject.emit = function(...args) { return this.jsEmit([...args]); }
        }
    )");
    ScriptingEngine::call(initFunc, {emittableSignal});

    self.setProperty("EmittableSignal", emittableSignal);
    self.setProperty("Point", engine().qtEngine().newQMetaObject(&PointF::staticMetaObject));
    self.setProperty("Timer", engine().qtEngine().newQMetaObject(&Timer::staticMetaObject));
}

QJSValue MainModule::delay(double duration)
{
    if (duration < 1 || duration > INT32_MAX) {
        engine().qtEngine().throwError(QJSValue::RangeError, QString("Value %1 is out of range.").arg(QString::number(duration)));
        return {};
    }

    const auto promise = engine().newPromise();
    QTimer::singleShot(std::floor(duration), Qt::PreciseTimer, this, [promise]() {
        promise.fulfill();
    });
    return promise.jsPromise();
}

}