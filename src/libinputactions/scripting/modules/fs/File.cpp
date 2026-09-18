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

#include "File.h"
#include <QFile>
#include <QThreadPool>
#include <libinputactions/helpers/QThread.h>
#include <libinputactions/scripting/ScriptingEngine.h>
#include <libinputactions/scripting/promises/FulfillablePromise.h>

namespace InputActions
{

QJSValue FileStatic::readAllText(const QString &path)
{
    const auto promise = ScriptingEngine::engineForObject(this)->newPromise();
    QThreadPool::globalInstance()->start([path, promise]() {
        QFile file(path);
        if (!file.open(QIODeviceBase::ReadOnly | QIODeviceBase::Text)) {
            promise.reject(file.errorString());
            return;
        }

        QTextStream stream(&file);
        promise.fulfill(stream.readAll());
    });
    return promise.jsPromise();
}

QJSValue FileStatic::writeAllText(const QString &path, const QString &text)
{
    const auto promise = ScriptingEngine::engineForObject(this)->newPromise();
    QThreadPool::globalInstance()->start([path, text, promise]() {
        QFile file(path);
        if (!file.open(QIODeviceBase::WriteOnly | QIODeviceBase::Text)) {
            promise.reject(file.errorString());
            return;
        }

        QTextStream stream(&file);
        stream << text;
        stream.flush();
        promise.fulfill();
    });
    return promise.jsPromise();
}

}