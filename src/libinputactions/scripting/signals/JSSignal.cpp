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

#include "JSSignal.h"
#include "EmittableJSSignal.h"
#include <libinputactions/globals.h>
#include <libinputactions/scripting/ScriptingEngine.h>

namespace InputActions
{

JSSignal::JSSignal()
    : m_handlers(std::make_shared<std::vector<QJSValue>>())
    , m_engineSource(this)
{
}

JSSignal::JSSignal(ScriptingEngine &engine)
    : m_handlers(std::make_shared<std::vector<QJSValue>>())
    , m_engineSource(&engine)
{
}

JSSignal::JSSignal(const QObject &object)
    : m_handlers(std::make_shared<std::vector<QJSValue>>())
    , m_engineSource(&object)
{
}

JSSignal::JSSignal(EmittableJSSignal &emittableSignal)
    : m_handlers(static_cast<JSSignal &>(emittableSignal).m_handlers)
    , m_engineSource(emittableSignal.getEngine())
{
}

void JSSignal::connect(const QJSValue &func)
{
    if (auto *engine = getEngine()) {
        if (!engine->validateFunction("func", func)) {
            return;
        }

        m_handlers->push_back(func);
    }
}

void JSSignal::disconnect(const QJSValue &func)
{
    if (auto *engine = getEngine()) {
        if (!engine->validateFunction("func", func)) {
            return;
        }

        const auto count = std::erase_if(*m_handlers, [&func](const auto &value) {
            return value.strictlyEquals(func);
        });
        if (!count) {
            engine->qtEngine().throwError(QString("The specified function is not connected to the signal."));
        }
    }
}

bool JSSignal::hasHandlers() const
{
    return !m_handlers->empty();
}

ScriptingEngine *JSSignal::getEngine() const
{
    // clang-format off
    auto *result = std::visit(overloads {
        [](ScriptingEngine *engine) {
            return engine;
        },
        [this](const QObject *object) {
            auto *engine = ScriptingEngine::engineForObject(object);
            if (!engine) {
                qCCritical(INPUTACTIONS_SCRIPTING).noquote().nospace() << "Failed to get engine for signal " << this << " of object " << object;
            }
            return engine;
        },
        [this](const std::monostate &) {
            return ScriptingEngine::engineForObject(this);
        }
    }, m_engineSource);
    // clang-format on
    return result;
}

}