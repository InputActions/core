#include "Test.h"
#include <libinputactions/actions/ReplaceTextAction.h>
#include <libinputactions/conditions/CanReplaceTextCondition.h>
#include <libinputactions/config/Node.h>

namespace InputActions
{

class TestCanReplaceTextConditionNodeParser : public Test
{
    Q_OBJECT

private slots:
    void valid__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            can_replace_text:
              - regex: a
                replace: _
        )");
        const auto condition = std::dynamic_pointer_cast<CanReplaceTextCondition>(node->as<std::shared_ptr<Condition>>());

        QVERIFY(condition);
        QCOMPARE(condition->rules().size(), 1);
        QCOMPARE(condition->rules()[0].regex().pattern(), "a");
    }
};

}

QTEST_MAIN(InputActions::TestCanReplaceTextConditionNodeParser)
#include "TestCanReplaceTextConditionNodeParser.moc"