#include "Test.h"
#include <libinputactions/actions/Action.h>
#include <libinputactions/actions/TriggerAction.h>
#include <libinputactions/triggers/Trigger.h>

namespace InputActions
{

class TestTrigger : public Test
{
    Q_OBJECT

private slots:
    void init() { m_trigger = std::make_unique<Trigger>(); }

    void canActivate_mouseButtons_data()
    {
        QTest::addColumn<std::optional<std::vector<MouseButton>>>("triggerButtons");
        QTest::addColumn<std::optional<std::vector<MouseButton>>>("eventButtons");
        QTest::addColumn<bool>("orderMatters");
        QTest::addColumn<bool>("result");

        const auto unset = std::optional<std::vector<MouseButton>>();
        const auto none = std::optional<std::vector<MouseButton>>(std::vector<MouseButton>());
        const auto left = std::optional<std::vector<MouseButton>>{{BTN_LEFT}};
        const auto right = std::optional<std::vector<MouseButton>>{{BTN_RIGHT}};
        const auto leftRight = std::optional<std::vector<MouseButton>>{{BTN_LEFT, BTN_RIGHT}};
        const auto rightLeft = std::optional<std::vector<MouseButton>>{{BTN_RIGHT, BTN_LEFT}};

        QTest::newRow("1") << left << unset << false << true;
        QTest::newRow("3") << left << left << false << true;
        QTest::newRow("4") << right << left << false << false;
        QTest::newRow("5") << left << right << false << false;
        QTest::newRow("6") << leftRight << left << false << false;
        QTest::newRow("7") << left << leftRight << false << false;

        QTest::newRow("8") << left << unset << true << true;
        QTest::newRow("9") << left << left << true << true;
        QTest::newRow("10") << right << left << true << false;
        QTest::newRow("11") << left << right << true << false;
        QTest::newRow("12") << leftRight << left << true << false;
        QTest::newRow("13") << left << leftRight << true << false;

        QTest::newRow("14") << leftRight << leftRight << false << true;
        QTest::newRow("15") << leftRight << rightLeft << false << true;
        QTest::newRow("16") << rightLeft << leftRight << false << true;

        QTest::newRow("17") << leftRight << rightLeft << true << false;
        QTest::newRow("18") << rightLeft << leftRight << true << false;

        QTest::newRow("19") << none << unset << false << true;
        QTest::newRow("20") << none << left << false << true;
        QTest::newRow("21") << none << unset << true << true;
        QTest::newRow("22") << none << left << true << true;
    }

    void canActivate_mouseButtons()
    {
        QFETCH(std::optional<std::vector<MouseButton>>, triggerButtons);
        QFETCH(std::optional<std::vector<MouseButton>>, eventButtons);
        QFETCH(bool, orderMatters);
        QFETCH(bool, result);

        TriggerActivationEvent event;
        m_trigger->setMouseButtons(triggerButtons.value());
        m_trigger->setMouseButtonsExactOrder(orderMatters);
        event.setMouseButtons(eventButtons);
        QCOMPARE(m_trigger->canActivate(event), result);
    }

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

        auto action = std::make_unique<TriggerAction>();
        const auto *actionRaw = action.get();
        action->setOn(On::Begin);
        m_trigger->addAction(std::move(action));
        if (threshold) {
            m_trigger->setThreshold(threshold.value());
        }

        TriggerUpdateEvent event;
        for (const auto &delta : deltas) {
            event.setDelta(delta);
            m_trigger->update(event);
        }
        QCOMPARE(actionRaw->action()->executions(), actionExecuted);
    }

private:
    std::unique_ptr<Trigger> m_trigger;
};

}

QTEST_MAIN(InputActions::TestTrigger)
#include "TestTrigger.moc"