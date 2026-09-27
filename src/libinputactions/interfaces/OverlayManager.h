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

#pragma once

#include <QDBusInterface>
#include <QFuture>
#include <QObject>
#include <QProcess>

namespace InputActions
{

class OverlayManager : public QObject
{
    Q_OBJECT

public:
    OverlayManager();
    ~OverlayManager() override;

    void setEnabled(bool value);
    void setMouseStrokeOverlayEnabled(bool value) { m_mouseStrokeOverlayEnabled = value; }

    bool overlayProgramExists() const;

    void showMouseStrokeOverlay();
    void hideMouseStrokeOverlay();
    bool mouseStrokeOverlayVisible() const { return m_mouseStrokeOverlayVisible; }

    /**
     * @returns A future finished with the ID of the triggered action or an empty string.
     */
    QFuture<QString> showContextMenu(const QString &json);

private:
    /**
     * Nullptr if not available.
     */
    QDBusInterface *contextMenuOverlayInterface();
    /**
     * Nullptr if not available.
     */
    QDBusInterface *mouseStrokeOverlayInterface();

    QProcess *m_process;

    std::optional<QDBusInterface> m_contextMenuOverlayInterface;
    std::optional<QDBusInterface> m_mouseStrokeOverlayInterface;
    bool m_mouseStrokeOverlayVisible{};
    bool m_mouseStrokeOverlayEnabled{};
};

inline std::unique_ptr<OverlayManager> g_overlayManager;

}