#include "Test.h"
#include <libinputactions/actions/CommandAction.h>
#include <libinputactions/actions/DirectionalAction.h>
#include <libinputactions/actions/InputAction.h>
#include <libinputactions/actions/PlasmaGlobalShortcutAction.h>
#include <libinputactions/actions/TriggerAction.h>
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/variables/VariableRegistry.h>

namespace InputActions
{

class TestDirectionalActionNodeParser : public Test
{
    Q_OBJECT

private slots:
    void init()
    {
        g_variableRegistry = std::make_shared<VariableRegistry>();
        g_variableRegistry->registerStored<bool>("a");
    }

    void actions__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            directional:
              actions:
                left:
                  input:
                    - keyboard: [ left ]
                right:
                  plasma_shortcut: kwin,Switch One Desktop to the Right
        )");
        const auto action = node->as<std::unique_ptr<Action>>();

        const auto *directionalAction = dynamic_cast<const DirectionalAction *>(action.get());
        QVERIFY(directionalAction);
        QCOMPARE(directionalAction->distance(), 75);
        QVERIFY(!directionalAction->repeatCondition());

        const auto &actions = directionalAction->actions();
        QVERIFY(dynamic_cast<const InputAction *>(actions[0].get()));
        QVERIFY(dynamic_cast<const PlasmaGlobalShortcutAction *>(actions[1].get()));
        QVERIFY(!actions[2]);
        QVERIFY(!actions[3]);

        for (const auto &initial : directionalAction->initial()) {
            QVERIFY(!initial);
        }
    }

    void allProperties__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            directional:
              distance: 50
              initial:
                up:
                  command: true
              actions:
                down:
                  input:
                    - keyboard: [ down ]
                  conditions: $a
              repeat_conditions: $a
        )");
        const auto action = node->as<std::unique_ptr<Action>>();

        const auto *directionalAction = dynamic_cast<const DirectionalAction *>(action.get());
        QVERIFY(directionalAction);
        QCOMPARE(directionalAction->distance(), 50);
        QVERIFY(directionalAction->repeatCondition());

        const auto *down = directionalAction->actions()[3].get();
        QVERIFY(dynamic_cast<const InputAction *>(down));
        QVERIFY(down->condition());
        QVERIFY(dynamic_cast<const CommandAction *>(directionalAction->initial()[2].get()));
    }

    void noActions__throwsMissingRequiredPropertyConfigException()
    {
        const auto node = Node::create("directional: { distance: 50 }");

        QVERIFY_THROWS_EXCEPTION(MissingRequiredPropertyConfigException, node->as<std::unique_ptr<Action>>());
    }

    void limit__throwsInvalidValueContextConfigException()
    {
        const auto node = Node::create("{ limit: 1, directional: { actions: { left: { command: true } } } }");

        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::unique_ptr<Action>>(), InvalidValueContextConfigException, 0, 9);
    }

    void inGroup__throwsInvalidValueContextConfigException()
    {
        const auto node = Node::create("{ on: update, one: [ { directional: { actions: { left: { command: true } } } } ] }");

        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::unique_ptr<TriggerAction>>(), InvalidValueContextConfigException, 0, 19);
    }

    void triggerAction_interval__throwsInvalidValueContextConfigException()
    {
        const auto node = Node::create("{ on: update, interval: 10, directional: { actions: { left: { command: true } } } }");

        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::unique_ptr<TriggerAction>>(), InvalidValueContextConfigException, 0, 24);
    }

    void triggerAction_notUpdate__throwsInvalidValueContextConfigException_data()
    {
        QTest::addColumn<QString>("raw");
        QTest::addColumn<int>("column");

        QTest::newRow("end") << "{ on: end, directional: { actions: { left: { command: true } } } }" << 6;
        QTest::newRow("default") << "{ directional: { actions: { left: { command: true } } } }" << 0;
    }

    void triggerAction_notUpdate__throwsInvalidValueContextConfigException()
    {
        QFETCH(QString, raw);
        QFETCH(int, column);
        const auto node = Node::create(raw);

        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::unique_ptr<TriggerAction>>(), InvalidValueContextConfigException, 0, column);
    }

    void invalidDistance__throwsInvalidValueConfigException_data()
    {
        QTest::addColumn<QString>("distance");

        QTest::newRow("zero") << "0";
        QTest::newRow("negative") << "-1";
    }

    void invalidDistance__throwsInvalidValueConfigException()
    {
        QFETCH(QString, distance);
        const auto node = Node::create(QString("directional: { distance: %1, actions: { left: { command: true } } }").arg(distance));

        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::unique_ptr<Action>>(), InvalidValueConfigException, 0, 25);
    }
};

}

QTEST_MAIN(InputActions::TestDirectionalActionNodeParser)
#include "TestDirectionalActionNodeParser.moc"