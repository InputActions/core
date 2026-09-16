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

#include "Timer.h"
#include <libinputactions/scripting/ScriptingEngine.h>

namespace InputActions
{

Timer::Timer()
{
    m_timer.setInterval(1);
    m_timer.setTimerType(Qt::TimerType::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &Timer::onTimerTimeout);
}

void Timer::start()
{
    m_timer.start();
}

void Timer::start(qreal interval)
{
    setInterval(interval);
    start();
}

void Timer::stop()
{
    if (!m_timer.isActive()) {
        ScriptingEngine::engineForObject(this)->qtEngine().throwError(QString("Cannot stop an inactive timer."));
        return;
    }

    m_timer.stop();
}

bool Timer::active() const
{
    return m_timer.isActive();
}

int Timer::interval() const
{
    return m_timer.interval();
}

void Timer::setInterval(qreal value)
{
    if (value < 1 || value > INT32_MAX) {
        ScriptingEngine::engineForObject(this)->qtEngine().throwError(QJSValue::RangeError, QString("Value %1 is out of range.").arg(QString::number(value)));
        return;
    }

    m_timer.setInterval(static_cast<int>(value));
}

void Timer::onTimerTimeout()
{
    Q_EMIT tick();
}

}