#include "Test.h"
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/ConfigIssueManager.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/triggers/touchpad/TouchpadCircleTrigger.h>
#include <libinputactions/triggers/touchpad/TouchpadClickTrigger.h>
#include <libinputactions/triggers/touchpad/TouchpadHoldTrigger.h>
#include <libinputactions/triggers/touchpad/TouchpadPinchTrigger.h>
#include <libinputactions/triggers/touchpad/TouchpadRotateTrigger.h>
#include <libinputactions/triggers/touchpad/TouchpadStrokeTrigger.h>
#include <libinputactions/triggers/touchpad/TouchpadSwipeTrigger.h>
#include <libinputactions/triggers/touchpad/TouchpadTapTrigger.h>

namespace InputActions
{

class TestTouchpadTriggerNodeParser : public Test
{
    Q_OBJECT

private slots:
    void click__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            type: click
            id: _
        )");

        const auto trigger = node->as<std::unique_ptr<TouchpadTrigger>>();
        QVERIFY(dynamic_cast<const TouchpadClickTrigger *>(trigger.get()));

        INPUTACTIONS_VERIFY_ADDS_NO_CONFIG_ISSUE(node->addUnusedMapPropertyIssues());
    }

    void circle__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            type: circle
            direction: any
            speed: fast
            id: _
        )");

        const auto trigger = node->as<std::unique_ptr<TouchpadTrigger>>();
        QVERIFY(dynamic_cast<const TouchpadCircleTrigger *>(trigger.get()));

        INPUTACTIONS_VERIFY_ADDS_NO_CONFIG_ISSUE(node->addUnusedMapPropertyIssues());
    }

    void hold__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            type: hold
            id: _
        )");

        const auto trigger = node->as<std::unique_ptr<TouchpadTrigger>>();
        QVERIFY(dynamic_cast<const TouchpadHoldTrigger *>(trigger.get()));

        INPUTACTIONS_VERIFY_ADDS_NO_CONFIG_ISSUE(node->addUnusedMapPropertyIssues());
    }

    void hold_typePress__parsesNodeCorrectly()
    {
        const auto node = Node::create("type: press");

        const auto trigger = node->as<std::unique_ptr<TouchpadTrigger>>();
        QVERIFY(dynamic_cast<const TouchpadHoldTrigger *>(trigger.get()));
    }

    void pinch__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            type: pinch
            direction: any
            speed: fast
            id: _
        )");

        const auto trigger = node->as<std::unique_ptr<TouchpadTrigger>>();
        QVERIFY(dynamic_cast<const TouchpadPinchTrigger *>(trigger.get()));

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

        const auto trigger = node->as<std::unique_ptr<TouchpadTrigger>>();
        QVERIFY(dynamic_cast<const TouchpadRotateTrigger *>(trigger.get()));

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

        const auto trigger = node->as<std::unique_ptr<TouchpadTrigger>>();
        QVERIFY(dynamic_cast<const TouchpadStrokeTrigger *>(trigger.get()));

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

        const auto trigger = node->as<std::unique_ptr<TouchpadTrigger>>();
        QVERIFY(dynamic_cast<const TouchpadSwipeTrigger *>(trigger.get()));

        INPUTACTIONS_VERIFY_ADDS_NO_CONFIG_ISSUE(node->addUnusedMapPropertyIssues());
    }

    void tap__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            type: tap
            id: _
        )");

        const auto trigger = node->as<std::unique_ptr<TouchpadTrigger>>();
        QVERIFY(dynamic_cast<const TouchpadTapTrigger *>(trigger.get()));

        INPUTACTIONS_VERIFY_ADDS_NO_CONFIG_ISSUE(node->addUnusedMapPropertyIssues());
    }

    void noType__throwsMissingRequiredPropertyConfigException()
    {
        const auto node = Node::create("_: _");

        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION_SAVE(node->as<std::unique_ptr<TouchpadTrigger>>(), MissingRequiredPropertyConfigException, 0, 0, e);
        QCOMPARE(e->property(), "type");
    }

    void invalidType__throwsInvalidValueConfigException()
    {
        const auto node = Node::create("type: _");
        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::unique_ptr<TouchpadTrigger>>(), InvalidValueConfigException, 0, 6);
    }
};

}

QTEST_MAIN(InputActions::TestTouchpadTriggerNodeParser)
#include "TestTouchpadTriggerNodeParser.moc"