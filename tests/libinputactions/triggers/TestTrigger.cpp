#include "Test.h"
#include "mocks/MockTrigger.h"
#include <libinputactions/actions/Action.h>
#include <libinputactions/actions/CustomAction.h>
#include <libinputactions/actions/DirectionalAction.h>
#include <libinputactions/actions/InputAction.h>
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

    void addAction_directionalInput__clearsModifiers_data()
    {
        QTest::addColumn<bool>("initial");

        QTest::newRow("actions") << false;
        QTest::newRow("initial") << true;
    }

    void addAction_directionalInput__clearsModifiers()
    {
        QFETCH(bool, initial);
        DirectionalAction::Bindings actions;
        DirectionalAction::Bindings initialActions;
        auto &bindings = initial ? initialActions : actions;
        bindings[0] = std::make_unique<InputAction>(std::vector<InputActionItem>{});

        MockTrigger trigger;
        trigger.addAction(std::make_unique<TriggerAction>(std::make_unique<DirectionalAction>(75, std::move(actions), std::move(initialActions))));

        QCOMPARE(trigger.m_clearModifiers, std::optional<bool>(true));
    }

    void addAction_directionalWithoutInput__doesNotClearModifiers()
    {
        DirectionalAction::Bindings actions;
        actions[0] = std::make_unique<CustomAction>([](const auto &) {});

        MockTrigger trigger;
        trigger.addAction(std::make_unique<TriggerAction>(std::make_unique<DirectionalAction>(75, std::move(actions))));

        QVERIFY(!trigger.m_clearModifiers);
    }
};

}

QTEST_MAIN(InputActions::TestTrigger)
#include "TestTrigger.moc"