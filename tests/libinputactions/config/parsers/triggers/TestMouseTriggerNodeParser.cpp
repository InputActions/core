#include "Test.h"
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/ConfigIssueManager.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/triggers/mouse/MouseCircleTrigger.h>
#include <libinputactions/triggers/mouse/MousePressTrigger.h>
#include <libinputactions/triggers/mouse/MouseStrokeTrigger.h>
#include <libinputactions/triggers/mouse/MouseSwipeTrigger.h>
#include <libinputactions/triggers/mouse/MouseWheelTrigger.h>

namespace InputActions
{

class TestMouseTriggerNodeParser : public Test
{
    Q_OBJECT

private slots:
    void circle__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            type: circle
            direction: any
            mouse_buttons: [ left ]
            mouse_buttons_exact_order: true
            lock_pointer: true
            speed: fast
            id: _
        )");

        const auto trigger = node->as<std::unique_ptr<MouseTrigger>>();
        QVERIFY(dynamic_cast<const MouseCircleTrigger *>(trigger.get()));

        INPUTACTIONS_VERIFY_ADDS_NO_CONFIG_ISSUE(node->addUnusedMapPropertyIssues());
    }

    void press__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            type: press
            instant: true
            mouse_buttons: [ left ]
            mouse_buttons_exact_order: true
            id: _
        )");

        const auto trigger = node->as<std::unique_ptr<MouseTrigger>>();
        QVERIFY(dynamic_cast<const MousePressTrigger *>(trigger.get()));

        INPUTACTIONS_VERIFY_ADDS_NO_CONFIG_ISSUE(node->addUnusedMapPropertyIssues());
    }

    void stroke__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            type: stroke
            strokes: [ 'MgAAMjJkZAA=' ]
            mouse_buttons: [ left ]
            mouse_buttons_exact_order: true
            speed: fast
            id: _
        )");

        const auto trigger = node->as<std::unique_ptr<MouseTrigger>>();
        QVERIFY(dynamic_cast<const MouseStrokeTrigger *>(trigger.get()));

        INPUTACTIONS_VERIFY_ADDS_NO_CONFIG_ISSUE(node->addUnusedMapPropertyIssues());
    }

    void swipe__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            type: swipe
            direction: any
            mouse_buttons: [ left ]
            mouse_buttons_exact_order: true
            lock_pointer: true
            speed: fast
            id: _
        )");

        const auto trigger = node->as<std::unique_ptr<MouseTrigger>>();
        QVERIFY(dynamic_cast<const MouseSwipeTrigger *>(trigger.get()));

        INPUTACTIONS_VERIFY_ADDS_NO_CONFIG_ISSUE(node->addUnusedMapPropertyIssues());
    }

    void wheel__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            type: wheel
            direction: any
            mouse_buttons: [ left ]
            mouse_buttons_exact_order: true
            lock_pointer: true
            id: _
        )");

        const auto trigger = node->as<std::unique_ptr<MouseTrigger>>();
        QVERIFY(dynamic_cast<const MouseWheelTrigger *>(trigger.get()));

        INPUTACTIONS_VERIFY_ADDS_NO_CONFIG_ISSUE(node->addUnusedMapPropertyIssues());
    }

    void noType__throwsMissingRequiredPropertyConfigException()
    {
        const auto node = Node::create("_: _");

        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION_SAVE(node->as<std::unique_ptr<MouseTrigger>>(), MissingRequiredPropertyConfigException, 0, 0, e);
        QCOMPARE(e->property(), "type");
    }

    void invalidType__throwsInvalidValueConfigException()
    {
        const auto node = Node::create("type: _");
        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::unique_ptr<MouseTrigger>>(), InvalidValueConfigException, 0, 6);
    }

    void mouseButtons_duplicateItem__throwsDuplicateSetItemConfigException()
    {
        const auto node = Node::create(R"(
            type: press
            mouse_buttons: [ left, left ]
        )");

        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION_SAVE(node->as<std::unique_ptr<MouseTrigger>>(), DuplicateSetItemConfigException, 2, 35, e);
        QCOMPARE(e->index(), 1);
    }

    void mouseButtons__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            type: press
            mouse_buttons: [ left, right ]
        )");

        const auto trigger = node->as<std::unique_ptr<MouseTrigger>>();
        QCOMPARE(trigger->mouseButtons().size(), 2);
        QCOMPARE(trigger->mouseButtons()[0], BTN_LEFT);
        QCOMPARE(trigger->mouseButtons()[1], BTN_RIGHT);
        QCOMPARE(trigger->mouseButtonsExactOrder(), false);
    }

    void mouseButtons_exactOrder__parsesNodeCorrectly()
    {
        const auto node = Node::create(R"(
            type: press
            mouse_buttons: [ left, right ]
            mouse_buttons_exact_order: true
        )");

        const auto trigger = node->as<std::unique_ptr<MouseTrigger>>();
        QCOMPARE(trigger->mouseButtons().size(), 2);
        QCOMPARE(trigger->mouseButtons()[0], BTN_LEFT);
        QCOMPARE(trigger->mouseButtons()[1], BTN_RIGHT);
        QCOMPARE(trigger->mouseButtonsExactOrder(), true);
    }
};

}

QTEST_MAIN(InputActions::TestMouseTriggerNodeParser)
#include "TestMouseTriggerNodeParser.moc"