#include "Test.h"
#include "mocks/MockTrigger.h"
#include <libinputactions/actions/Action.h>
#include <libinputactions/actions/TriggerAction.h>

namespace InputActions
{

class TestTrigger : public Test
{
    Q_OBJECT

private slots:
    void update_threshold_data()
    {
        QTest::addColumn<std::optional<Range<qreal>>>("threshold");
        QTest::addColumn<std::vector<qreal>>("deltas");
        QTest::addColumn<bool>("actionExecuted");

        // total delta > threshold is tested to ensure the begin action only executes once
        QTest::newRow("no threshold, 0 delta") << std::optional<Range<qreal>>() << std::vector<qreal>{0} << true;
        QTest::newRow("total delta < min_threshold") << std::optional<Range<qreal>>(Range<qreal>(3, {})) << std::vector<qreal>{1, 1} << false;
        QTest::newRow("total delta > min_threshold") << std::optional<Range<qreal>>(Range<qreal>(3, {})) << std::vector<qreal>{2, 2} << true;
        QTest::newRow("delta1, delta2 > max_threshold") << std::optional<Range<qreal>>(Range<qreal>({}, 3)) << std::vector<qreal>{4, 4} << false;
        QTest::newRow("total delta > max_threshold") << std::optional<Range<qreal>>(Range<qreal>({}, 3)) << std::vector<qreal>{2, 2} << true;
    }

    void update_threshold()
    {
        QFETCH(std::optional<Range<qreal>>, threshold);
        QFETCH(std::vector<qreal>, deltas);
        QFETCH(bool, actionExecuted);

        MockTrigger trigger;
        if (threshold) {
            trigger.setThreshold(threshold.value());
        }

        auto action = std::make_unique<TriggerAction>();
        const auto *actionRaw = action.get();
        action->setOn(On::Begin);
        trigger.addAction(std::move(action));

        TriggerUpdateEvent event;
        for (const auto &delta : deltas) {
            event.setDelta(delta);
            trigger.update(event);
        }
        QCOMPARE(actionRaw->action()->executions(), actionExecuted);
    }
};

}

QTEST_MAIN(InputActions::TestTrigger)
#include "TestTrigger.moc"