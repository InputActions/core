#include "Test.h"
#include <QSignalSpy>
#include <libinputactions/conditions/CustomCondition.h>
#include <libinputactions/handlers/TriggerHandler.h>
#include <libinputactions/triggers/touchpad/TouchpadClickTrigger.h>
#include <libinputactions/triggers/touchpad/TouchpadHoldTrigger.h>

namespace InputActions
{

class TestTriggerHandler : public Test
{
    Q_OBJECT

private slots:
    void init() { m_handler = std::unique_ptr<TriggerHandler>(new TriggerHandler); }

    void triggers_data()
    {
        QTest::addColumn<TriggerType>("type");
        QTest::addColumn<std::vector<Trigger *>>("triggers");
        QTest::addColumn<int>("size");

        QTest::newRow("not activatable") << TriggerType::Click << std::vector<Trigger *>({makeTrigger<TouchpadClickTrigger>(false)}) << 0;
        QTest::newRow("activatable") << TriggerType::Click << std::vector<Trigger *>({makeTrigger<TouchpadClickTrigger>(true)}) << 1;
        QTest::newRow("activatable, wrong type") << TriggerType::Press << std::vector<Trigger *>({makeTrigger<TouchpadClickTrigger>(true)}) << 0;
        QTest::newRow("activatable, all") << TriggerType::All
                                          << std::vector<Trigger *>({makeTrigger<TouchpadClickTrigger>(true), makeTrigger<TouchpadHoldTrigger>(true)}) << 2;
    }

    void triggers()
    {
        QFETCH(TriggerType, type);
        QFETCH(std::vector<Trigger *>, triggers);
        QFETCH(int, size);

        for (auto trigger : triggers) {
            m_handler->addTrigger(std::unique_ptr<Trigger>(trigger));
        }
        TriggerActivationEvent event;

        QCOMPARE(m_handler->triggers(type, event).size(), size);
        QCOMPARE(m_handler->activateTriggers(type, event).success, size != 0);
        QCOMPARE(m_handler->activeTriggers(type).size(), size);
    }

    void activateTriggers_cancelsAllTriggers()
    {
        QSignalSpy spy(m_handler.get(), &TriggerHandler::cancellingTriggers);

        m_handler->addTrigger(std::make_unique<TouchpadClickTrigger>());
        m_handler->activateTriggers(TriggerType::Press);
        QCOMPARE(spy.count(), 0);
        m_handler->activateTriggers(TriggerType::Press | TriggerType::Click);
        QCOMPARE(spy.count(), 0);
        m_handler->activateTriggers(TriggerType::All);
        QCOMPARE(spy.count(), 1);

        for (const auto &args : spy) {
            QCOMPARE(args.at(0).value<TriggerTypes>(), TriggerType::All);
        }
    }

private:
    template<typename T>
    T *makeTrigger(bool activatable)
    {
        auto *trigger = new T();
        trigger->setActivationCondition(std::make_unique<CustomCondition>([activatable](const auto &arguments) {
            return activatable;
        }));
        return trigger;
    }

    std::unique_ptr<TriggerHandler> m_handler;
};

}

QTEST_MAIN(InputActions::TestTriggerHandler)
#include "TestTriggerHandler.moc"