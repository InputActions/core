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

#include "JSFunctionCondition.h"
#include <libinputactions/config/ConfigIssueManager.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/scripting/ScriptingEngine.h>

namespace InputActions
{

JSFunctionCondition::JSFunctionCondition(QJSValue function, std::shared_ptr<const Node> sourceNode)
    : m_function(std::move(function))
    , m_sourceNode(std::move(sourceNode))
{
}

bool JSFunctionCondition::doEvaluate(const ConditionEvaluationArguments &arguments)
{
    const auto result = ScriptingEngine::call(m_function);
    if (result.isError() && m_sourceNode) {
        g_configIssueManager->addIssue(UncaughtScriptErrorConfigIssue(m_sourceNode.get(), result));
        return false;
    }

    return result.toBool();
}

}