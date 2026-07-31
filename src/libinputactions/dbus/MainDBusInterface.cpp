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

#include "MainDBusInterface.h"
#include <QRegularExpression>
#include <libinputactions/InputActionsMain.h>
#include <libinputactions/config/ConfigIssueManager.h>
#include <libinputactions/config/ConfigLoader.h>
#include <libinputactions/config/GlobalConfig.h>
#include <libinputactions/helpers/QDBusConnection.h>
#include <libinputactions/input/StrokeRecorder.h>
#include <libinputactions/input/backends/InputBackend.h>
#include <libinputactions/input/devices/InputDevice.h>
#include <libinputactions/interfaces/OnScreenMessageManager.h>
#include <libinputactions/triggers/core/StrokeTriggerCore.h>
#include <libinputactions/variables/VariableRegistry.h>

namespace InputActions
{

MainDBusInterface::MainDBusInterface()
    : m_bus(QDBusConnectionHelpers::sessionBus())
{
    m_bus.registerService(INPUTACTIONS_DBUS_SERVICE);
    m_bus.registerObject(INPUTACTIONS_DBUS_PATH, this, QDBusConnection::ExportAllSlots);
}

MainDBusInterface::~MainDBusInterface()
{
    m_bus.unregisterService(INPUTACTIONS_DBUS_SERVICE);
    m_bus.unregisterObject(INPUTACTIONS_DBUS_PATH);
}

QString MainDBusInterface::deviceList()
{
    QStringList result;
    for (const auto *device : g_inputBackend->devices()) {
        result.push_back(device->toString());
    }
    result.sort();
    return result.join("\n\n");
}

QString MainDBusInterface::issues()
{
    return g_configIssueManager->issuesToString();
}

void MainDBusInterface::recordStroke(const QDBusMessage &message)
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

QString MainDBusInterface::reloadConfig()
{
    if (!m_allowConfigLoading) {
        sendErrorReply(QDBusError::Failed, "Loading the configuration is not allowed while the client is inactive.");
        return {};
    }

    g_configLoader->load({
        .manual = true,
    });
    return g_configIssueManager->issuesToString();
}

QString MainDBusInterface::suspend()
{
    if (!m_allowConfigLoading) {
        sendErrorReply(QDBusError::Failed, "Suspending is not allowed while the client is inactive.");
        return {};
    }

    g_inputActions->suspend();
    return "success";
}

QString MainDBusInterface::variables(QString filter)
{
    if (!g_globalConfig->allowExternalVariableAccess()) {
        return "External variable access has been disabled. Set 'external_variable_access' to 'true' to enable.";
    }

    QStringList result;
    const QRegularExpression filterRegex(filter);
    for (const auto &[name, variable] : g_variableRegistry->variables()) {
        if (variable->hidden() || !filterRegex.match(name).hasMatch()) {
            continue;
        }
        result.push_back(QString("%1: %2").arg(name, variable->operations()->toString()));
    }
    return result.join('\n');
}

QString MainDBusInterface::strokeToBase64(const Stroke &stroke)
{
    QByteArray bytes;
    const auto &points = stroke.points();
    for (size_t i = 0; i < points.size(); i++) {
        // All values range from -1 to 1
        bytes.push_back(static_cast<char>(points[i].x * 100));
        bytes.push_back(static_cast<char>(points[i].y * 100));
        bytes.push_back(static_cast<char>(points[i].t * 100));
        bytes.push_back(static_cast<char>(points[i].alpha * 100));
    }

    return QString("'%1'").arg(bytes.toBase64());
}

}
