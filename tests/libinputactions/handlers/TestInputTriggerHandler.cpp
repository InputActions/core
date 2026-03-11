#include "Test.h"
#include <QSignalSpy>
#include <libinputactions/handlers/InputTriggerHandler.h>
#include <libinputactions/input/backends/InputBackend.h>
#include <libinputactions/input/devices/InputDevice.h>
#include <libinputactions/input/events.h>
#include <libinputactions/triggers/Trigger.h>
#include <linux/input-event-codes.h>

namespace InputActions
{

class TestInputTriggerHandler : public Test
{
    Q_OBJECT

private slots:
    void init()
    {
        m_handler = std::unique_ptr<InputTriggerHandler>(new InputTriggerHandler);

        g_inputBackend = std::make_unique<InputBackend>();
        g_inputBackend->initialize();

        m_keyboard = std::make_unique<InputDevice>(InputDeviceType::Keyboard);
        g_inputBackend->addDevice(m_keyboard.get());
    }

    void keyboardKey__modifierReleased_pressedBeforeTriggerActivation__triggersEnded()
    {
        QSignalSpy spy(m_handler.get(), &TriggerHandler::endingTriggers);
        m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Press));

        g_inputBackend->handleEvent(KeyboardKeyEvent(m_keyboard.get(), KEY_LEFTCTRL, true));
        m_handler->handleEvent(KeyboardKeyEvent(m_keyboard.get(), KEY_LEFTCTRL, true));

        m_handler->activateTriggers(TriggerType::Press);
        QCOMPARE(spy.count(), 0);

        g_inputBackend->handleEvent(KeyboardKeyEvent(m_keyboard.get(), KEY_LEFTCTRL, false));
        m_handler->handleEvent(KeyboardKeyEvent(m_keyboard.get(), KEY_LEFTCTRL, false));
        QCOMPARE(spy.count(), 1);
    }

    void keyboardKey__modifierReleased_pressedAfterTriggerActivation__triggersNotEnded()
    {
        QSignalSpy spy(m_handler.get(), &TriggerHandler::endingTriggers);
        m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Press));

        m_handler->activateTriggers(TriggerType::Press);

        g_inputBackend->handleEvent(KeyboardKeyEvent(m_keyboard.get(), KEY_LEFTCTRL, true));
        m_handler->handleEvent(KeyboardKeyEvent(m_keyboard.get(), KEY_LEFTCTRL, true));

        g_inputBackend->handleEvent(KeyboardKeyEvent(m_keyboard.get(), KEY_LEFTCTRL, false));
        m_handler->handleEvent(KeyboardKeyEvent(m_keyboard.get(), KEY_LEFTCTRL, false));
        QCOMPARE(spy.count(), 0);
    }

private:
    std::unique_ptr<InputDevice> m_keyboard;
    std::unique_ptr<InputTriggerHandler> m_handler;
};

}

QTEST_MAIN(InputActions::TestInputTriggerHandler)
#include "TestInputTriggerHandler.moc"