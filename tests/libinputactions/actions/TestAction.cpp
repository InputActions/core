#include "Test.h"
#include <libinputactions/actions/Action.h>
#include <libinputactions/actions/ActionExecutor.h>

namespace InputActions
{

class TestAction : public Test
{
    Q_OBJECT

private slots:
    void canExecute_noLimit()
    {
        Action action;

        QVERIFY(action.canExecute());
        g_actionExecutor->execute(action);
        QVERIFY(action.canExecute());
    }

    void canExecute_withLimit()
    {
        Action action;
        action.setExecutionLimit(1);

        QVERIFY(action.canExecute());
        g_actionExecutor->execute(action);
        QVERIFY(!action.canExecute());

        action.reset();
        QVERIFY(action.canExecute());
        g_actionExecutor->execute(action);
        QVERIFY(!action.canExecute());
    }
};

}

QTEST_MAIN(InputActions::TestAction)
#include "TestAction.moc"