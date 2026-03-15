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

#include "TextInput.h"
#include <libinputactions/helpers/QThread.h>
#include <libinputactions/variables/VariableManager.h>

namespace InputActions
{

bool TextInput::canReplaceSurroundingText(const QRegularExpression &regex)
{
    const auto text = surroundingText();
    if (!text) {
        return false;
    }

    const auto match = regex.match(text.value());
    if (!match.hasMatch()) {
        return false;
    }

    const auto cursorPos = surroundingTextCursorPosition();
    return !cursorPos || cursorPos == match.capturedEnd();
}

void TextInput::replaceSurroundingText(const QRegularExpression &regex, const Value<QString> &newText)
{
    uint32_t capturedLength{};
    QThreadHelpers::runOnThread(
        QThreadHelpers::mainThread(),
        [this, &regex, &capturedLength]() {
            const auto match = regex.match(surroundingText().value());
            capturedLength = match.capturedLength();

            for (auto i = 0; i < REGEX_MATCH_VARIABLE_COUNT; i++) {
                auto *variable = g_variableManager->getVariable(QString("match_%1").arg(i));
                if (i > match.lastCapturedIndex()) {
                    variable->set({});
                    continue;
                }

                variable->set(match.capturedTexts()[i]);
            }
        },
        true);

    const auto text = newText.get();
    if (!text) {
        return;
    }

    QThreadHelpers::runOnThread(
        QThreadHelpers::mainThread(),
        [this, capturedLength, &text = text.value()]() {
            deleteSurroundingText(capturedLength, 0);
            writeText(text);
        },
        true);
}

}