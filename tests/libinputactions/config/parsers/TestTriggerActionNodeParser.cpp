#include "Test.h"
#include <libinputactions/actions/TriggerAction.h>
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/Node.h>

namespace InputActions
{

class TestTriggerActionNodeParser : public Test
{
    Q_OBJECT

private slots:
    void interval_validOn__doesNotThrow_data()
    {
        QTest::addColumn<QString>("on");

        QTest::addRow("tick") << "tick";
        QTest::addRow("update") << "update";
    }

    void interval_validOn__doesNotThrow()
    {
        QFETCH(QString, on);

        const auto node = Node::create(QString(R"(
            on: %1
            interval: 1
            command: _
        )")
                                           .arg(on));

        QVERIFY_THROWS_NO_EXCEPTION(node->as<std::unique_ptr<TriggerAction>>());
    }

    void interval_invalidOn__throwsInvalidValueConfigException_data()
    {
        QTest::addColumn<QString>("on");

        QTest::addRow("begin") << "begin";
        QTest::addRow("cancel") << "cancel";
        QTest::addRow("end") << "end";
        QTest::addRow("end_cancel") << "end_cancel";
    }

    void interval_invalidOn__throwsInvalidValueConfigException()
    {
        QFETCH(QString, on);

        const auto node = Node::create(QString(R"(
            on: %1
            interval: 1
            command: _
        )")
                                           .arg(on));

        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::unique_ptr<TriggerAction>>(), InvalidValueContextConfigException, 2, 22);
    }

    void threshold_validOn__doesNotThrow_data()
    {
        QTest::addColumn<QString>("on");

        QTest::addRow("cancel") << "cancel";
        QTest::addRow("end") << "end";
        QTest::addRow("end_cancel") << "end_cancel";
        QTest::addRow("tick") << "tick";
        QTest::addRow("update") << "update";
    }

    void threshold_validOn__doesNotThrow()
    {
        QFETCH(QString, on);

        const auto node = Node::create(QString(R"(
            on: %1
            threshold: 1
            command: _
        )")
                                           .arg(on));

        QVERIFY_THROWS_NO_EXCEPTION(node->as<std::unique_ptr<TriggerAction>>());
    }

    void threshold_invalidOn__throwsInvalidValueConfigException_data()
    {
        QTest::addColumn<QString>("on");

        QTest::addRow("begin") << "begin";
    }

    void threshold_invalidOn__throwsInvalidValueConfigException()
    {
        QFETCH(QString, on);

        const auto node = Node::create(QString(R"(
            on: %1
            threshold: 1
            command: _
        )")
                                           .arg(on));

        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::unique_ptr<TriggerAction>>(), InvalidValueContextConfigException, 2, 23);
    }
};

}

QTEST_MAIN(InputActions::TestTriggerActionNodeParser)
#include "TestTriggerActionNodeParser.moc"