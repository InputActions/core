#include "Test.h"
#include <libinputactions/conditions/ConditionGroup.h>
#include <libinputactions/conditions/VariableCondition.h>
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/ConfigIssueManager.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/config/parsers/triggers.h>
#include <libinputactions/triggers/core/StrokeTriggerCore.h>
#include <libinputactions/triggers/core/TimeTriggerCore.h>
#include <libinputactions/triggers/mouse/MousePressTrigger.h>
#include <libinputactions/triggers/mouse/MouseStrokeTrigger.h>

namespace InputActions
{

class TestTriggerNodeParser : public Test
{
    Q_OBJECT

private slots:
    void finalizeTrigger_stroke_withConflictingBeginAction__throwsInvalidValueContextConfigException()
    {
        const auto trigger = std::make_unique<MouseStrokeTrigger>(std::make_unique<StrokeTriggerCore>(std::vector<Stroke>{}));
        const auto node = Node::create(R"(
            actions:
              - on: begin
                command: _
        )");

        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(finalizeTrigger(node.get(), trigger.get()), InvalidValueContextConfigException, 2, 16);
    }

    void finalizeTrigger_stroke_withNonConflictingBeginAction__doesNotThrow()
    {
        const auto trigger = std::make_unique<MouseStrokeTrigger>(std::make_unique<StrokeTriggerCore>(std::vector<Stroke>{}));
        const auto node = Node::create(R"(
            actions:
              - on: begin
                conflicting: false
                command: _
        )");

        QVERIFY_THROWS_NO_EXCEPTION(finalizeTrigger(node.get(), trigger.get()));
    }

    void finalizeTrigger_stroke_withEndAction__doesNotThrow()
    {
        const auto trigger = std::make_unique<MouseStrokeTrigger>(std::make_unique<StrokeTriggerCore>(std::vector<Stroke>{}));
        const auto node = Node::create(R"(
            actions:
              - on: end
                command: _
        )");

        QVERIFY_THROWS_NO_EXCEPTION(finalizeTrigger(node.get(), trigger.get()));
    }

    void finalizeTrigger_fingers__parsesNodeCorrectly()
    {
        const auto trigger = std::make_unique<MousePressTrigger>();
        const auto node = Node::create("fingers: 2");
        finalizeTrigger(node.get(), trigger.get());

        const auto condition = std::dynamic_pointer_cast<VariableCondition>(trigger->activationCondition());
        QVERIFY(condition);
        QCOMPARE(condition->negate(), false);
        QCOMPARE(condition->variableName(), "fingers");
        QCOMPARE(condition->comparisonOperator(), ComparisonOperator::EqualTo);

        const auto &values = condition->values();
        QCOMPARE(values.size(), 1);
        QCOMPARE(std::any_cast<qreal>(values[0].get().value()), 2);
    }

    void fingers_range__parsesNodeCorrectly()
    {
        const auto trigger = std::make_unique<MousePressTrigger>();
        const auto node = Node::create("fingers: 2-3");
        finalizeTrigger(node.get(), trigger.get());

        const auto condition = std::dynamic_pointer_cast<VariableCondition>(trigger->activationCondition());
        QVERIFY(condition);
        QCOMPARE(condition->negate(), false);
        QCOMPARE(condition->variableName(), "fingers");
        QCOMPARE(condition->comparisonOperator(), ComparisonOperator::Between);

        const auto &values = condition->values();
        QCOMPARE(values.size(), 2);
        QCOMPARE(std::any_cast<qreal>(values[0].get().value()), 2);
        QCOMPARE(std::any_cast<qreal>(values[1].get().value()), 3);
    }

    void keyboardModifiers__addsDeprecatedFeatureConfigIssue()
    {
        const auto trigger = std::make_unique<MousePressTrigger>();
        const auto node = Node::create("keyboard_modifiers: none");

        INPUTACTIONS_VERIFY_ADDS_CONFIG_ISSUE_SAVE(finalizeTrigger(node.get(), trigger.get()), DeprecatedFeatureConfigIssue, 0, 20, e);
        QCOMPARE(e->feature(), DeprecatedFeature::TriggerKeyboardModifiers);
    }

    void keyboardModifiers_any__doesNotAddCondition()
    {
        const auto trigger = std::make_unique<MousePressTrigger>();
        const auto node = Node::create("keyboard_modifiers: any");
        finalizeTrigger(node.get(), trigger.get());

        QVERIFY(!trigger->activationCondition());
    }

    void keyboardModifiers_metaAlt__parsesNodeCorrectly()
    {
        const auto trigger = std::make_unique<MousePressTrigger>();
        const auto node = Node::create("keyboard_modifiers: [ meta, alt ]");
        finalizeTrigger(node.get(), trigger.get());

        const auto condition = std::dynamic_pointer_cast<VariableCondition>(trigger->activationCondition());
        QVERIFY(condition);
        QCOMPARE(condition->negate(), false);
        QCOMPARE(condition->variableName(), "keyboard_modifiers");
        QCOMPARE(condition->comparisonOperator(), ComparisonOperator::EqualTo);

        const auto &values = condition->values();
        QCOMPARE(values.size(), 1);
        QCOMPARE(std::any_cast<Qt::KeyboardModifiers>(values[0].get().value()), Qt::KeyboardModifier::MetaModifier | Qt::KeyboardModifier::AltModifier);
    }

    void keyboardModifiers_none__parsesNodeCorrectly()
    {
        const auto trigger = std::make_unique<MousePressTrigger>();
        const auto node = Node::create("keyboard_modifiers: none");
        finalizeTrigger(node.get(), trigger.get());

        const auto condition = std::dynamic_pointer_cast<VariableCondition>(trigger->activationCondition());
        QVERIFY(condition);
        QCOMPARE(condition->negate(), false);
        QCOMPARE(condition->variableName(), "keyboard_modifiers");
        QCOMPARE(condition->comparisonOperator(), ComparisonOperator::EqualTo);

        const auto &values = condition->values();
        QCOMPARE(values.size(), 1);
        QCOMPARE(std::any_cast<Qt::KeyboardModifiers>(values[0].get().value()), Qt::KeyboardModifier::NoModifier);
    }

    void keyboardModifiers_invalid__throwsInvalidValueConfigException()
    {
        const auto trigger = std::make_unique<MousePressTrigger>();
        const auto node = Node::create("keyboard_modifiers: e");

        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(finalizeTrigger(node.get(), trigger.get()), InvalidValueConfigException, 0, 20);
    }

    void fingers_keyboardModifiers_triggerCondition__mergedIntoAllGroup()
    {
        const auto trigger = std::make_unique<MousePressTrigger>();
        const auto node = Node::create(R"(
            fingers: 2-3
            conditions: $window_maximized
            keyboard_modifiers: none
        )");
        finalizeTrigger(node.get(), trigger.get());

        const auto condition = std::dynamic_pointer_cast<ConditionGroup>(trigger->activationCondition());
        QVERIFY(condition);
        QCOMPARE(condition->negate(), false);
        QCOMPARE(condition->mode(), ConditionGroupMode::All);
        QCOMPARE(condition->conditions().size(), 3);

        const auto fingersCondition = std::dynamic_pointer_cast<VariableCondition>(condition->conditions()[0]);
        QVERIFY(fingersCondition);
        QCOMPARE(fingersCondition->negate(), false);
        QCOMPARE(fingersCondition->variableName(), "fingers");

        const auto keyboardModifiersCondition = std::dynamic_pointer_cast<VariableCondition>(condition->conditions()[1]);
        QVERIFY(keyboardModifiersCondition);
        QCOMPARE(keyboardModifiersCondition->negate(), false);
        QCOMPARE(keyboardModifiersCondition->variableName(), "keyboard_modifiers");

        const auto windowMaximizedCondition = std::dynamic_pointer_cast<VariableCondition>(condition->conditions()[2]);
        QVERIFY(windowMaximizedCondition);
        QCOMPARE(windowMaximizedCondition->negate(), false);
        QCOMPARE(windowMaximizedCondition->variableName(), "window_maximized");
    }
};

}

QTEST_MAIN(InputActions::TestTriggerNodeParser)
#include "TestTriggerNodeParser.moc"