#include "Test.h"
#include <libinputactions/PointF.h>
#include <libinputactions/config/Node.h>

namespace InputActions
{

class TestPointFNodeParser : public Test
{
    Q_OBJECT

private slots:
    void valid__parsesNodeCorrectly()
    {
        const auto node = Node::create("12.34,43.21");
        const auto point = node->as<PointF>();

        QCOMPARE(point, PointF(12.34, 43.21));
    }

    void invalid__throwsInvalidValueConfigException()
    {
        const auto node = Node::create("12.34");
        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<PointF>(), InvalidValueConfigException, 0, 0);
    }
};

}

QTEST_MAIN(InputActions::TestPointFNodeParser)
#include "TestPointFNodeParser.moc"