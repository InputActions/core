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

#include <QJSValue>
#include <QObject>

namespace InputActions
{

class ScriptingEngine;

class FinishedProcess : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int exitCode READ exitCode)

    Q_PROPERTY(QString standardOutput READ standardOutput)
    Q_PROPERTY(QString standardError READ standardError)

public:
    FinishedProcess(int exitCode);

    int exitCode() const { return m_exitCode; }
    void setExitCode(int value) { m_exitCode = value; }

    const QString &standardOutput() const { return m_standardOutput; }
    void setStandardOutput(QString value) { m_standardOutput = std::move(value); }

    const QString &standardError() const { return m_standardError; }
    void setStandardError(QString value) { m_standardError = std::move(value); }

private:
    int m_exitCode;
    QString m_standardOutput;
    QString m_standardError;
};

class ProcessRunArguments
{
    Q_GADGET

    Q_PROPERTY(QStringList arguments READ arguments WRITE setArguments)
    Q_PROPERTY(std::map<QString, QString> extraEnvironment READ extraEnvironment WRITE setExtraEnvironment)
    Q_PROPERTY(QString workingDirectory READ _workingDirectory WRITE setWorkingDirectory)

    Q_PROPERTY(bool captureOutput READ captureOutput WRITE setCaptureOutput)
    Q_PROPERTY(bool mergeOutput READ mergeOutput WRITE setMergeOutput)

public:
    const QStringList &arguments() const { return m_arguments; }
    void setArguments(QStringList value) { m_arguments = std::move(value); }

    const std::map<QString, QString> &extraEnvironment() const { return m_extraEnvironment; }
    void setExtraEnvironment(std::map<QString, QString> value) { m_extraEnvironment = std::move(value); }

    const std::optional<QString> &workingDirectory() const { return m_workingDirectory; }
    void setWorkingDirectory(QString value);

    bool captureOutput() const { return m_captureOutput; }
    void setCaptureOutput(bool value) { m_captureOutput = value; }

    bool mergeOutput() const { return m_mergeOutput; }
    void setMergeOutput(bool value) { m_mergeOutput = value; }

private:
    QString _workingDirectory() const;

    QStringList m_arguments;
    std::map<QString, QString> m_extraEnvironment;
    std::optional<QString> m_workingDirectory;

    bool m_captureOutput{};
    bool m_mergeOutput{};
};

class Process : public QObject
{
    Q_OBJECT
};

class ProcessStatic : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE ProcessStatic() = default;

    Q_INVOKABLE QJSValue run(const QString &program, const QJSValue &argumentsObject = {});
};

}