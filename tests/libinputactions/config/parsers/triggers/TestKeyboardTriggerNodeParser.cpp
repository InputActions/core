#include "Test.h"
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/ConfigIssueManager.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/triggers/keyboard/KeyboardShortcutTrigger.h>

namespace InputActions
{

class TestKeyboardTriggerNodeParser : public Test
{
    Q_OBJECT

private slots:
    void shortcut__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            type: shortcut
            shortcut: [ leftmeta, x ]
            id: _
        )");

        const auto trigger = node->as<std::unique_ptr<KeyboardTrigger>>();
        const auto *shortcutTrigger = dynamic_cast<const KeyboardShortcutTrigger *>(trigger.get());

        QVERIFY(shortcutTrigger);
        QCOMPARE(shortcutTrigger->shortcut().keys.size(), 2);
        QCOMPARE(shortcutTrigger->shortcut().keys.contains(KEY_LEFTMETA), true);
        QCOMPARE(shortcutTrigger->shortcut().keys.contains(KEY_X), true);

        INPUTACTIONS_VERIFY_ADDS_NO_CONFIG_ISSUE(node->addUnusedMapPropertyIssues());
    }

    void noType__throwsMissingRequiredPropertyConfigException()
    {
        const auto node = Node::create("_: _");

        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION_SAVE(node->as<std::unique_ptr<KeyboardTrigger>>(), MissingRequiredPropertyConfigException, 0, 0, e);
        QCOMPARE(e->property(), "type");
    }

    void invalidType__throwsInvalidValueConfigException()
    {
        const auto node = Node::create("type: _");
        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::unique_ptr<KeyboardTrigger>>(), InvalidValueConfigException, 0, 6);
    }
};

}

QTEST_MAIN(InputActions::TestKeyboardTriggerNodeParser)
#include "TestKeyboardTriggerNodeParser.moc"