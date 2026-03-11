#include "Test.h"
#include <libinputactions/config/Node.h>

namespace InputActions
{

class TestQStringListNodeParser : public Test
{
    Q_OBJECT

private slots:
    void valid__parsesNodeCorrectly()
    {
        const auto node = Node::create("[ aaa, 123 ]");
        const auto list = node->as<QStringList>();

        QCOMPARE(list.size(), 2);
        QCOMPARE(list.at(0), "aaa");
        QCOMPARE(list.at(1), "123");
    }
};

}

QTEST_MAIN(InputActions::TestQStringListNodeParser)
#include "TestQStringListNodeParser.moc"