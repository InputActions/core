#include "Test.h"
#include <libinputactions/actions/ActionExecutor.h>
#include <libinputactions/actions/ActionGroup.h>
#include <libinputactions/actions/CustomAction.h>

namespace InputActions
{

class TestActionGroup : public Test
{
    Q_OBJECT

private slots:
    void all_execute__propagatesArgumentsToSubActions()
    {
        auto *assertAction = new CustomAction([](const auto &args) {
            QCOMPARE(args.inputActionArgs.motionPointDelta, QPointF(12, 34));
        });

        auto group = std::make_shared<ActionGroup>(ActionGroupExecutionMode::All);
        group->append(std::unique_ptr<Action>(assertAction));

        g_actionExecutor->execute(*group, {
            .actionArgs = {
                .inputActionArgs = {
                    .motionPointDelta = QPointF(12, 34),
                },
            }});

        QCOMPARE(assertAction->executions(), 1);
    }

    void first_execute__propagatesArgumentsToSubActions()
    {
        auto *assertAction = new CustomAction([](const auto &args) {
            QCOMPARE(args.inputActionArgs.motionPointDelta, QPointF(12, 34));
        });

        auto group = std::make_shared<ActionGroup>(ActionGroupExecutionMode::First);
        group->append(std::unique_ptr<Action>(assertAction));

        g_actionExecutor->execute(*group, {
            .actionArgs = {
                .inputActionArgs = {
                    .motionPointDelta = QPointF(12, 34),
                },
            }});

        QCOMPARE(assertAction->executions(), 1);
    }
};

}

QTEST_MAIN(InputActions::TestActionGroup)
#include "TestActionGroup.moc"