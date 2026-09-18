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

#include <QProcessEnvironment>
#include <QString>

namespace InputActions::QProcessHelpers
{

struct CommandArguments
{
    /**
     * Whether to expose InputActions variables referenced in the command as environment variables.
     */
    bool exposeInputActionsVariables = false;
    bool waitForFinished = false;
};

struct CommandOutputArguments
{
    /**
     * Whether to expose InputActions variables referenced in the command as environment variables.
     */
    bool exposeInputActionsVariables = false;
};

/**
 * Runs a command in /bin/sh.
 */
void command(const QString &command, const CommandArguments &args = {});
/**
 * Runs a command in /bin/sh.
 * @returns The standard output.
 */
QString commandOutput(const QString &command, const CommandOutputArguments &args = {});

const QProcessEnvironment &cachedSystemEnvironment();

}