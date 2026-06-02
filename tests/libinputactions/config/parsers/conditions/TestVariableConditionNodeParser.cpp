#include "Test.h"
#include <libinputactions/conditions/VariableCondition.h>
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/config/parsers/core.h>
#include <libinputactions/variables/VariableRegistry.h>

namespace InputActions
{

class TestVariableConditionNodeParser : public Test
{
    Q_OBJECT

private slots:
    void init()
    {
        g_variableRegistry = std::make_shared<VariableRegistry>();
        g_variableRegistry->registerStored<bool>("bool");
        g_variableRegistry->registerStored<qreal>("number");
        g_variableRegistry->registerStored<PointF>("point");
        g_variableRegistry->registerStored<QString>("string");
        g_variableRegistry->registerStored<Qt::KeyboardModifiers>("keyboard_modifiers");
    }

    void boolVariableWithoutOperator__parsesNodeCorrectly()
    {
        const auto node = Node::create("$bool");
        const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

        QVERIFY(condition);
        QCOMPARE(condition->negate(), false);
        QCOMPARE(condition->variableName(), "bool");
        QCOMPARE(condition->comparisonOperator(), ComparisonOperator::EqualTo);

        const auto &values = condition->values();
        QCOMPARE(values.size(), 1);
        INPUTACTIONS_COMPARE_VARIANT(values[0].get().value(), bool, true);
    }

    void negatedBoolVariableWithoutOperator__parsesNodeCorrectly()
    {
        const auto node = Node::create("_: !$bool")->at("_")->shared_from_this();
        const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

        QVERIFY(condition);
        QCOMPARE(condition->negate(), true);
        QCOMPARE(condition->variableName(), "bool");
        QCOMPARE(condition->comparisonOperator(), ComparisonOperator::EqualTo);

        const auto &values = condition->values();
        QCOMPARE(values.size(), 1);
        INPUTACTIONS_COMPARE_VARIANT(values[0].get().value(), bool, true);
    }

    void negated__parsesNodeCorrectly()
    {
        const auto node = Node::create("!$number == 1");
        const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

        QVERIFY(condition);
        QCOMPARE(condition->negate(), true);
        QCOMPARE(condition->variableName(), "number");
        QCOMPARE(condition->comparisonOperator(), ComparisonOperator::EqualTo);

        const auto &values = condition->values();
        QCOMPARE(values.size(), 1);
        INPUTACTIONS_COMPARE_VARIANT(values[0].get().value(), qreal, 1);
    }

    void between__parsesNodeCorrectly()
    {
        const auto node = Node::create("$number between 1;2");
        const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

        QVERIFY(condition);
        QCOMPARE(condition->negate(), false);
        QCOMPARE(condition->variableName(), "number");
        QCOMPARE(condition->comparisonOperator(), ComparisonOperator::Between);

        const auto &values = condition->values();
        QCOMPARE(values.size(), 2);
        INPUTACTIONS_COMPARE_VARIANT(values[0].get().value(), qreal, 1);
        INPUTACTIONS_COMPARE_VARIANT(values[1].get().value(), qreal, 2);
    }

    void between_point__parsesNodeCorrectly()
    {
        const auto node = Node::create("$point between 0.1,0.2;0.3,0.4");
        const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

        QVERIFY(condition);
        QCOMPARE(condition->negate(), false);
        QCOMPARE(condition->variableName(), "point");
        QCOMPARE(condition->comparisonOperator(), ComparisonOperator::Between);

        const auto &values = condition->values();
        QCOMPARE(values.size(), 2);
        INPUTACTIONS_COMPARE_VARIANT(values[0].get().value(), PointF, PointF(0.1, 0.2));
        INPUTACTIONS_COMPARE_VARIANT(values[1].get().value(), PointF, PointF(0.3, 0.4));
    }

    void between_invalid_oneValue__throwsInvalidValueConfigException()
    {
        const auto node = Node::create("$number between 1");
        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::shared_ptr<Condition>>(), InvalidValueConfigException, 0, 16);
    }

    void between_invalid_threeValues__throwsInvalidValueConfigException()
    {
        const auto node = Node::create("$number between 1;2;3");
        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::shared_ptr<Condition>>(), InvalidValueConfigException, 0, 16);
    }

    void contains_string__parsesNodeCorrectly()
    {
        const auto node = Node::create("$string contains a");
        const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

        QVERIFY(condition);
        QCOMPARE(condition->negate(), false);
        QCOMPARE(condition->variableName(), "string");
        QCOMPARE(condition->comparisonOperator(), ComparisonOperator::Contains);

        const auto &values = condition->values();
        QCOMPARE(values.size(), 1);
        INPUTACTIONS_COMPARE_VARIANT(values[0].get().value(), QString, "a");
    }

    void contains_flags_sequence__parsesNodeCorrectly()
    {
        const auto node = Node::create("$keyboard_modifiers contains [ ctrl, meta ]");
        const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

        QVERIFY(condition);
        QCOMPARE(condition->negate(), false);
        QCOMPARE(condition->variableName(), "keyboard_modifiers");
        QCOMPARE(condition->comparisonOperator(), ComparisonOperator::Contains);

        const auto &values = condition->values();
        QCOMPARE(values.size(), 1);
        INPUTACTIONS_COMPARE_VARIANT(values[0].get().value(),
                                     Qt::KeyboardModifiers,
                                     Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::MetaModifier);
    }

    void contains_flags_scalar__parsesNodeCorrectly()
    {
        const auto node = Node::create("$keyboard_modifiers contains meta");
        const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

        QVERIFY(condition);
        QCOMPARE(condition->negate(), false);
        QCOMPARE(condition->variableName(), "keyboard_modifiers");
        QCOMPARE(condition->comparisonOperator(), ComparisonOperator::Contains);

        const auto &values = condition->values();
        QCOMPARE(values.size(), 1);
        INPUTACTIONS_COMPARE_VARIANT(values[0].get().value(), Qt::KeyboardModifiers, Qt::KeyboardModifier::MetaModifier);
    }

    void oneOf_sequence__parsesNodeCorrectly()
    {
        const auto node = Node::create("$string one_of [ a, b ]");
        const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

        QVERIFY(condition);
        QCOMPARE(condition->negate(), false);
        QCOMPARE(condition->variableName(), "string");
        QCOMPARE(condition->comparisonOperator(), ComparisonOperator::OneOf);

        const auto &values = condition->values();
        QCOMPARE(values.size(), 2);
        INPUTACTIONS_COMPARE_VARIANT(values[0].get().value(), QString, "a");
        INPUTACTIONS_COMPARE_VARIANT(values[1].get().value(), QString, "b");
    }

    void oneOf_scalar__parsesNodeCorrectly()
    {
        const auto node = Node::create("$string one_of a");
        const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

        QVERIFY(condition);
        QCOMPARE(condition->negate(), false);
        QCOMPARE(condition->variableName(), "string");
        QCOMPARE(condition->comparisonOperator(), ComparisonOperator::OneOf);

        const auto &values = condition->values();
        QCOMPARE(values.size(), 1);
        INPUTACTIONS_COMPARE_VARIANT(values[0].get().value(), QString, "a");
    }

    void matches__parsesNodeCorrectly()
    {
        const auto node = Node::create("$string matches \"[a]\"");
        const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

        QVERIFY(condition);
        QCOMPARE(condition->negate(), false);
        QCOMPARE(condition->variableName(), "string");
        QCOMPARE(condition->comparisonOperator(), ComparisonOperator::Regex);

        const auto &values = condition->values();
        QCOMPARE(values.size(), 1);
        INPUTACTIONS_COMPARE_VARIANT(values[0].get().value(), QString, "[a]");
    }

    void matches_invalidRegex__throwsInvalidValueConfigException()
    {
        const auto node = Node::create("$string matches (");
        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::shared_ptr<Condition>>(), InvalidValueConfigException, 0, 16);
    }

    void simpleOperators__parsesNodeCorrectly_data()
    {
        QTest::addColumn<ComparisonOperator>("comparisonOperator");
        QTest::addColumn<QString>("raw");

        QTest::addRow("==") << ComparisonOperator::EqualTo << "==";
        QTest::addRow("!=") << ComparisonOperator::NotEqualTo << "!=";
        QTest::addRow(">") << ComparisonOperator::GreaterThan << ">";
        QTest::addRow(">=") << ComparisonOperator::GreaterThanOrEqual << ">=";
        QTest::addRow("<") << ComparisonOperator::LessThan << "<";
        QTest::addRow("<=") << ComparisonOperator::LessThanOrEqual << "<=";
    }

    void simpleOperators__parsesNodeCorrectly()
    {
        QFETCH(ComparisonOperator, comparisonOperator);
        QFETCH(QString, raw);

        const auto node = Node::create(QString("$number %1 1").arg(raw));
        const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

        QVERIFY(condition);
        QCOMPARE(condition->negate(), false);
        QCOMPARE(condition->variableName(), "number");
        QCOMPARE(condition->comparisonOperator(), comparisonOperator);

        const auto &values = condition->values();
        QCOMPARE(values.size(), 1);
        INPUTACTIONS_COMPARE_VARIANT(values[0].get().value(), qreal, 1);
    }

    void inGroups__variableRegistryPropagated_doesNotThrow()
    {
        const auto node = Node::create(R"(
            all:
              - any:
                  - none:
                      - $b
        )");

        VariableRegistry variableRegistry;
        variableRegistry.registerStored<bool>("b");

        QVERIFY_THROWS_NO_EXCEPTION(parseCondition(node.get(), &variableRegistry));
    }

    void invalid_invalidVariable__throwsInvalidValueConfigException()
    {
        const auto node = Node::create("$_");

        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION_SAVE(node->as<std::shared_ptr<Condition>>(), InvalidVariableConfigException, 0, 0, e);
        QCOMPARE(e->variableName(), "_");
    }

    void invalid_noOperator__throwsInvalidValueConfigException()
    {
        const auto node = Node::create("$number");
        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::shared_ptr<Condition>>(), InvalidValueConfigException, 0, 1);
    }

    void invalid_noValue__throwsInvalidValueConfigException()
    {
        const auto node = Node::create("$number ==");
        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::shared_ptr<Condition>>(), InvalidValueConfigException, 0, 0);
    }
};

}

QTEST_MAIN(InputActions::TestVariableConditionNodeParser)
#include "TestVariableConditionNodeParser.moc"