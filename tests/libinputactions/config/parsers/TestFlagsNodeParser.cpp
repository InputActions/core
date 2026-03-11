#include "Test.h"
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/config/parsers/flags.h>

namespace InputActions
{

class TestFlagsNodeParser : public Test
{
    Q_OBJECT

private slots:
    void empty__parsesNodeCorrectly()
    {
        const auto node = Node::create("[]");
        QCOMPARE(node->as<Qt::KeyboardModifiers>(), Qt::KeyboardModifier::NoModifier);
    }

    void oneItem__parsesNodeCorrectly()
    {
        const auto node = Node::create("[ ctrl ]");
        QCOMPARE(node->as<Qt::KeyboardModifiers>(), Qt::KeyboardModifier::ControlModifier);
    }

    void twoItems__parsesNodeCorrectly()
    {
        const auto node = Node::create("[ ctrl, meta ]");
        QCOMPARE(node->as<Qt::KeyboardModifiers>(), Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::MetaModifier);
    }

    void invalid_duplicateItem__throwsInvalidNodeTypeConfigException()
    {
        const auto node = Node::create("[ meta, meta ]");

        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION_SAVE(node->as<Qt::KeyboardModifiers>(), DuplicateSetItemConfigException, 0, 8, e);
        QCOMPARE(e->index(), 1);
    }

    void invalid_scalar__throwsInvalidNodeTypeConfigException()
    {
        const auto node = Node::create("meta");

        INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION_SAVE(node->as<Qt::KeyboardModifiers>(), InvalidNodeTypeConfigException, 0, 0, e);
        QCOMPARE(e->expected(), NodeType::Sequence);
        QCOMPARE(e->actual(), NodeType::Scalar);
    }
};

}

QTEST_MAIN(InputActions::TestFlagsNodeParser)
#include "TestFlagsNodeParser.moc"