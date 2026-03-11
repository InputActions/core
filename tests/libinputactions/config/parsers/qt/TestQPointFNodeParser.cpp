#include "Test.h"
#include <libinputactions/config/Node.h>

namespace InputActions
{

class TestQPointFNodeParser : public Test
{
    Q_OBJECT

private slots:
    void valid__parsesNodeCorrectly()
    {
        const auto node = Node::create("12.34,43.21");
        const auto point = node->as<QPointF>();

        QCOMPARE(point, QPointF(12.34, 43.21));
    }

    void invalid__throwsInvalidValueConfigException()
    {
        const auto node = Node::create("12.34");
        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<QPointF>(), InvalidValueConfigException, 0, 0);
    }
};

}

QTEST_MAIN(InputActions::TestQPointFNodeParser)
#include "TestQPointFNodeParser.moc"