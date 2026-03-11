#include "Test.h"
#include <libinputactions/triggers/core/MotionTriggerCore.h>

namespace InputActions
{

class TestMotionTriggerCore : public Test
{
    Q_OBJECT

private slots:
    void canUpdate_speed_data()
    {
        QTest::addColumn<TriggerSpeed>("triggerSpeed");
        QTest::addColumn<TriggerSpeed>("eventSpeed");
        QTest::addColumn<bool>("result");

        QTest::addRow("any, any") << TriggerSpeed::Any << TriggerSpeed::Any << true;
        QTest::addRow("any, fast") << TriggerSpeed::Any << TriggerSpeed::Fast << true;
        QTest::addRow("any, slow") << TriggerSpeed::Any << TriggerSpeed::Slow << true;
        QTest::addRow("fast, any") << TriggerSpeed::Fast << TriggerSpeed::Any << false;
        QTest::addRow("fast, fast") << TriggerSpeed::Fast << TriggerSpeed::Fast << true;
        QTest::addRow("fast, slow") << TriggerSpeed::Fast << TriggerSpeed::Slow << false;
        QTest::addRow("slow, any") << TriggerSpeed::Slow << TriggerSpeed::Any << false;
        QTest::addRow("slow, fast") << TriggerSpeed::Slow << TriggerSpeed::Fast << false;
        QTest::addRow("slow, slow") << TriggerSpeed::Slow << TriggerSpeed::Slow << true;
    }

    void canUpdate_speed()
    {
        QFETCH(TriggerSpeed, triggerSpeed);
        QFETCH(TriggerSpeed, eventSpeed);
        QFETCH(bool, result);

        MotionTriggerCore core;
        core.setSpeed(triggerSpeed);
        MotionTriggerUpdateEvent event;
        event.setSpeed(eventSpeed);

        QCOMPARE(core.canUpdate(event), result);
    }
};

}

QTEST_MAIN(InputActions::TestMotionTriggerCore)
#include "TestMotionTriggerCore.moc"