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

#include <libinputactions/conditions/ConditionGroup.h>
#include <libinputactions/conditions/VariableCondition.h>
#include <libinputactions/config/Node.h>
#include <memory>
#include <vector>

namespace InputActions
{

class Trigger;

// Trigger list, handles triggers groups as well
template<typename T>
std::vector<std::unique_ptr<T>> parseTriggerList(const Node *node)
{
    std::vector<std::unique_ptr<T>> result;
    for (const auto *triggerNode : node->sequenceItems(true)) {
        if (const auto *subTriggersNode = triggerNode->at("gestures")) {
            // Trigger group
            for (const auto *subTriggerNode : subTriggersNode->sequenceItems()) {
                const auto mergedNode = std::make_shared<Node>(*subTriggerNode);

                std::shared_ptr<Condition> groupCondition;
                for (const auto &[key, value] : triggerNode->mapItemsRawKeys()) {
                    const auto keyStr = key->as<QString>();
                    if (keyStr == "conditions") {
                        groupCondition = triggerNode->at("conditions")->as<std::shared_ptr<Condition>>();
                    } else if (keyStr != "gestures") {
                        mergedNode->addMapItem(key->shared_from_this(), value->shared_from_this());
                    }
                }
                for (auto &trigger : parseTriggerList<T>(mergedNode.get())) {
                    if (groupCondition) {
                        if (const auto triggerCondition = trigger->activationCondition()) {
                            if (const auto triggerConditionGroup = std::dynamic_pointer_cast<ConditionGroup>(triggerCondition);
                                triggerConditionGroup && triggerConditionGroup->mode() == ConditionGroupMode::All) {
                                triggerConditionGroup->prepend(groupCondition);
                            } else {
                                auto conditionGroup = std::make_shared<ConditionGroup>();
                                conditionGroup->append(groupCondition);
                                conditionGroup->append(trigger->activationCondition());
                                trigger->setActivationCondition(conditionGroup);
                            }
                        } else {
                            trigger->setActivationCondition(groupCondition);
                        }
                    }

                    result.push_back(std::move(trigger));
                }
            }
            continue;
        }

        result.push_back(triggerNode->as<std::unique_ptr<T>>());
    }

    return result;
}

void finalizeTrigger(const Node *node, Trigger *trigger);

}