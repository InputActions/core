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

#include "triggers.h"
#include "NodeParser.h"
#include "containers.h"
#include "flags.h"
#include "separated-string.h"
#include "utils.h"
#include <libinputactions/conditions/ConditionGroup.h>
#include <libinputactions/conditions/VariableCondition.h>
#include <libinputactions/config/ConfigIssueManager.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/triggers/core/DirectionalMotionTriggerCore.h>
#include <libinputactions/triggers/core/StrokeTriggerCore.h>
#include <libinputactions/triggers/core/SwipeTriggerCore.h>
#include <libinputactions/triggers/core/TimeTriggerCore.h>
#include <libinputactions/triggers/keyboard/KeyboardShortcutTrigger.h>
#include <libinputactions/triggers/mouse/MouseCircleTrigger.h>
#include <libinputactions/triggers/mouse/MousePressTrigger.h>
#include <libinputactions/triggers/mouse/MouseStrokeTrigger.h>
#include <libinputactions/triggers/mouse/MouseSwipeTrigger.h>
#include <libinputactions/triggers/mouse/MouseTrigger.h>
#include <libinputactions/triggers/mouse/MouseWheelTrigger.h>
#include <libinputactions/triggers/pointer/PointerHoverTrigger.h>
#include <libinputactions/triggers/touchpad/TouchpadCircleTrigger.h>
#include <libinputactions/triggers/touchpad/TouchpadClickTrigger.h>
#include <libinputactions/triggers/touchpad/TouchpadHoldTrigger.h>
#include <libinputactions/triggers/touchpad/TouchpadPinchTrigger.h>
#include <libinputactions/triggers/touchpad/TouchpadRotateTrigger.h>
#include <libinputactions/triggers/touchpad/TouchpadStrokeTrigger.h>
#include <libinputactions/triggers/touchpad/TouchpadSwipeTrigger.h>
#include <libinputactions/triggers/touchpad/TouchpadTapTrigger.h>
#include <libinputactions/triggers/touchscreen/TouchscreenCircleTrigger.h>
#include <libinputactions/triggers/touchscreen/TouchscreenHoldTrigger.h>
#include <libinputactions/triggers/touchscreen/TouchscreenPinchTrigger.h>
#include <libinputactions/triggers/touchscreen/TouchscreenRotateTrigger.h>
#include <libinputactions/triggers/touchscreen/TouchscreenStrokeTrigger.h>
#include <libinputactions/triggers/touchscreen/TouchscreenSwipeTrigger.h>
#include <libinputactions/triggers/touchscreen/TouchscreenTapTrigger.h>
#include <libinputactions/variables/VariableManager.h>

namespace InputActions
{

template<>
void NodeParser<std::unique_ptr<RotateTriggerCore>>::parse(const Node *node, std::unique_ptr<RotateTriggerCore> &result)
{
    result = std::make_unique<RotateTriggerCore>(node->at("direction", true)->as<RotateDirection>());
}

template<>
void NodeParser<std::unique_ptr<PinchTriggerCore>>::parse(const Node *node, std::unique_ptr<PinchTriggerCore> &result)
{
    result = std::make_unique<PinchTriggerCore>(node->at("direction", true)->as<PinchDirection>());
}

template<>
void NodeParser<std::unique_ptr<SimpleSwipeTriggerCore>>::parse(const Node *node, std::unique_ptr<SimpleSwipeTriggerCore> &result)
{
    result = std::make_unique<SimpleSwipeTriggerCore>(node->at("direction", true)->as<SimpleSwipeDirection>());
}

template<>
void NodeParser<std::unique_ptr<TimeTriggerCore>>::parse(const Node *node, std::unique_ptr<TimeTriggerCore> &result)
{
    result = std::make_unique<TimeTriggerCore>();
}

template<>
void NodeParser<std::unique_ptr<StrokeTriggerCore>>::parse(const Node *node, std::unique_ptr<StrokeTriggerCore> &result)
{
    result = std::make_unique<StrokeTriggerCore>(node->at("strokes", true)->as<std::vector<Stroke>>(true));
}

template<>
void NodeParser<std::unique_ptr<SwipeTriggerCore>>::parse(const Node *node, std::unique_ptr<SwipeTriggerCore> &result)
{
    if (const auto *directionNode = node->at("direction")) {
        result = std::make_unique<SwipeTriggerCore>(directionNode->as<SwipeDirection>());
    } else {
        const auto *angleNode = node->at("angle", true);
        const auto angles = parseSeparatedString2<qreal>(angleNode, '-');
        if (angles.first > 360 || angles.second > 360) {
            throw InvalidValueConfigException(angleNode, "The angle may not be greater than 360.");
        }

        auto swipeTriggerCore = std::make_unique<SwipeTriggerCore>(angles.first, angles.second);
        loadSetter(swipeTriggerCore.get(), &SwipeTriggerCore::setBidirectional, node->at("bidirectional"));
        result = std::move(swipeTriggerCore);
    }
}

void finalizeTrigger(const Node *node, Trigger *trigger)
{
    loadSetter(trigger, &Trigger::setBlockEvents, node->at("block_events"));
    loadSetter(trigger, &Trigger::setClearModifiers, node->at("clear_modifiers"));
    loadSetter(trigger, &Trigger::setEndCondition, node->at("end_conditions"));
    loadSetter(trigger, &Trigger::setId, node->at("id"));
    loadSetter(trigger, &Trigger::setResumeTimeout, node->at("resume_timeout"));
    loadSetter(trigger, &Trigger::setSetLastTrigger, node->at("set_last_trigger"));
    loadSetter(trigger, &Trigger::setThreshold, node->at("threshold"));
    if (auto *motionTriggerCore = dynamic_cast<MotionTriggerCore *>(&trigger->core())) {
        loadSetter(motionTriggerCore, &MotionTriggerCore::setSpeed, node->at("speed"));
    }

    auto conditionGroup = std::make_shared<ConditionGroup>();
    if (const auto *fingersNode = node->at("fingers")) {
        auto range = fingersNode->as<Range<qreal>>();
        if (!range.max()) {
            conditionGroup->append(std::make_shared<VariableCondition>(BuiltinVariables::Fingers,
                                                                       Value<qreal>(range.min().value()),
                                                                       ComparisonOperator::EqualTo));
        } else {
            conditionGroup->append(std::make_shared<VariableCondition>(BuiltinVariables::Fingers,
                                                                       std::vector<Value<std::any>>{
                                                                           Value<qreal>(range.min().value()), Value<qreal>(range.max().value())},
                                                                       ComparisonOperator::Between));
        }
    }
    if (const auto *modifiersNode = node->at("keyboard_modifiers")) {
        g_configIssueManager->addIssue(DeprecatedFeatureConfigIssue(modifiersNode, DeprecatedFeature::TriggerKeyboardModifiers));

        std::optional<Qt::KeyboardModifiers> modifiers;
        if (modifiersNode->isSequence()) {
            modifiers = modifiersNode->as<Qt::KeyboardModifiers>();
        } else {
            const auto modifierMatchingMode = modifiersNode->as<QString>();
            if (modifierMatchingMode == "none") {
                modifiers = Qt::KeyboardModifier::NoModifier;
            } else if (modifierMatchingMode != "any") {
                throw InvalidValueConfigException(modifiersNode, "Invalid keyboard modifier.");
            }
        }

        if (modifiers) {
            conditionGroup->append(std::make_shared<VariableCondition>(BuiltinVariables::KeyboardModifiers,
                                                                       Value<Qt::KeyboardModifiers>(modifiers.value()),
                                                                       ComparisonOperator::EqualTo));
        }
    }
    if (const auto *conditionsNode = node->at("conditions")) {
        conditionGroup->append(conditionsNode->as<std::shared_ptr<Condition>>());
    }

    if (conditionGroup->conditions().size() == 1) {
        trigger->setActivationCondition(conditionGroup->conditions()[0]);
    } else if (conditionGroup->conditions().size()) {
        trigger->setActivationCondition(conditionGroup);
    }

    bool accelerated{};
    loadMember(accelerated, node->at("accelerated"));

    if (const auto *actionsNode = node->at("actions")) {
        for (const auto *actionNode : actionsNode->sequenceItems()) {
            auto action = actionNode->as<std::unique_ptr<TriggerAction>>();
            if (dynamic_cast<StrokeTriggerCore *>(&trigger->core()) && action->on() != On::End && action->conflicting()) {
                throw InvalidValueContextConfigException(actionNode, "Stroke triggers only support 'on: end' conflicting actions.");
            }

            action->setAccelerated(accelerated);
            trigger->addAction(std::move(action));
        }
    }
}

template<>
void NodeParser<std::unique_ptr<KeyboardTrigger>>::parse(const Node *node, std::unique_ptr<KeyboardTrigger> &result)
{
    const auto *typeNode = node->at("type", true);
    auto type = typeNode->as<QString>();

    if (type == "shortcut") {
        result = std::make_unique<KeyboardShortcutTrigger>(node->at("shortcut", true)->as<KeyboardShortcut>(), node->as<std::unique_ptr<TimeTriggerCore>>());
    } else {
        throw InvalidValueConfigException(typeNode, QString("Trigger '%1' does not exist for this device.").arg(type));
    }

    finalizeTrigger(node, result.get());
}

template<>
void NodeParser<std::unique_ptr<MouseTrigger>>::parse(const Node *node, std::unique_ptr<MouseTrigger> &result)
{
    const auto *typeNode = node->at("type", true);
    auto type = typeNode->as<QString>();

    if (type == "circle") {
        result = std::make_unique<MouseCircleTrigger>(node->as<std::unique_ptr<RotateTriggerCore>>());
    } else if (type == "press") {
        auto pressTrigger = std::make_unique<MousePressTrigger>(node->as<std::unique_ptr<TimeTriggerCore>>());
        loadSetter(pressTrigger.get(), &MousePressTrigger::setInstant, node->at("instant"));
        result = std::move(pressTrigger);
    } else if (type == "stroke") {
        result = std::make_unique<MouseStrokeTrigger>(node->as<std::unique_ptr<StrokeTriggerCore>>());
    } else if (type == "swipe") {
        result = std::make_unique<MouseSwipeTrigger>(node->as<std::unique_ptr<SwipeTriggerCore>>());
    } else if (type == "wheel") {
        result = std::make_unique<MouseWheelTrigger>(node->as<std::unique_ptr<SimpleSwipeTriggerCore>>());
    } else {
        throw InvalidValueConfigException(typeNode, QString("Trigger '%1' does not exist for this device.").arg(type));
    }

    loadSetter(result, &MouseTrigger::setMouseButtonsExactOrder, node->at("mouse_buttons_exact_order"));
    if (const auto *mouseButtonsNode = node->at("mouse_buttons")) {
        mouseButtonsNode->as<std::set<MouseButton>>(); // Ensure no duplicates, MouseTrigger::setMouseButtons accepts a vector
        loadSetter(result, &MouseTrigger::setMouseButtons, mouseButtonsNode);
    }

    if (auto *motionTrigger = dynamic_cast<MouseMotionTrigger *>(result.get())) {
        loadSetter(motionTrigger, &MouseMotionTrigger::setLockPointer, node->at("lock_pointer"));
    }

    finalizeTrigger(node, result.get());
}

template<>
void NodeParser<std::unique_ptr<PointerTrigger>>::parse(const Node *node, std::unique_ptr<PointerTrigger> &result)
{
    const auto *typeNode = node->at("type", true);
    auto type = typeNode->as<QString>();

    if (type == "hover") {
        result = std::make_unique<PointerHoverTrigger>(node->as<std::unique_ptr<TimeTriggerCore>>());
    } else {
        throw InvalidValueConfigException(typeNode, QString("Trigger '%1' does not exist for this device.").arg(type));
    }

    finalizeTrigger(node, result.get());
}

template<>
void NodeParser<std::unique_ptr<TouchpadTrigger>>::parse(const Node *node, std::unique_ptr<TouchpadTrigger> &result)
{
    const auto *typeNode = node->at("type", true);
    auto type = typeNode->as<QString>();

    if (type == "circle") {
        result = std::make_unique<TouchpadCircleTrigger>(node->as<std::unique_ptr<RotateTriggerCore>>());
    } else if (type == "click") {
        result = std::make_unique<TouchpadClickTrigger>(node->as<std::unique_ptr<TimeTriggerCore>>());
    } else if (type == "hold" || type == "press") {
        result = std::make_unique<TouchpadHoldTrigger>(node->as<std::unique_ptr<TimeTriggerCore>>());
    } else if (type == "pinch") {
        result = std::make_unique<TouchpadPinchTrigger>(node->as<std::unique_ptr<PinchTriggerCore>>());
    } else if (type == "rotate") {
        result = std::make_unique<TouchpadRotateTrigger>(node->as<std::unique_ptr<RotateTriggerCore>>());
    } else if (type == "stroke") {
        result = std::make_unique<TouchpadStrokeTrigger>(node->as<std::unique_ptr<StrokeTriggerCore>>());
    } else if (type == "swipe") {
        result = std::make_unique<TouchpadSwipeTrigger>(node->as<std::unique_ptr<SwipeTriggerCore>>());
    } else if (type == "tap") {
        result = std::make_unique<TouchpadTapTrigger>(node->as<std::unique_ptr<TimeTriggerCore>>());
    } else {
        throw InvalidValueConfigException(typeNode, QString("Trigger '%1' does not exist for this device.").arg(type));
    }

    finalizeTrigger(node, result.get());
}

template<>
void NodeParser<std::unique_ptr<TouchscreenTrigger>>::parse(const Node *node, std::unique_ptr<TouchscreenTrigger> &result)
{
    const auto *typeNode = node->at("type", true);
    auto type = typeNode->as<QString>();

    if (type == "circle") {
        result = std::make_unique<TouchscreenCircleTrigger>(node->as<std::unique_ptr<RotateTriggerCore>>());
    } else if (type == "hold" || type == "press") {
        result = std::make_unique<TouchscreenHoldTrigger>(node->as<std::unique_ptr<TimeTriggerCore>>());
    } else if (type == "pinch") {
        result = std::make_unique<TouchscreenPinchTrigger>(node->as<std::unique_ptr<PinchTriggerCore>>());
    } else if (type == "rotate") {
        result = std::make_unique<TouchscreenRotateTrigger>(node->as<std::unique_ptr<RotateTriggerCore>>());
    } else if (type == "stroke") {
        result = std::make_unique<TouchscreenStrokeTrigger>(node->as<std::unique_ptr<StrokeTriggerCore>>());
    } else if (type == "swipe") {
        result = std::make_unique<TouchscreenSwipeTrigger>(node->as<std::unique_ptr<SwipeTriggerCore>>());
    } else if (type == "tap") {
        result = std::make_unique<TouchscreenTapTrigger>(node->as<std::unique_ptr<TimeTriggerCore>>());
    } else {
        throw InvalidValueConfigException(typeNode, QString("Trigger '%1' does not exist for this device.").arg(type));
    }

    finalizeTrigger(node, result.get());
}

}