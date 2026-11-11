#include "Test.h"
#include "utils.h"
#include <QSemaphore>
#include <QThread>
#include <QThreadPool>
#include <libinputactions/actions/ActionExecutor.h>
#include <libinputactions/actions/CustomAction.h>
#include <libinputactions/actions/DirectionalAction.h>
#include <libinputactions/helpers/QThread.h>

namespace InputActions
{

/**
 * Keeps the shared action thread busy until destroyed.
 */
class BusyActionThread
{
public:
    BusyActionThread()
        : m_action(
              [this](const auto &) {
                  m_started.release();
                  m_release.acquire();
              },
              true)
    {
        g_actionExecutor->execute(m_action);
        m_started.acquire();
    }

    ~BusyActionThread()
    {
        release();
        g_actionExecutor->waitForDone();
    }

    void release()
    {
        if (!m_released) {
            m_released = true;
            m_release.release();
        }
    }

private:
    QSemaphore m_started;
    QSemaphore m_release;
    bool m_released{};
    CustomAction m_action;
};

class TestDirectionalAction : public Test
{
    Q_OBJECT

private slots:
    void init() { m_log.clear(); }

    void execute_distanceReachedOverMultipleUpdates__executesAction()
    {
        DirectionalAction action(75, arrows());

        move(action, 74, 0);
        QCOMPARE(m_log, QStringList());

        move(action, 1, 0);
        QCOMPARE(m_log, QStringList({"right"}));
    }

    void execute_eachDirection__executesActionForDirection_data()
    {
        QTest::addColumn<qreal>("x");
        QTest::addColumn<qreal>("y");
        QTest::addColumn<QString>("name");

        QTest::newRow("left") << -75.0 << 0.0 << "left";
        QTest::newRow("right") << 75.0 << 0.0 << "right";
        QTest::newRow("up") << 0.0 << -75.0 << "up";
        QTest::newRow("down") << 0.0 << 75.0 << "down";
    }

    void execute_eachDirection__executesActionForDirection()
    {
        QFETCH(qreal, x);
        QFETCH(qreal, y);
        QFETCH(QString, name);
        DirectionalAction action(75, arrows());

        move(action, x, y);

        QCOMPARE(m_log, QStringList({name}));
    }

    void execute_directionReversed__restartsAccumulation()
    {
        DirectionalAction action(75, arrows());

        move(action, 74, 0);
        move(action, -75, 0);

        QCOMPARE(m_log, QStringList({"left"}));
    }

    void execute_largeMotion__executesAtMostEightActions()
    {
        DirectionalAction action(75, arrows());

        move(action, 75 * 20, 0);

        QCOMPARE(m_log.size(), 8);
    }

    void execute_axisChanged__resetsOtherAxis()
    {
        DirectionalAction action(75, arrows());

        move(action, 74, 0);
        move(action, 0, 75);
        move(action, 1, 0);
        QCOMPARE(m_log, QStringList({"down"}));

        move(action, 0, 74);
        move(action, 75, 0);
        move(action, 0, 1);
        QCOMPARE(m_log, QStringList({"down", "right"}));
    }

    void execute_actionConditionNotSatisfied__skipsActionAndConsumesMotion()
    {
        bool satisfied = false;
        auto actions = arrows();
        actions[1]->setCondition(referenceCondition(satisfied));
        DirectionalAction action(75, std::move(actions));

        move(action, 75, 0);
        QCOMPARE(m_log, QStringList());

        satisfied = true;
        move(action, 74, 0);
        QCOMPARE(m_log, QStringList());

        move(action, 1, 0);
        QCOMPARE(m_log, QStringList({"right"}));
    }

    void execute_fromOtherThreadWhileMainThreadWaits__executesOnMainThreadWithoutDeadlock()
    {
        // The main thread waits for action threads while reloading the configuration.
        QThread *thread{};
        DirectionalAction::Bindings actions;
        actions[1] = std::make_unique<CustomAction>([&thread](const auto &) {
            thread = QThread::currentThread();
        });
        DirectionalAction action(75, std::move(actions));
        QThreadPool pool;

        pool.start([&action] {
            move(action, 75, 0);
        });
        const auto finished = pool.waitForDone(1000);
        while (!pool.waitForDone(10)) {
            QCoreApplication::processEvents();
        }
        QVERIFY(finished);

        QTRY_VERIFY(thread);
        QCOMPARE(thread, QThreadHelpers::mainThread());
    }

    void execute_fromOtherThread_destroyedBeforeMainThread__ignoresMotion()
    {
        auto action = std::make_unique<DirectionalAction>(75, arrows());
        auto *raw = action.get();
        QThreadPool pool;

        pool.start([raw] {
            move(*raw, 75, 0);
        });
        pool.waitForDone();
        action.reset();
        QCoreApplication::sendPostedEvents();

        QCOMPARE(m_log, QStringList());
    }

    void execute_actionThreadBusy__executesSynchronousActionImmediately()
    {
        DirectionalAction action(75, arrows());
        QStringList log;
        {
            BusyActionThread busy;
            move(action, 75, 0);
            log = m_log;
        }

        QCOMPARE(log, QStringList({"right"}));
    }

    void execute_asynchronousAction__executesOnActionThread()
    {
        QThread *thread{};
        DirectionalAction::Bindings actions;
        actions[1] = std::make_unique<CustomAction>(
            [&thread](const auto &) {
                thread = QThread::currentThread();
            },
            true);
        DirectionalAction action(75, std::move(actions));

        move(action, 75, 0);
        g_actionExecutor->waitForDone();

        QVERIFY(thread);
        QVERIFY(thread != QThreadHelpers::mainThread());
    }

    void execute_initial__executesInitialImmediatelyAndDoesNotCountMotion()
    {
        DirectionalAction action(75, arrows(), tabs());

        move(action, 1, 0);
        QCOMPARE(m_log, QStringList({"tab"}));

        move(action, 74, 0);
        QCOMPARE(m_log, QStringList({"tab"}));

        move(action, 1, 0);
        QCOMPARE(m_log, QStringList({"tab", "right"}));
    }

    void execute_initial_motionAngle__usesDominantAxisOfAngle_data()
    {
        QTest::addColumn<qreal>("angle");
        QTest::addColumn<QString>("name");

        QTest::newRow("right") << 0.0 << "initial right";
        QTest::newRow("up") << 90.0 << "initial up";
        QTest::newRow("left") << 180.0 << "initial left";
        QTest::newRow("down") << 270.0 << "initial down";
        QTest::newRow("mostly up") << 60.0 << "initial up";
        QTest::newRow("mostly right") << 330.0 << "initial right";
    }

    void execute_initial_motionAngle__usesDominantAxisOfAngle()
    {
        QFETCH(qreal, angle);
        QFETCH(QString, name);
        DirectionalAction action(75, arrows(), bindings({"initial left", "initial right", "initial up", "initial down"}));

        move(action, 1, 0, angle);

        QCOMPARE(m_log, QStringList({name}));
    }

    void execute_initial_repeatConditionSatisfied__skipsInitial()
    {
        bool satisfied = true;
        DirectionalAction action(75, arrows(), tabs());
        action.setRepeatCondition(referenceCondition(satisfied));

        move(action, 75, 0);

        QCOMPARE(m_log, QStringList({"right"}));
    }

    void execute_initial_repeatConditionNotSatisfied__executesOnlyInitial()
    {
        bool satisfied = false;
        DirectionalAction action(75, arrows(), tabs());
        action.setRepeatCondition(referenceCondition(satisfied));

        move(action, 1, 0);
        move(action, 150, 0);

        QCOMPARE(m_log, QStringList({"tab"}));
    }

    void execute_actionUnsatisfiesRepeatCondition__discardsRemainingMotion()
    {
        bool satisfied = true;
        DirectionalAction::Bindings actions;
        actions[1] = std::make_unique<CustomAction>([this, &satisfied](const auto &) {
            m_log.push_back("right");
            satisfied = false;
        });
        DirectionalAction action(75, std::move(actions));
        action.setRepeatCondition(referenceCondition(satisfied));

        move(action, 150, 0);
        QCOMPARE(m_log, QStringList({"right"}));

        satisfied = true;
        move(action, 74, 0);
        QCOMPARE(m_log, QStringList({"right"}));
    }

    void execute_repeatConditionNotSatisfied__discardsMotion()
    {
        bool satisfied = false;
        DirectionalAction action(75, arrows());
        action.setRepeatCondition(referenceCondition(satisfied));

        move(action, 74, 0);
        satisfied = true;
        move(action, 1, 0);
        QCOMPARE(m_log, QStringList());

        move(action, 74, 0);
        QCOMPARE(m_log, QStringList({"right"}));
    }

    void execute_noMotion__isIgnored()
    {
        DirectionalAction action(75, arrows(), tabs());

        move(action, 0, 0);
        move(action, 0, 0, 180);
        QCOMPARE(m_log, QStringList());

        move(action, -1, 0);
        QCOMPARE(m_log, QStringList({"shift+tab"}));
    }

    void reset_actionLimitReached__executesActionAgain()
    {
        auto actions = arrows();
        actions[1]->setExecutionLimit(1);
        DirectionalAction action(75, std::move(actions));

        move(action, 150, 0);
        QCOMPARE(m_log, QStringList({"right"}));

        action.reset();
        move(action, 75, 0);
        QCOMPARE(m_log, QStringList({"right", "right"}));
    }

    void reset__executesInitialAgain()
    {
        DirectionalAction action(75, arrows(), tabs());

        move(action, 1, 0);
        action.reset();
        move(action, 1, 0);

        QCOMPARE(m_log, QStringList({"tab", "tab"}));
    }

    void reset_partialMotion__discardsAccumulatedMotion()
    {
        DirectionalAction action(75, arrows());

        move(action, 74, 0);
        action.reset();
        move(action, 1, 0);

        QCOMPARE(m_log, QStringList());
    }

private:
    std::unique_ptr<Action> record(const QString &name)
    {
        return std::make_unique<CustomAction>([this, name](const auto &) {
            m_log.push_back(name);
        });
    }

    /**
     * @param names Ordered left, right, up, down. An empty name leaves the direction without an action.
     */
    DirectionalAction::Bindings bindings(const std::array<QString, 4> &names)
    {
        DirectionalAction::Bindings result;
        for (size_t i = 0; i < names.size(); ++i) {
            if (!names[i].isEmpty()) {
                result[i] = record(names[i]);
            }
        }
        return result;
    }

    DirectionalAction::Bindings arrows() { return bindings({"left", "right", "up", "down"}); }
    DirectionalAction::Bindings tabs() { return bindings({"shift+tab", "tab", "", ""}); }

    static ActionExecutionArguments motion(qreal x, qreal y, std::optional<qreal> angle = {})
    {
        ActionExecutionArguments args;
        args.inputActionArgs.motionPointDelta = PointF(x, y);
        args.inputActionArgs.motionAngle = angle;
        return args;
    }

    static void move(DirectionalAction &action, qreal x, qreal y, std::optional<qreal> angle = {}) { action.execute(motion(x, y, angle)); }

    QStringList m_log;
};

}

QTEST_MAIN(InputActions::TestDirectionalAction)
#include "TestDirectionalAction.moc"