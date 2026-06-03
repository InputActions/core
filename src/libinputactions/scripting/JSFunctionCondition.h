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
#include <libinputactions/conditions/Condition.h>

namespace InputActions
{

class Node;

class JSFunctionCondition : public Condition
{
public:
    /**
     * @param function Called with no arguments when the condition is evaluated. The condition is satisfied when the return value is true.
     * @param sourceNode The configuration node this action was defined in. May be nullptr.
     */
    JSFunctionCondition(QJSValue function, std::shared_ptr<const Node> sourceNode);

protected:
    bool doEvaluate(const ConditionEvaluationArguments &arguments) override;

private:
    QJSValue m_function;
    std::shared_ptr<const Node> m_sourceNode;
};

}