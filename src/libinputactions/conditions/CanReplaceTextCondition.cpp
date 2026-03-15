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

#include "CanReplaceTextCondition.h"
#include <libinputactions/actions/ReplaceTextAction.h>
#include <libinputactions/interfaces/TextInput.h>

namespace InputActions
{

CanReplaceTextCondition::CanReplaceTextCondition(std::vector<TextSubstitutionRule> rules)
    : m_rules(std::move(rules))
{
}

CanReplaceTextCondition::~CanReplaceTextCondition() = default;

bool CanReplaceTextCondition::evaluateImpl(const ConditionEvaluationArguments &arguments)
{
    return std::ranges::any_of(m_rules, [](const auto &rule) {
        return g_textInput->canReplaceSurroundingText(rule.regex());
    });
}

}