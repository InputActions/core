#include "Test.h"
#include <libinputactions/conditions/Condition.h>
#include <libinputactions/config/Node.h>

namespace InputActions
{

class TestConditionNodeParser : public Test
{
    Q_OBJECT

private slots:
    void invalid_map__throwsInvalidValueConfigException()
    {
        const auto node = Node::create("a:");
        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::shared_ptr<Condition>>(), InvalidValueConfigException, 0, 0);
    }

    void invalid_scalar__throwsInvalidValueConfigException()
    {
        const auto node = Node::create("a");
        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::shared_ptr<Condition>>(), InvalidValueConfigException, 0, 0);
    }
};

}

QTEST_MAIN(InputActions::TestConditionNodeParser)
#include "TestConditionNodeParser.moc"