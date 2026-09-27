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

#include <QDBusInterface>
#include <QDBusPendingReply>
#include <QFuture>
#include <QPromise>

namespace InputActions::QDBusInterfaceHelpers
{

class DBusErrorException : public std::exception
{
public:
    DBusErrorException(QDBusError error);

    const QDBusError &error() const { return m_error; }

    const char *what() const noexcept override;

private:
    QDBusError m_error;
    std::string m_message;
};

/**
 * @returns A future that is finished with the call's return value on success or failed with a DBusErrorException.
 */
template<typename TReturn = void, typename... TArgs>
QFuture<TReturn> asyncCall(QDBusInterface &interface, const QString &method, TArgs &&...args)
{
    auto promise = std::make_shared<QPromise<TReturn>>();
    promise->start();

    auto call = interface.asyncCall(method, std::forward<TArgs>(args)...);
    auto *watcher = new QDBusPendingCallWatcher(call);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, [promise](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<TReturn> reply(*watcher);
        if (reply.isError()) {
            promise->setException(std::make_exception_ptr(DBusErrorException(reply.error())));
        } else if constexpr (!std::is_same_v<TReturn, void>) {
            promise->addResult(reply.template argumentAt<0>());
        }
        promise->finish();

        watcher->deleteLater();
    });

    return promise->future();
}

}