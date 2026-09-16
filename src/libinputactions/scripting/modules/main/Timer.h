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

#include <QObject>

namespace InputActions
{

class Timer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool active READ active)
    Q_PROPERTY(qreal interval READ interval WRITE setInterval)

public:
    Q_INVOKABLE Timer();

    bool active() const;

    int interval() const;
    void setInterval(qreal value);

    Q_INVOKABLE void start();
    Q_INVOKABLE void start(qreal interval);
    Q_INVOKABLE void stop();

signals:
    void tick();

private slots:
    void onTimerTimeout();

private:
    QTimer m_timer;
};

}