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

#include "Action.h"
#include <QString>
#include <libinputactions/Value.h>
#include <qregularexpression.h>

namespace InputActions
{

class TextSubstitutionRule
{
public:
    TextSubstitutionRule() = default;
    TextSubstitutionRule(QRegularExpression regex, Value<QString> newText);

    const QRegularExpression &regex() const { return m_regex; }
    const Value<QString> &newText() const { return m_newText; }

private:
    QRegularExpression m_regex;
    Value<QString> m_newText;
};

class ReplaceTextAction : public Action
{
public:
    ReplaceTextAction(std::vector<TextSubstitutionRule> rules);

    const std::vector<TextSubstitutionRule> &rules() const { return m_rules; }

    bool async() const override;

protected:
    void executeImpl(const ActionExecutionArguments &args) override;

private:
    std::vector<TextSubstitutionRule> m_rules;
};

}