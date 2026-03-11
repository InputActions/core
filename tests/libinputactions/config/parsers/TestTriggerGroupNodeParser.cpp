#include "Test.h"
#include <libinputactions/conditions/ConditionGroup.h>
#include <libinputactions/conditions/VariableCondition.h>
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/ConfigIssueManager.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/config/parsers/triggers.h>
#include <libinputactions/triggers/mouse/MouseTrigger.h>
#include <libinputactions/variables/VariableManager.h>

namespace InputActions
{

class TestTriggerGroupNodeParser : public Test
{
    Q_OBJECT

private slots:
    void init()
    {
        g_variableManager = std::make_shared<VariableManager>();
        g_variableManager->registerLocalVariable<bool>("a");
        g_variableManager->registerLocalVariable<bool>("b");
        g_variableManager->registerLocalVariable<bool>("c");
        g_variableManager->registerLocalVariable<bool>("d");
    }

    void oneProperty__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            - id: test
              gestures:
                - type: press
        )");
        const auto triggers = parseTriggerList<MouseTrigger>(node.get());

        QCOMPARE(triggers.size(), 1);
        QCOMPARE(triggers[0]->id(), "test");
    }

    void twoTriggers__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            - id: test
              gestures:
                - type: press
                - type: press
        )");
        const auto triggers = parseTriggerList<MouseTrigger>(node.get());

        QCOMPARE(triggers.size(), 2);
        QCOMPARE(triggers[0]->id(), "test");
        QCOMPARE(triggers[1]->id(), "test");
    }

    void nested_propertyInFirstGroup__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            - id: test
              gestures:
                - gestures:
                    - type: press
        )");
        const auto triggers = parseTriggerList<MouseTrigger>(node.get());

        QCOMPARE(triggers.size(), 1);
        QCOMPARE(triggers[0]->id(), "test");
    }

    void nested_propertyInSecondGroup__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            - gestures:
                - id: test
                  gestures:
                    - type: press
        )");
        const auto triggers = parseTriggerList<MouseTrigger>(node.get());

        QCOMPARE(triggers.size(), 1);
        QCOMPARE(triggers[0]->id(), "test");
    }

    void condition_triggerWithoutCondition__appliesConditionToTrigger()
    {
        const auto node = Node::create(R"(
            - conditions: $a
              gestures:
                - type: press
        )");
        const auto triggers = parseTriggerList<MouseTrigger>(node.get());

        QCOMPARE(triggers.size(), 1);
        QVERIFY(dynamic_pointer_cast<VariableCondition>(triggers[0]->activationCondition()));
    }

    void condition_triggerWithSingleCondition__mergesConditionsIntoAllGroup()
    {
        const auto node = Node::create(R"(
            - conditions: $a
              gestures:
                - type: press
                  conditions: $b
        )");
        const auto triggers = parseTriggerList<MouseTrigger>(node.get());

        QCOMPARE(triggers.size(), 1);

        const auto condition = std::dynamic_pointer_cast<ConditionGroup>(triggers[0]->activationCondition());
        QVERIFY(condition);
        QCOMPARE(condition->negate(), false);
        QCOMPARE(condition->mode(), ConditionGroupMode::All);

        const auto &items = condition->conditions();
        QCOMPARE(items.size(), 2);

        const auto firstCondition = std::dynamic_pointer_cast<VariableCondition>(items[0]);
        QVERIFY(firstCondition);
        QCOMPARE(firstCondition->variableName(), "a");

        const auto secondCondition = std::dynamic_pointer_cast<VariableCondition>(items[1]);
        QVERIFY(secondCondition);
        QCOMPARE(secondCondition->variableName(), "b");
    }

    void nested_condition_triggerWithSingleCondition__mergesConditionsIntoAllGroup()
    {
        const auto node = Node::create(R"(
            - conditions: $a
              gestures:
                - conditions: $b
                  gestures:
                    - type: press
                      conditions: $c
        )");
        const auto triggers = parseTriggerList<MouseTrigger>(node.get());

        QCOMPARE(triggers.size(), 1);

        const auto condition = std::dynamic_pointer_cast<ConditionGroup>(triggers[0]->activationCondition());
        QVERIFY(condition);
        QCOMPARE(condition->negate(), false);
        QCOMPARE(condition->mode(), ConditionGroupMode::All);

        const auto &items = condition->conditions();
        QCOMPARE(items.size(), 3);

        const auto firstCondition = std::dynamic_pointer_cast<VariableCondition>(items[0]);
        QVERIFY(firstCondition);
        QCOMPARE(firstCondition->variableName(), "a");

        const auto secondCondition = std::dynamic_pointer_cast<VariableCondition>(items[1]);
        QVERIFY(secondCondition);
        QCOMPARE(secondCondition->variableName(), "b");

        const auto thirdCondition = std::dynamic_pointer_cast<VariableCondition>(items[2]);
        QVERIFY(thirdCondition);
        QCOMPARE(thirdCondition->variableName(), "c");
    }

    void condition_triggerWithAllConditionGroup__prependsConditionToTrigger()
    {
        const auto node = Node::create(R"(
            - conditions: $a
              gestures:
                - type: press
                  conditions:
                    - $b
                    - $c
        )");
        const auto triggers = parseTriggerList<MouseTrigger>(node.get());

        QCOMPARE(triggers.size(), 1);

        const auto condition = std::dynamic_pointer_cast<ConditionGroup>(triggers[0]->activationCondition());
        QVERIFY(condition);

        const auto &items = condition->conditions();
        QCOMPARE(items.size(), 3);
        QCOMPARE(condition->negate(), false);
        QCOMPARE(condition->mode(), ConditionGroupMode::All);

        const auto firstCondition = std::dynamic_pointer_cast<VariableCondition>(items[0]);
        QVERIFY(firstCondition);
        QCOMPARE(firstCondition->variableName(), "a");

        const auto secondCondition = std::dynamic_pointer_cast<VariableCondition>(items[1]);
        QVERIFY(secondCondition);
        QCOMPARE(secondCondition->variableName(), "b");

        const auto thirdCondition = std::dynamic_pointer_cast<VariableCondition>(items[2]);
        QVERIFY(thirdCondition);
        QCOMPARE(thirdCondition->variableName(), "c");
    }

    void condition_triggerWithAnyConditionGroup__mergesConditionsIntoAllGroup()
    {
        const auto node = Node::create(R"(
            - conditions: $a
              gestures:
                - type: press
                  conditions:
                    any:
                      - $b
                      - $c
        )");
        const auto triggers = parseTriggerList<MouseTrigger>(node.get());

        QCOMPARE(triggers.size(), 1);

        const auto condition = std::dynamic_pointer_cast<ConditionGroup>(triggers[0]->activationCondition());
        QVERIFY(condition);
        QCOMPARE(condition->negate(), false);
        QCOMPARE(condition->mode(), ConditionGroupMode::All);

        const auto &items = condition->conditions();
        QCOMPARE(items.size(), 2);

        const auto firstCondition = std::dynamic_pointer_cast<VariableCondition>(items[0]);
        QVERIFY(firstCondition);
        QCOMPARE(firstCondition->variableName(), "a");

        const auto secondCondition = std::dynamic_pointer_cast<ConditionGroup>(items[1]);
        QVERIFY(secondCondition);
        QCOMPARE(secondCondition->negate(), false);
        QCOMPARE(secondCondition->mode(), ConditionGroupMode::Any);

        const auto &secondConditionItems = secondCondition->conditions();
        QCOMPARE(secondConditionItems.size(), 2);

        const auto thirdCondition = std::dynamic_pointer_cast<VariableCondition>(secondConditionItems[0]);
        QVERIFY(thirdCondition);
        QCOMPARE(thirdCondition->variableName(), "b");

        const auto fourthCondition = std::dynamic_pointer_cast<VariableCondition>(secondConditionItems[1]);
        QVERIFY(fourthCondition);
        QCOMPARE(fourthCondition->variableName(), "c");
    }

    void invalidPropertyValue__throwsExceptionAtCorrectPosition()
    {
        const auto node = Node::create(R"(
            - instant: _
              gestures:
                - type: press
        )");

        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(parseTriggerList<MouseTrigger>(node.get()), InvalidValueConfigException, 1, 23);
    }

    void unusedProperty__addsConfigIssueAtCorrectPosition()
    {
        const auto node = Node::create(R"(
            - _: _
              gestures:
                - type: press
        )");

        parseTriggerList<MouseTrigger>(node.get());
        INPUTACTIONS_VERIFY_ADDS_CONFIG_ISSUE_SAVE(node->addUnusedMapPropertyIssues(), UnusedPropertyConfigIssue, 1, 14, issue);
        QCOMPARE(issue->property(), "_");
    }
};

}

QTEST_MAIN(InputActions::TestTriggerGroupNodeParser)
#include "TestTriggerGroupNodeParser.moc"