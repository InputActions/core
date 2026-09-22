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

#include "JSSignal.h"

namespace InputActions
{

class EmittableJSSignal : public JSSignal
{
    Q_OBJECT

public:
    /**
     * Only for JavaScript, using this constructor in C++ will result in broken signals.
     */
    Q_INVOKABLE EmittableJSSignal() = default;

    explicit EmittableJSSignal(ScriptingEngine &engine);
    explicit EmittableJSSignal(const QObject &object);

    Q_INVOKABLE JSSignal *toNonEmittableSignal();

    Q_INVOKABLE void jsEmit(const QJSValueList &args);

    void emit(const QJSValueList &args);
    QFuture<void> emitAsync(const QJSValueList &args, bool failOnError);
};

}