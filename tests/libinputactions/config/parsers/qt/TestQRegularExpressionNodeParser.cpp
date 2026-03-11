#include "Test.h"
#include <QRegularExpression>
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/Node.h>

namespace InputActions
{

class TestQRegularExpressionNodeParser : public Test
{
    Q_OBJECT

private slots:
    void valid__parsesNodeCorrectly()
    {
        const auto node = Node::create("\"[a]\"");
        const auto regex = node->as<QRegularExpression>();

        QVERIFY(regex.isValid());
        QCOMPARE(regex.pattern(), "[a]");
    }

    void invalid__throwsInvalidValueConfigException()
    {
        const auto node = Node::create("\"[a\"");
        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<QRegularExpression>(), InvalidValueConfigException, 0, 0);
    }
};

}

QTEST_MAIN(InputActions::TestQRegularExpressionNodeParser)
#include "TestQRegularExpressionNodeParser.moc"