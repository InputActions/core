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

#include "ConfigIssueManager.h"
#include "ConfigIssue.h"
#include <QStringList>
#include <libinputactions/ansi-escape-codes.h>

namespace InputActions
{

std::vector<const ConfigIssue *> ConfigIssueManager::issues() const
{
    std::vector<const ConfigIssue *> result;
    for (const auto &issue : m_issues) {
        result.emplace_back(issue.get());
    }
    return result;
}

QString ConfigIssueManager::issuesToString() const
{
    QString result;

    bool hasError{};
    for (const auto &issue : m_issues) {
        QString fileContents = "<failed to get file contents>";
        if (const auto *sourceFile = issue->sourceFile()) {
            fileContents = sourceFile->contents();
        }

        const auto lines = fileContents.split("\n");
        const auto maxLineNumberLength = QString::number(lines.size() + 1).size();

        if (issue->severity() == ConfigIssueSeverity::Error) {
            hasError = true;
        } else if (dynamic_cast<UnusedPropertyConfigIssue *>(issue.get()) && hasError) {
            // An exception will result in most unused property issues generated after it being false-positives
            continue;
        }

        result += issue->toString() + "\n";

        // Returns line number in the following format: "[number][padding] |"
        const auto lineNumber = [&maxLineNumberLength](int32_t line) {
            line++;
            const auto padding = QString(" ").repeated(maxLineNumberLength - QString::number(line).size());
            return QString("%1%2 | ").arg(QString::number(line), padding);
        };

        if (issue->position().isValid()) {
            auto lineIndex = issue->position().line();
            if (lineIndex >= lines.size()) {
                lineIndex = std::max(lines.size() - 1, static_cast<qsizetype>(0));
            }

            const auto column = issue->position().column();
            const auto &line = lines[lineIndex];
            const auto surroundingLines = 3;

            // Lines before offending line
            for (auto i = std::max(lineIndex - surroundingLines, 0); i < lineIndex; i++) {
                result += QString("%1%2\n").arg(lineNumber(i), lines[i]);
            }

            // Offending line
            const auto color = issue->colorAnsiSequence();
            result += QString("%1%2\n").arg(lineNumber(lineIndex), AnsiEscapeCode::Color::Bold + color + line + AnsiEscapeCode::Color::Reset);

            // Highlight offending line
            result += QString("%1 | %2").arg(QString(" ").repeated(maxLineNumberLength), AnsiEscapeCode::Color::Bold + color);

            auto highlight = QString("~").repeated(line.size());

            // Remove highlight for leading and trailing whitespace
            for (auto i = 0; i < line.size(); i++) {
                const auto &c = line[i];
                if (c == ' ' || c == '\t') {
                    highlight[i] = ' ';
                    continue;
                }
                break;
            }
            for (auto i = line.size() - 1; i >= 0; i--) {
                const auto &c = line[i];
                if (c == ' ' || c == '\t') {
                    highlight[i] = ' ';
                    continue;
                }
                break;
            }

            if (column < line.size()) {
                highlight[column] = '^';
            } else if (!column) {
                highlight += "^";
            }

            result += highlight + AnsiEscapeCode::Color::Reset + "\n";

            // Lines after offending line
            for (auto i = lineIndex + 1; i < std::min(lines.size(), static_cast<qsizetype>(lineIndex + surroundingLines + 1)); i++) {
                result += QString("%1%2\n").arg(lineNumber(i), lines[i]);
            }
        }
        result += "\n";
    }

    if (hasError) {
        result += "At least one error was found, which may have suppressed other issues. Run the command again after fixing it to ensure other problems are "
                  "not missed.";
    } else {
        result.chop(2);
    }
    return result;
}

void ConfigIssueManager::clearIssues()
{
    m_issues.clear();
}

}
