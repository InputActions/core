#include "Test.h"
#include "mocks/MockInputTriggerHandler.h"
#include <libinputactions/handlers/InputTriggerHandler.h>
#include <libinputactions/input/backends/InputBackend.h>
#include <libinputactions/input/devices/InputDevice.h>
#include <libinputactions/input/events.h>
#include <libinputactions/triggers/Trigger.h>
#include <libinputactions/triggers/mouse/MousePressTrigger.h>
#include <linux/input-event-codes.h>

using namespace ::testing;

namespace InputActions
{

class TestInputTriggerHandler : public Test
{
    Q_OBJECT

private slots:
    void init()
    {
        g_inputBackend = std::make_unique<InputBackend>();
        g_inputBackend->initialize();

        m_keyboard = std::make_unique<InputDevice>(InputDeviceType::Keyboard);
        g_inputBackend->addDevice(m_keyboard.get());
    }

    void keyboardKey__modifierReleased_pressedBeforeTriggerActivation__triggersEnded()
    {
        MockInputTriggerHandler handler;
        handler.addTrigger(std::make_unique<MousePressTrigger>());

        EXPECT_CALL(handler, doEndTriggers(_)).Times(0);
        g_inputBackend->handleEvent(KeyboardKeyEvent(m_keyboard.get(), KEY_LEFTCTRL, true));
        handler.handleEvent(KeyboardKeyEvent(m_keyboard.get(), KEY_LEFTCTRL, true));
        handler.activateTriggers(TriggerType::Press);
        QVERIFY(Mock::VerifyAndClearExpectations(&handler));

        EXPECT_CALL(handler, doEndTriggers(_)).Times(1);
        g_inputBackend->handleEvent(KeyboardKeyEvent(m_keyboard.get(), KEY_LEFTCTRL, false));
        handler.handleEvent(KeyboardKeyEvent(m_keyboard.get(), KEY_LEFTCTRL, false));
        QVERIFY(Mock::VerifyAndClearExpectations(&handler));
    }

    void keyboardKey__modifierReleased_pressedAfterTriggerActivation__triggersNotEnded()
    {
        MockInputTriggerHandler handler;
        handler.addTrigger(std::make_unique<MousePressTrigger>());

        EXPECT_CALL(handler, doEndTriggers(_)).Times(0);

        handler.activateTriggers(TriggerType::Press);

        g_inputBackend->handleEvent(KeyboardKeyEvent(m_keyboard.get(), KEY_LEFTCTRL, true));
        handler.handleEvent(KeyboardKeyEvent(m_keyboard.get(), KEY_LEFTCTRL, true));

        g_inputBackend->handleEvent(KeyboardKeyEvent(m_keyboard.get(), KEY_LEFTCTRL, false));
        handler.handleEvent(KeyboardKeyEvent(m_keyboard.get(), KEY_LEFTCTRL, false));

        QVERIFY(Mock::VerifyAndClearExpectations(&handler));
    }

private:
    std::unique_ptr<InputDevice> m_keyboard;
};

}

QTEST_MAIN(InputActions::TestInputTriggerHandler)
#include "TestInputTriggerHandler.moc"