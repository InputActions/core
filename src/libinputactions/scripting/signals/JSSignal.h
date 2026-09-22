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

#include <QJSValue>
#include <QObject>

namespace InputActions
{

class EmittableJSSignal;
class ScriptingEngine;

using EngineSourceVariant = std::variant<ScriptingEngine *, const QObject *, std::monostate>;

class JSSignal : public QObject
{
    Q_OBJECT

public:
    explicit JSSignal(ScriptingEngine &engine);
    explicit JSSignal(const QObject &object);
    /**
     * Constructs a non-emittable signal from the specified emittable signal. Both object share the same list of handlers.
     */
    explicit JSSignal(EmittableJSSignal &emittableSignal);

    Q_INVOKABLE void connect(const QJSValue &func);
    Q_INVOKABLE void disconnect(const QJSValue &func);

    bool hasHandlers() const;
    /**
     * The engine associated with the signal or nullptr if not available.
     */
    ScriptingEngine *getEngine() const;

protected:
    JSSignal();

    std::shared_ptr<std::vector<QJSValue>> m_handlers;
    EngineSourceVariant m_engineSource;
};

}