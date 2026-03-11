#include "Test.h"
#include <libinputactions/config/Node.h>

namespace InputActions
{

class TestQStringNodeParser : public Test
{
    Q_OBJECT

private slots:
    void valid__parsesNodeCorrectly()
    {
        const auto node = Node::create("aaa123");
        QCOMPARE(node->as<QString>(), "aaa123");
    }
};

}

QTEST_MAIN(InputActions::TestQStringNodeParser)
#include "TestQStringNodeParser.moc"