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

#include "IntegratedDBusInterface.h"
#include <QRegularExpression>
#include <libinputactions/InputActionsMain.h>
#include <libinputactions/config/ConfigIssueManager.h>
#include <libinputactions/config/ConfigLoader.h>
#include <libinputactions/helpers/QDBusConnection.h>
#include <libinputactions/input/StrokeRecorder.h>
#include <libinputactions/input/backends/InputBackend.h>
#include <libinputactions/interfaces/OnScreenMessageManager.h>
#include <libinputactions/triggers/core/StrokeTriggerCore.h>
#include <libinputactions/variables/VariableRegistry.h>

namespace InputActions
{

IntegratedDBusInterface::IntegratedDBusInterface()
    : m_bus(QDBusConnectionHelpers::sessionBus())
{
    m_bus.registerService(INPUTACTIONS_DBUS_SERVICE);
    m_bus.registerObject(INPUTACTIONS_DBUS_PATH, this, QDBusConnection::ExportAllSlots);
}

IntegratedDBusInterface::~IntegratedDBusInterface()
{
    m_bus.unregisterService(INPUTACTIONS_DBUS_SERVICE);
    m_bus.unregisterObject(INPUTACTIONS_DBUS_PATH);
}

QString IntegratedDBusInterface::deviceList()
{
    return DBusInterfaceBase::deviceList();
}

QString IntegratedDBusInterface::issues()
{
    return DBusInterfaceBase::issues();
}

void IntegratedDBusInterface::recordStroke(const QDBusMessage &message)
{
    if (!g_inputBackend->initialized()) {
        sendErrorReply(QDBusError::Failed, "Stroke recording requires a valid configuration to be active.");
        return;
    }

    g_onScreenMessageManager->showMessage("InputActions is recording input. Perform a stroke gesture by moving the mouse or any amount of fingers all in the "
                                          "same direction on a touchpad or a touchscreen. Recording will end after 250 ms of inactivity.");

    message.setDelayedReply(true);
    m_reply = message.createReply();

    g_strokeRecorder->recordStroke([this](const auto &stroke) {
        m_reply << strokeToBase64(stroke);
        m_bus.send(m_reply);
        g_onScreenMessageManager->hideMessage();
    });
}

QString IntegratedDBusInterface::reloadConfig()
{
    g_configLoader->load({
        .manual = true,
    });
    return g_configIssueManager->issuesToString();
}

QString IntegratedDBusInterface::suspend()
{
    g_inputActions->suspend();
    return "success";
}

QString IntegratedDBusInterface::variables(QString filter)
{
    return variableList(g_variableRegistry.get(), filter);
}

}
