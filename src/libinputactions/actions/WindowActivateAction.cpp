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

#include "WindowActivateAction.h"
#include <QThread>
#include <libinputactions/helpers/QThread.h>
#include <libinputactions/interfaces/Window.h>
#include <libinputactions/interfaces/WindowProvider.h>
#include <libinputactions/variables/VariableRegistry.h>

namespace InputActions
{

WindowActivateAction::WindowActivateAction(Value<QString> targetWindowId)
    : m_targetWindowId(std::move(targetWindowId))
    , m_previousWindowIdVariable(g_variableRegistry->variable(BuiltinVariables::PreviousWindowId).value())
{
}

void WindowActivateAction::doExecute(const ActionExecutionArguments &args)
{
    QThreadHelpers::runOnThread(
        QThreadHelpers::mainThread(),
        [this]() {
            const auto targetWindowId = m_targetWindowId.get();
            if (!targetWindowId) {
                return;
            }

            if (const auto targetWindow = g_windowProvider->findWindowById(targetWindowId.value())) {
                if (const auto previousWindow = g_windowProvider->activeWindow()) {
                    m_previousWindowIdVariable.setValue(previousWindow->id());
                }
                targetWindow->activate();
            }
        },
        true);
}

}