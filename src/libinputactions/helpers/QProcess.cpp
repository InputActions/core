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

#include "QProcess.h"
#include <libinputactions/variables/VariableRegistry.h>
#include <QProcess>
#include <QString>

namespace InputActions::QProcessHelpers
{

void setProcessEnvironment(QProcess &process, std::map<QString, QString> environmentVariables)
{
    auto environment = QProcessEnvironment::systemEnvironment();
    for (const auto &[key, value] : environmentVariables) {
        environment.insert(key, value);
    }
    process.setProcessEnvironment(environment);
}

void process(const QString &program, const QStringList &arguments, std::map<QString, QString> extraEnvironment, bool waitForFinished)
{
    auto *process = new QProcess;
    QObject::connect(process, &QProcess::finished, [process]() {
        process->deleteLater();
    });
    process->setProgram(program);
    process->setArguments(arguments);
    setProcessEnvironment(*process, extraEnvironment);
    process->start();
    if (waitForFinished) {
        process->waitForFinished();
    }
}

QString processOutput(const QString &program, const QStringList &arguments, std::map<QString, QString> extraEnvironment)
{
    QProcess process;
    process.setProgram(program);
    process.setArguments(arguments);
    setProcessEnvironment(process, extraEnvironment);
    process.start();
    process.waitForFinished();
    return process.readAllStandardOutput();
}

void command(const QString &command, const CommandArguments &args)
{
    std::map<QString, QString> extraEnvironment;
    if (args.exposeInputActionsVariables) {
        extraEnvironment = g_variableRegistry->extraProcessEnvironment(command);
    }
    process("/bin/sh", {"-c", command}, extraEnvironment, args.waitForFinished);
}

QString commandOutput(const QString &command, const CommandOutputArguments &args)
{
    std::map<QString, QString> extraEnvironment;
    if (args.exposeInputActionsVariables) {
        extraEnvironment = g_variableRegistry->extraProcessEnvironment(command);
    }
    return processOutput("/bin/sh", {"-c", command}, extraEnvironment);
}


}