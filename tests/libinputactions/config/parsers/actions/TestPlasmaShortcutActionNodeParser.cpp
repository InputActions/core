#include "Test.h"
#include <libinputactions/actions/PlasmaGlobalShortcutAction.h>
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/Node.h>

namespace InputActions
{

class TestPlasmaShortcutActionNodeParser : public Test
{
    Q_OBJECT

private slots:
    void valid__parsesNodeCorrectly()
    {
        const auto node = Node::create("plasma_shortcut: a,b");
        const auto action = node->as<std::unique_ptr<Action>>();

        const auto *plasmaAction = dynamic_cast<const PlasmaGlobalShortcutAction *>(action.get());
        QVERIFY(plasmaAction);

        QCOMPARE(plasmaAction->component(), "a");
        QCOMPARE(plasmaAction->shortcut(), "b");
    }

    void invalid__throwsInvalidValueConfigException()
    {
        const auto node = Node::create("plasma_shortcut: _");
        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::unique_ptr<Action>>(), InvalidValueConfigException, 0, 17);
    }
};

}

QTEST_MAIN(InputActions::TestPlasmaShortcutActionNodeParser)
#include "TestPlasmaShortcutActionNodeParser.moc"