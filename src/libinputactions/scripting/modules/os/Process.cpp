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

#include "Process.h"
#include <QProcess>
#include <libinputactions/helpers/QProcess.h>
#include <libinputactions/scripting/ScriptingEngine.h>
#include <libinputactions/scripting/promises/FulfillablePromise.h>

namespace InputActions
{

QJSValue ProcessStatic::run(const QString &program, const QJSValue &argumentsObject)
{
    auto *engine = ScriptingEngine::engineForObject(this);
    auto error = engine->qtEngine().newErrorObject(QJSValue::GenericError);

    const auto arguments = g_scriptingEngine->objectToGadget<ProcessRunArguments>(argumentsObject);
    const auto promise = engine->newPromise();
    auto *process = new QProcess;
    connect(process, &QProcess::errorOccurred, this, [promise, process, error](const auto processError) mutable {
        if (processError == QProcess::ProcessError::FailedToStart) {
            error.setProperty("message", QString("Failed to start process: %1.").arg(process->errorString()));
            process->deleteLater();
            promise.reject(error);
        }
    });
    connect(process, &QProcess::finished, this, [promise, process, arguments](const auto exitCode) {
        auto *finishedProcess = new FinishedProcess(exitCode);
        QJSEngine::setObjectOwnership(finishedProcess, QJSEngine::JavaScriptOwnership);

        if (arguments.captureOutput()) {
            if (arguments.mergeOutput()) {
                finishedProcess->setStandardOutput(process->readAll());
            } else {
                finishedProcess->setStandardOutput(process->readAllStandardOutput());
                finishedProcess->setStandardError(process->readAllStandardError());
            }
        }

        promise.fulfill(finishedProcess);
        process->deleteLater();
    });

    process->setProgram(program);
    process->setArguments(arguments.arguments());
    if (arguments.captureOutput() && arguments.mergeOutput()) {
        process->setProcessChannelMode(QProcess::MergedChannels);
    }
    if (const auto &workingDirectory = arguments.workingDirectory()) {
        process->setWorkingDirectory(workingDirectory.value());
    }

    if (const auto &extraEnvironment = arguments.extraEnvironment(); !extraEnvironment.empty()) {
        auto environment = QProcessHelpers::cachedSystemEnvironment();
        for (const auto &[key, value] : extraEnvironment) {
            environment.insert(key, value);
        }
        process->setProcessEnvironment(environment);
    }

    process->start();
    return promise.jsPromise();
}

FinishedProcess::FinishedProcess(int exitCode)
    : m_exitCode(exitCode)
{
}

void ProcessRunArguments::setWorkingDirectory(QString value)
{
    if (value.isEmpty()) {
        return;
    }
    m_workingDirectory = std::move(value);
}

QString ProcessRunArguments::_workingDirectory() const
{
    return m_workingDirectory.value_or("");
}

}