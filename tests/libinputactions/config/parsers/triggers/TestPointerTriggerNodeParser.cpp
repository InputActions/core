#include "Test.h"
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/ConfigIssueManager.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/triggers/pointer/PointerHoverTrigger.h>

namespace InputActions
{

class TestPointerTriggerNodeParser : public Test
{
    Q_OBJECT

private slots:
    void hover__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            type: hover
            id: _
        )");

        const auto trigger = node->as<std::unique_ptr<PointerTrigger>>();
        QVERIFY(dynamic_cast<const PointerHoverTrigger *>(trigger.get()));

        INPUTACTIONS_VERIFY_ADDS_NO_CONFIG_ISSUE(node->addUnusedMapPropertyIssues());
    }

    void noType__throwsMissingRequiredPropertyConfigException()
    {
        const auto node = Node::create("_: _");

        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION_SAVE(node->as<std::unique_ptr<PointerTrigger>>(), MissingRequiredPropertyConfigException, 0, 0, e);
        QCOMPARE(e->property(), "type");
    }

    void invalidType__throwsInvalidValueConfigException()
    {
        const auto node = Node::create("type: _");
        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::unique_ptr<PointerTrigger>>(), InvalidValueConfigException, 0, 6);
    }
};

}

QTEST_MAIN(InputActions::TestPointerTriggerNodeParser)
#include "TestPointerTriggerNodeParser.moc"