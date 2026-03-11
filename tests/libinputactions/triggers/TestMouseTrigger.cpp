#include "Test.h"
#include "mocks/MockMouseTrigger.h"

namespace InputActions
{

class TestMouseTrigger : public Test
{
    Q_OBJECT

private slots:
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

        MockMouseTrigger trigger;
        trigger.setMouseButtons(triggerButtons.value());
        trigger.setMouseButtonsExactOrder(orderMatters);

        TriggerActivationEvent event;
        event.setMouseButtons(eventButtons);
        QCOMPARE(trigger.canActivate(event), result);
    }
};

}

QTEST_MAIN(InputActions::TestMouseTrigger)
#include "TestMouseTrigger.moc"