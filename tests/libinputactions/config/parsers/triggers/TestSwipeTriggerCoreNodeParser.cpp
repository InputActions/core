#include "Test.h"
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/triggers/core/SwipeTriggerCore.h>

namespace InputActions
{

class TestSwipeTriggerCoreNodeParser : public Test
{
    Q_OBJECT

private slots:
    void angle__parsesNodeCorrectly()
    {
        const auto node = Node::create("angle: 30-60");

        const auto core = node->as<std::unique_ptr<SwipeTriggerCore>>();
        QCOMPARE(core->minAngle(), 30);
        QCOMPARE(core->maxAngle(), 60);
        QCOMPARE(core->bidirectional(), false);
    }

    void angle_bidirectional__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            angle: 30-60
            bidirectional: true
        )");

        const auto core = node->as<std::unique_ptr<SwipeTriggerCore>>();
        QCOMPARE(core->minAngle(), 30);
        QCOMPARE(core->maxAngle(), 60);
        QCOMPARE(core->bidirectional(), true);
    }

    void direction__doesNotThrow()
    {
        const auto node = Node::create("direction: right");

        QVERIFY_THROWS_NO_EXCEPTION(node->as<std::unique_ptr<SwipeTriggerCore>>());
    }

    void invalidAngle__throwsInvalidValueConfigException()
    {
        const auto node = Node::create("angle: 1-361");

        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::unique_ptr<SwipeTriggerCore>>(), InvalidValueConfigException, 0, 7);
    }
};

}

QTEST_MAIN(InputActions::TestSwipeTriggerCoreNodeParser)
#include "TestSwipeTriggerCoreNodeParser.moc"