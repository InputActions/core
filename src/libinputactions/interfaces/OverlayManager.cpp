/*
    Input Actions - Input handler that executes user-defined actions
    Copyright (C) 2026 Marcin Woźniak

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

#include "OverlayManager.h"
#include <QFuture>
#include <QStandardPaths>
#include <libinputactions/helpers/QDBusConnection.h>
#include <libinputactions/helpers/QDBusInterface.h>
#include <libinputactions/input/backends/InputBackend.h>

namespace InputActions
{

OverlayManager::OverlayManager()
    : m_process(new QProcess)
{
    m_process->setProgram("/usr/bin/env");
    m_process->setArguments({"inputactions-overlay"});
    m_process->setUnixProcessParameters(QProcess::UnixProcessFlag::ResetIds);
}

OverlayManager::~OverlayManager()
{
    if (m_process->state() == QProcess::NotRunning) {
        m_process->deleteLater();
    } else {
        QObject::connect(m_process, &QProcess::finished, [process = m_process]() {
            process->deleteLater();
        });
        m_process->terminate();
    }
}

void OverlayManager::setEnabled(bool value)
{
    if (!value && m_process->state() != QProcess::NotRunning) {
        m_process->terminate();
    } else if (value && m_process->state() == QProcess::NotRunning) {
        m_process->start();
    }
}

bool OverlayManager::overlayProgramExists() const
{
    return !QStandardPaths::findExecutable("inputactions-overlay").isEmpty();
}

void OverlayManager::showMouseStrokeOverlay()
{
    if (auto *interface = mouseStrokeOverlayInterface()) {
        interface->asyncCall("show");
        m_mouseStrokeOverlayVisible = true;
    }
}

void OverlayManager::hideMouseStrokeOverlay()
{
    if (auto *interface = mouseStrokeOverlayInterface()) {
        interface->asyncCall("hide");
        m_mouseStrokeOverlayVisible = false;
    }
}

QFuture<QString> OverlayManager::showContextMenu(const QString &json)
{
    if (auto *interface = contextMenuOverlayInterface()) {
        return QDBusInterfaceHelpers::asyncCall<bool>(*interface, "showOverlay")
            .then([](const bool overlayAlreadyVisible) {
                if (!overlayAlreadyVisible) {
                    // The compositor will not grant pointer focus to a window that opens under a stationary pointer, so we move the pointer. Scrolling doesn't
                    // work on Hyprland and buttons can have side effects.
                    g_inputBackend->virtualMouse()->mouseMotion({-2, -2});
                    QTimer::singleShot(10, []() {
                        g_inputBackend->virtualMouse()->mouseMotion({2, 2});
                    });
                }
            })
            .then([interface, json]() {
                return QDBusInterfaceHelpers::asyncCall<QString>(*interface, "showMenu", json);
            })
            .unwrap()
            .onFailed([]() {
                return QString("");
            });
    } else {
        return QtFuture::makeReadyValueFuture(QString(""));
    }
}

QDBusInterface *OverlayManager::contextMenuOverlayInterface()
{
    if (m_contextMenuOverlayInterface) {
        return &m_contextMenuOverlayInterface.value();
    }

    m_contextMenuOverlayInterface.emplace("org.inputactions.overlay",
                                          "/org/inputactions/overlay/ContextMenuOverlay",
                                          "org.inputactions.overlay.ContextMenuOverlay",
                                          QDBusConnectionHelpers::sessionBus());
    if (m_contextMenuOverlayInterface->isValid()) {
        return &m_contextMenuOverlayInterface.value();
    }

    m_contextMenuOverlayInterface.reset();
    return {};
}

QDBusInterface *OverlayManager::mouseStrokeOverlayInterface()
{
    if (!m_mouseStrokeOverlayEnabled) {
        return {};
    }

    if (m_mouseStrokeOverlayInterface) {
        return &m_mouseStrokeOverlayInterface.value();
    }

    m_mouseStrokeOverlayInterface.emplace("org.inputactions.overlay",
                                          "/org/inputactions/overlay/MouseStrokeOverlay",
                                          "org.inputactions.overlay.MouseStrokeOverlay",
                                          QDBusConnectionHelpers::sessionBus());
    if (m_mouseStrokeOverlayInterface->isValid()) {
        return &m_mouseStrokeOverlayInterface.value();
    }

    m_mouseStrokeOverlayInterface.reset();
    return {};
}

}