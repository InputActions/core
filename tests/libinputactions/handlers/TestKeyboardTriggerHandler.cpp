#include "Test.h"
#include "mocks/MockKeyboardTriggerHandler.h"
#include <libinputactions/input/devices/InputDevice.h>
#include <libinputactions/input/events.h>
#include <libinputactions/triggers/keyboard/KeyboardShortcutTrigger.h>
#include <linux/input-event-codes.h>

using namespace ::testing;

namespace InputActions
{

class TestKeyboardTriggerHandler : public Test
{
    Q_OBJECT

private slots:
    void handleEvent_keyboardKey()
    {
        MockKeyboardTriggerHandler handler;
        InputDevice device(InputDeviceType::Keyboard);
        handler.addTrigger(std::make_unique<KeyboardShortcutTrigger>(KeyboardShortcut{.keys = {KEY_LEFTCTRL, KEY_A}}));

        EXPECT_CALL(handler, triggerActivated(_)).Times(0);
        QCOMPARE(handler.handleEvent(KeyboardKeyEvent(&device, KEY_A, true)), false);
        QCOMPARE(handler.handleEvent(KeyboardKeyEvent(&device, KEY_LEFTCTRL, true)), false);

        QCOMPARE(handler.handleEvent(KeyboardKeyEvent(&device, KEY_LEFTCTRL, false)), false);
        QCOMPARE(handler.handleEvent(KeyboardKeyEvent(&device, KEY_A, false)), false);
        QVERIFY(Mock::VerifyAndClearExpectations(&handler));

        EXPECT_CALL(handler, triggerActivated(_)).Times(1);
        EXPECT_CALL(handler, doEndTriggers(_)).Times(0);
        QCOMPARE(handler.handleEvent(KeyboardKeyEvent(&device, KEY_LEFTCTRL, true)), false);
        QCOMPARE(handler.handleEvent(KeyboardKeyEvent(&device, KEY_A, true)), true);
        QVERIFY(Mock::VerifyAndClearExpectations(&handler));

        EXPECT_CALL(handler, triggerActivated(_)).Times(0);
        EXPECT_CALL(handler, doEndTriggers(_)).Times(1);
        QCOMPARE(handler.handleEvent(KeyboardKeyEvent(&device, KEY_A, false)), true);
        QVERIFY(Mock::VerifyAndClearExpectations(&handler));

        EXPECT_CALL(handler, triggerActivated(_)).Times(0);
        EXPECT_CALL(handler, doEndTriggers(_)).Times(0);
        QCOMPARE(handler.handleEvent(KeyboardKeyEvent(&device, KEY_LEFTCTRL, false)), false);
        QVERIFY(Mock::VerifyAndClearExpectations(&handler));
    }
};

}

QTEST_MAIN(InputActions::TestKeyboardTriggerHandler)
#include "TestKeyboardTriggerHandler.moc"