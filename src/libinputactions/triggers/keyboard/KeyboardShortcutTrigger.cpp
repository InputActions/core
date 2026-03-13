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

#include "KeyboardShortcutTrigger.h"
#include <libinputactions/triggers/core/TimeTriggerCore.h>

namespace InputActions
{

KeyboardShortcutTrigger::KeyboardShortcutTrigger(KeyboardShortcut shortcut)
    : KeyboardTrigger(TriggerType::KeyboardShortcut, std::make_unique<TimeTriggerCore>())
    , m_shortcut(std::move(shortcut))
{
}

KeyboardShortcutTrigger::KeyboardShortcutTrigger(KeyboardShortcut shortcut, std::unique_ptr<TimeTriggerCore> core)
    : KeyboardTrigger(TriggerType::KeyboardShortcut, std::move(core))
    , m_shortcut(std::move(shortcut))
{
}

bool KeyboardShortcutTrigger::canActivate(const TriggerActivationEvent &event) const
{
    return Trigger::canActivate(event) && (m_shortcut.keys == event.keyboardKeys());
}

}