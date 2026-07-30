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

#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusMessage>
#include <QObject>

namespace InputActions
{

static const QString INPUTACTIONS_DBUS_SERVICE = "org.inputactions";
static const QString INPUTACTIONS_DBUS_PATH = "/";

class Stroke;

class MainDBusInterface
    : public QObject
    , protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.inputactions")

public:
    /**
     * Registers the interface.
     */
    MainDBusInterface();

    /**
     * Unregisters the interface.
     */
    ~MainDBusInterface() override;

    /**
     * Sets whether loading the config and suspending InputActions through the DBus interface is allowed. This is only used in the standalone implementation.
     */
    void setAllowConfigLoading(bool value) { m_allowConfigLoading = value; }

public slots:
    QString deviceList();
    QString issues();
    Q_NOREPLY void recordStroke(const QDBusMessage &message);
    QString reloadConfig();
    QString suspend();
    QString variables(QString filter = "");

private:
    static QString strokeToBase64(const Stroke &stroke);

    QDBusConnection m_bus;
    QDBusMessage m_reply;

    bool m_allowConfigLoading = true;
};

inline std::shared_ptr<MainDBusInterface> g_mainDbusInterface;

}