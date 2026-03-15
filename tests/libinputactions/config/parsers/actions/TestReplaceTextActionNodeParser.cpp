#include "Test.h"
#include <libinputactions/actions/ReplaceTextAction.h>
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/Node.h>

namespace InputActions
{

class TestReplaceTextActionNodeParser : public Test
{
    Q_OBJECT

private slots:
    void valid__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            replace_text:
              - regex: a
                replace: b
        )");
        const auto action = node->as<std::unique_ptr<Action>>();

        const auto *replaceTextAction = dynamic_cast<const ReplaceTextAction *>(action.get());
        QVERIFY(replaceTextAction);
        QCOMPARE(replaceTextAction->rules().size(), 1);
        QCOMPARE(replaceTextAction->rules()[0].regex().pattern(), "a");
        QCOMPARE(replaceTextAction->rules()[0].newText().get(), "b");
    }

    void valid_command__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            replace_text:
              - regex: a
                replace:
                  command: echo -n c
        )");
        const auto action = node->as<std::unique_ptr<Action>>();

        const auto *replaceTextAction = dynamic_cast<const ReplaceTextAction *>(action.get());
        QVERIFY(replaceTextAction);
        QCOMPARE(replaceTextAction->rules()[0].newText().get(), "c");
    }
};

}

QTEST_MAIN(InputActions::TestReplaceTextActionNodeParser)
#include "TestReplaceTextActionNodeParser.moc"