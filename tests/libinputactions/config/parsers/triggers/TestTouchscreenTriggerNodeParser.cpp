#include "Test.h"
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/ConfigIssueManager.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/triggers/touchscreen/TouchscreenCircleTrigger.h>
#include <libinputactions/triggers/touchscreen/TouchscreenHoldTrigger.h>
#include <libinputactions/triggers/touchscreen/TouchscreenPinchTrigger.h>
#include <libinputactions/triggers/touchscreen/TouchscreenRotateTrigger.h>
#include <libinputactions/triggers/touchscreen/TouchscreenStrokeTrigger.h>
#include <libinputactions/triggers/touchscreen/TouchscreenSwipeTrigger.h>
#include <libinputactions/triggers/touchscreen/TouchscreenTapTrigger.h>

namespace InputActions
{

class TestTouchscreenTriggerNodeParser : public Test
{
    Q_OBJECT

private slots:
    void circle__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            type: circle
            direction: any
            speed: fast
            id: _
        )");

        const auto trigger = node->as<std::unique_ptr<TouchscreenTrigger>>();
        QVERIFY(dynamic_cast<const TouchscreenCircleTrigger *>(trigger.get()));

        INPUTACTIONS_VERIFY_ADDS_NO_CONFIG_ISSUE(node->addUnusedMapPropertyIssues());
    }

    void hold__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            type: hold
            id: _
        )");

        const auto trigger = node->as<std::unique_ptr<TouchscreenTrigger>>();
        QVERIFY(dynamic_cast<const TouchscreenHoldTrigger *>(trigger.get()));

        INPUTACTIONS_VERIFY_ADDS_NO_CONFIG_ISSUE(node->addUnusedMapPropertyIssues());
    }

    void hold_typePress__parsesNodeCorrectly()
    {
        const auto node = Node::create("type: press");

        const auto trigger = node->as<std::unique_ptr<TouchscreenTrigger>>();
        QVERIFY(dynamic_cast<const TouchscreenHoldTrigger *>(trigger.get()));
    }

    void pinch__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            type: pinch
            direction: any
            speed: fast
            id: _
        )");

        const auto trigger = node->as<std::unique_ptr<TouchscreenTrigger>>();
        QVERIFY(dynamic_cast<const TouchscreenPinchTrigger *>(trigger.get()));

        INPUTACTIONS_VERIFY_ADDS_NO_CONFIG_ISSUE(node->addUnusedMapPropertyIssues());
    }

    void rotate__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            type: rotate
            direction: any
            speed: fast
            id: _
        )");

        const auto trigger = node->as<std::unique_ptr<TouchscreenTrigger>>();
        QVERIFY(dynamic_cast<const TouchscreenRotateTrigger *>(trigger.get()));

        INPUTACTIONS_VERIFY_ADDS_NO_CONFIG_ISSUE(node->addUnusedMapPropertyIssues());
    }

    void stroke__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            type: stroke
            strokes: [ 'MgAAMjJkZAA=' ]
            speed: fast
            id: _
        )");

        const auto trigger = node->as<std::unique_ptr<TouchscreenTrigger>>();
        QVERIFY(dynamic_cast<const TouchscreenStrokeTrigger *>(trigger.get()));

        INPUTACTIONS_VERIFY_ADDS_NO_CONFIG_ISSUE(node->addUnusedMapPropertyIssues());
    }

    void swipe__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            type: swipe
            direction: any
            speed: fast
            id: _
        )");

        const auto trigger = node->as<std::unique_ptr<TouchscreenTrigger>>();
        QVERIFY(dynamic_cast<const TouchscreenSwipeTrigger *>(trigger.get()));

        INPUTACTIONS_VERIFY_ADDS_NO_CONFIG_ISSUE(node->addUnusedMapPropertyIssues());
    }

    void tap__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            type: tap
            id: _
        )");

        const auto trigger = node->as<std::unique_ptr<TouchscreenTrigger>>();
        QVERIFY(dynamic_cast<const TouchscreenTapTrigger *>(trigger.get()));

        INPUTACTIONS_VERIFY_ADDS_NO_CONFIG_ISSUE(node->addUnusedMapPropertyIssues());
    }

    void noType__throwsMissingRequiredPropertyConfigException()
    {
        const auto node = Node::create("_: _");

        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION_SAVE(node->as<std::unique_ptr<TouchscreenTrigger>>(), MissingRequiredPropertyConfigException, 0, 0, e);
        QCOMPARE(e->property(), "type");
    }

    void invalidType__throwsInvalidValueConfigException()
    {
        const auto node = Node::create("type: _");
        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::unique_ptr<TouchscreenTrigger>>(), InvalidValueConfigException, 0, 6);
    }
};

}

QTEST_MAIN(InputActions::TestTouchscreenTriggerNodeParser)
#include "TestTouchscreenTriggerNodeParser.moc"