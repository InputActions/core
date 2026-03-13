#include "Test.h"
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/triggers/core/StrokeTriggerCore.h>

namespace InputActions
{

class TestStrokeTriggerCoreNodeParser : public Test
{
    Q_OBJECT

private slots:
    void invalidStroke_throwsInvalidValueConfigException()
    {
        const auto node = Node::create("strokes: [ 'MgAAMjJkZA=' ]");

        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::unique_ptr<StrokeTriggerCore>>(), InvalidValueConfigException, 0, 11);
    }
};

}

QTEST_MAIN(InputActions::TestStrokeTriggerCoreNodeParser)
#include "TestStrokeTriggerCoreNodeParser.moc"