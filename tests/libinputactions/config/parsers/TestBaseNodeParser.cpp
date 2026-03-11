#include "Test.h"
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/ConfigIssueManager.h>
#include <libinputactions/config/Node.h>

namespace InputActions
{

class TestBaseNodeParser : public Test
{
    Q_OBJECT

private slots:
    void boolean_valid__parsesNodeCorrectly_data()
    {
        QTest::addColumn<QString>("raw");
        QTest::addColumn<bool>("result");

        // Those are the only values officially defined as valid
        QTest::addRow("true") << "true" << true;
        QTest::addRow("false") << "false" << false;
    }

    void boolean_valid__parsesNodeCorrectly()
    {
        QFETCH(QString, raw);
        QFETCH(bool, result);

        const auto node = Node::create(raw);
        QCOMPARE(node->as<bool>(), result);
    }

    void boolean_invalid__throwsInvalidValueConfigException_data()
    {
        QTest::addColumn<QString>("raw");

        QTest::addRow("empty") << "";
        QTest::addRow("space") << "\" \"";
        QTest::addRow("char") << "a";
        QTest::addRow("number") << "1";
    }

    void boolean_invalid__throwsInvalidValueConfigException()
    {
        QFETCH(QString, raw);

        const auto node = Node::create(raw);
        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<bool>(), InvalidValueConfigException, 0, 0);
    }

    void int8_valid__parsesNodeCorrectly_data()
    {
        QTest::addColumn<QString>("raw");
        QTest::addColumn<int8_t>("result");

        QTest::addRow("min") << "-128" << static_cast<int8_t>(-128);
        QTest::addRow("-1") << "-1" << static_cast<int8_t>(-1);
        QTest::addRow("0") << "0" << static_cast<int8_t>(0);
        QTest::addRow("1") << "1" << static_cast<int8_t>(1);
        QTest::addRow("max") << "127" << static_cast<int8_t>(127);
        QTest::addRow("quoted") << "\"1\"" << static_cast<int8_t>(1);
    }

    void int8_valid__parsesNodeCorrectly()
    {
        QFETCH(QString, raw);
        QFETCH(int8_t, result);

        const auto node = Node::create(raw);
        QCOMPARE(node->as<int8_t>(), result);
    }

    void int8_invalid__throwsInvalidValueConfigException_data()
    {
        QTest::addColumn<QString>("raw");

        QTest::addRow("empty") << "";
        QTest::addRow("space") << "\" \"";
        QTest::addRow("char") << "a";
        QTest::addRow("float") << "1.0";
        QTest::addRow("min-1") << "-129";
        QTest::addRow("max+1") << "128";
    }

    void int8_invalid__throwsInvalidValueConfigException()
    {
        QFETCH(QString, raw);

        const auto node = Node::create(raw);
        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<int8_t>(), InvalidValueConfigException, 0, 0);
    }

    void uint8_valid__parsesNodeCorrectly_data()
    {
        QTest::addColumn<QString>("raw");
        QTest::addColumn<uint8_t>("result");

        QTest::addRow("min") << "0" << static_cast<uint8_t>(0);
        QTest::addRow("1") << "1" << static_cast<uint8_t>(1);
        QTest::addRow("max") << "255" << static_cast<uint8_t>(255);
        QTest::addRow("quoted") << "\"1\"" << static_cast<uint8_t>(1);
    }

    void uint8_valid__parsesNodeCorrectly()
    {
        QFETCH(QString, raw);
        QFETCH(uint8_t, result);

        const auto node = Node::create(raw);
        QCOMPARE(node->as<uint8_t>(), result);
    }

    void uint8_invalid__throwsInvalidValueConfigException_data()
    {
        QTest::addColumn<QString>("raw");

        QTest::addRow("empty") << "";
        QTest::addRow("space") << "\" \"";
        QTest::addRow("char") << "a";
        QTest::addRow("float") << "1.0";
        QTest::addRow("min-1") << "-1";
        QTest::addRow("max+1") << "256";
    }

    void uint8_invalid__throwsInvalidValueConfigException()
    {
        QFETCH(QString, raw);

        const auto node = Node::create(raw);
        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<uint8_t>(), InvalidValueConfigException, 0, 0);
    }

    void uint32_valid__parsesNodeCorrectly_data()
    {
        QTest::addColumn<QString>("raw");
        QTest::addColumn<uint32_t>("result");

        QTest::addRow("min") << "0" << static_cast<uint32_t>(0);
        QTest::addRow("1") << "1" << static_cast<uint32_t>(1);
        QTest::addRow("max") << "4294967295" << static_cast<uint32_t>(4294967295);
        QTest::addRow("quoted") << "\"1\"" << static_cast<uint32_t>(1);
    }

    void uint32_valid__parsesNodeCorrectly()
    {
        QFETCH(QString, raw);
        QFETCH(uint32_t, result);

        const auto node = Node::create(raw);
        QCOMPARE(node->as<uint32_t>(), result);
    }

    void uint32_invalid__throwsInvalidValueConfigException_data()
    {
        QTest::addColumn<QString>("raw");

        QTest::addRow("empty") << "";
        QTest::addRow("space") << "\" \"";
        QTest::addRow("char") << "a";
        QTest::addRow("float") << "1.0";
        QTest::addRow("min-1") << "-1";
        QTest::addRow("max+1") << "4294967296";
    }

    void uint32_invalid__throwsInvalidValueConfigException()
    {
        QFETCH(QString, raw);

        const auto node = Node::create(raw);
        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<uint32_t>(), InvalidValueConfigException, 0, 0);
    }

    void uint64_valid__parsesNodeCorrectly_data()
    {
        QTest::addColumn<QString>("raw");
        QTest::addColumn<uint64_t>("result");

        QTest::addRow("min") << "0" << static_cast<uint64_t>(0);
        QTest::addRow("1") << "1" << static_cast<uint64_t>(1);
        QTest::addRow("max") << "18446744073709551615" << static_cast<uint64_t>(18446744073709551615U);
        QTest::addRow("quoted") << "\"1\"" << static_cast<uint64_t>(1);
    }

    void uint64_valid__parsesNodeCorrectly()
    {
        QFETCH(QString, raw);
        QFETCH(uint64_t, result);

        const auto node = Node::create(raw);
        QCOMPARE(node->as<uint64_t>(), result);
    }

    void uint64_invalid__throwsInvalidValueConfigException_data()
    {
        QTest::addColumn<QString>("raw");

        QTest::addRow("empty") << "";
        QTest::addRow("space") << "\" \"";
        QTest::addRow("char") << "a";
        QTest::addRow("float") << "1.0";
        QTest::addRow("min-1") << "-1";
        QTest::addRow("max+1") << "18446744073709551616";
    }

    void uint64_invalid__throwsInvalidValueConfigException()
    {
        QFETCH(QString, raw);

        const auto node = Node::create(raw);
        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<uint64_t>(), InvalidValueConfigException, 0, 0);
    }

    void qreal_valid__parsesNodeCorrectly_data()
    {
        QTest::addColumn<QString>("raw");
        QTest::addColumn<qreal>("result");

        QTest::addRow("-1") << "-1" << static_cast<qreal>(-1);
        QTest::addRow("0") << "0" << static_cast<qreal>(0);
        QTest::addRow("1") << "1" << static_cast<qreal>(1);

        QTest::addRow("trailingDot") << "2." << static_cast<qreal>(2);
        QTest::addRow("leadingDot") << ".2" << static_cast<qreal>(0.2);

        QTest::addRow("-123.456") << "-123.456" << static_cast<qreal>(-123.456);
        QTest::addRow("123.456") << "123.456" << static_cast<qreal>(123.456);

        QTest::addRow("quoted") << "\"1.1\"" << static_cast<qreal>(1.1);
    }

    void qreal_valid__parsesNodeCorrectly()
    {
        QFETCH(QString, raw);
        QFETCH(qreal, result);

        const auto node = Node::create(raw);
        QCOMPARE(node->as<qreal>(), result);
    }

    void qreal_invalid__throwsInvalidValueConfigException_data()
    {
        QTest::addColumn<QString>("raw");

        QTest::addRow("empty") << "";
        QTest::addRow("space") << "\" \"";
        QTest::addRow("char") << "a";
    }

    void qreal_invalid__throwsInvalidValueConfigException()
    {
        QFETCH(QString, raw);

        const auto node = Node::create(raw);
        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<qreal>(), InvalidValueConfigException, 0, 0);
    }

    void string_valid__parsesNodeCorrectly_data()
    {
        QTest::addColumn<QString>("raw");

        QTest::addRow("bool") << "true";
        QTest::addRow("number") << "2";
        QTest::addRow("float") << "2";
    }

    void string_valid__parsesNodeCorrectly()
    {
        QFETCH(QString, raw);

        const auto node = Node::create(raw);
        QCOMPARE(node->as<QString>(), raw);
    }

    void map__throwsInvalidNodeTypeConfigException()
    {
        const auto node = Node::create("_: _");

        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION_SAVE(node->as<bool>(), InvalidNodeTypeConfigException, 0, 0, e);
        QCOMPARE(e->expected(), NodeType::Scalar);
        QCOMPARE(e->actual(), NodeType::Map);
    }

    void null__throwsInvalidNodeTypeConfigException()
    {
        const auto node = Node::create("_:")->at("_")->shared_from_this();
        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<bool>(), InvalidValueConfigException, 0, 0);
    }

    void sequence__throwsInvalidNodeTypeConfigException()
    {
        const auto node = Node::create("[]");

        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION_SAVE(node->as<bool>(), InvalidNodeTypeConfigException, 0, 0, e);
        QCOMPARE(e->expected(), NodeType::Scalar);
        QCOMPARE(e->actual(), NodeType::Sequence);
    }
};

}

QTEST_MAIN(InputActions::TestBaseNodeParser)
#include "TestBaseNodeParser.moc"