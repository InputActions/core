#include "Test.h"
#include "mocks/MockTriggerHandler.h"
#include <libinputactions/conditions/CustomCondition.h>
#include <libinputactions/handlers/TriggerHandler.h>
#include <libinputactions/triggers/touchpad/TouchpadClickTrigger.h>
#include <libinputactions/triggers/touchpad/TouchpadHoldTrigger.h>

using namespace ::testing;

namespace InputActions
{

class TestTriggerHandler : public Test
{
    Q_OBJECT

private slots:
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

        TriggerHandler handler;
        for (auto *trigger : triggers) {
            handler.addTrigger(std::unique_ptr<Trigger>(trigger));
        }
        TriggerActivationEvent event;

        QCOMPARE(handler.triggers(type, event).size(), size);
        QCOMPARE(handler.activateTriggers(type, event).success, size != 0);
        QCOMPARE(handler.activeTriggers(type).size(), size);
    }

    void activateTriggers_cancelsAllTriggers()
    {
        MockTriggerHandler handler;
        handler.addTrigger(std::make_unique<TouchpadClickTrigger>());

        EXPECT_CALL(handler, doCancelTriggers(_)).Times(0);
        handler.activateTriggers(TriggerType::Press);
        handler.activateTriggers(TriggerType::Press | TriggerType::Click);
        QVERIFY(Mock::VerifyAndClearExpectations(&handler));

        EXPECT_CALL(handler, doCancelTriggers(static_cast<TriggerTypes>(TriggerType::All))).Times(1);
        handler.activateTriggers(TriggerType::All);
        QVERIFY(Mock::VerifyAndClearExpectations(&handler));
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
};

}

QTEST_MAIN(InputActions::TestTriggerHandler)
#include "TestTriggerHandler.moc"