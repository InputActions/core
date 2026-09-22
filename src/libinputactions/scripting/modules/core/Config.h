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
#include <libinputactions/scripting/signals/Signal.h>

namespace InputActions
{

class Config : public QObject
{
    Q_OBJECT

    Q_PROPERTY(JSSignal *aboutToBeActivated READ aboutToBeActivated)
    Q_PROPERTY(JSSignal *aboutToBeDestroyed READ aboutToBeDestroyed)
    Q_PROPERTY(JSSignal *aboutToBeLoaded READ aboutToBeLoaded)
    Q_PROPERTY(JSSignal *activated READ activated)

public:
    Config(ScriptingEngine &engine);

    Signal<> &aboutToBeActivatedSignal() { return m_aboutToBeActivated; }
    Signal<> &aboutToBeDestroyedSignal() { return m_aboutToBeDestroyed; }
    Signal<> &aboutToBeLoadedSignal() { return m_aboutToBeLoaded; }
    Signal<> &activatedSignal() { return m_activated; }

private:
    JSSignal *aboutToBeActivated() { return m_aboutToBeActivated.jsSignal(); }
    JSSignal *aboutToBeDestroyed() { return m_aboutToBeDestroyed.jsSignal(); }
    JSSignal *aboutToBeLoaded() { return m_aboutToBeLoaded.jsSignal(); }
    JSSignal *activated() { return m_activated.jsSignal(); }

    Signal<> m_aboutToBeActivated;
    Signal<> m_aboutToBeDestroyed;
    Signal<> m_aboutToBeLoaded;
    Signal<> m_activated;
};

}