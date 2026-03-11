#include "Test.h"
#include <libinputactions/triggers/core/DirectionalMotionTriggerCore.h>

namespace InputActions
{

class TestDirectionalMotionTriggerCore : public Test
{
    Q_OBJECT

private slots:
    void canUpdate_data()
    {
        QTest::addColumn<uint32_t>("direction");
        QTest::addColumn<std::vector<uint32_t>>("valid");
        QTest::addColumn<std::vector<uint32_t>>("invalid");

        QTest::addRow("pinch any") << static_cast<TriggerDirection>(PinchDirection::Any)
                                   << std::vector<uint32_t>({static_cast<TriggerDirection>(PinchDirection::In),
                                                             static_cast<TriggerDirection>(PinchDirection::Out)})
                                   << std::vector<uint32_t>();
        QTest::addRow("pinch in") << static_cast<TriggerDirection>(PinchDirection::In)
                                  << std::vector<uint32_t>({static_cast<TriggerDirection>(PinchDirection::In)})
                                  << std::vector<uint32_t>({static_cast<TriggerDirection>(PinchDirection::Out)});
        QTest::addRow("pinch out") << static_cast<TriggerDirection>(PinchDirection::Out)
                                   << std::vector<uint32_t>({static_cast<TriggerDirection>(PinchDirection::Out)})
                                   << std::vector<uint32_t>({static_cast<TriggerDirection>(PinchDirection::In)});
        // Rotate is same as pinch
        QTest::addRow("swipe left") << static_cast<TriggerDirection>(SimpleSwipeDirection::Left)
                                    << std::vector<uint32_t>({static_cast<TriggerDirection>(SimpleSwipeDirection::Left)})
                                    << std::vector<uint32_t>({static_cast<TriggerDirection>(SimpleSwipeDirection::Right),
                                                              static_cast<TriggerDirection>(SimpleSwipeDirection::Up),
                                                              static_cast<TriggerDirection>(SimpleSwipeDirection::Down)});
        QTest::addRow("swipe right") << static_cast<TriggerDirection>(SimpleSwipeDirection::Right)
                                     << std::vector<uint32_t>({static_cast<TriggerDirection>(SimpleSwipeDirection::Right)})
                                     << std::vector<uint32_t>({static_cast<TriggerDirection>(SimpleSwipeDirection::Left),
                                                               static_cast<TriggerDirection>(SimpleSwipeDirection::Up),
                                                               static_cast<TriggerDirection>(SimpleSwipeDirection::Down)});
        QTest::addRow("swipe left/right")
            << static_cast<TriggerDirection>(SimpleSwipeDirection::LeftRight)
            << std::vector<uint32_t>({static_cast<TriggerDirection>(SimpleSwipeDirection::Left), static_cast<TriggerDirection>(SimpleSwipeDirection::Right)})
            << std::vector<uint32_t>({static_cast<TriggerDirection>(SimpleSwipeDirection::Up), static_cast<TriggerDirection>(SimpleSwipeDirection::Down)});
        QTest::addRow("swipe up") << static_cast<TriggerDirection>(SimpleSwipeDirection::Up)
                                  << std::vector<uint32_t>({static_cast<TriggerDirection>(SimpleSwipeDirection::Up)})
                                  << std::vector<uint32_t>({static_cast<TriggerDirection>(SimpleSwipeDirection::Right),
                                                            static_cast<TriggerDirection>(SimpleSwipeDirection::Left),
                                                            static_cast<TriggerDirection>(SimpleSwipeDirection::Down)});
        QTest::addRow("swipe down") << static_cast<TriggerDirection>(SimpleSwipeDirection::Down)
                                    << std::vector<uint32_t>({static_cast<TriggerDirection>(SimpleSwipeDirection::Down)})
                                    << std::vector<uint32_t>({static_cast<TriggerDirection>(SimpleSwipeDirection::Right),
                                                              static_cast<TriggerDirection>(SimpleSwipeDirection::Up),
                                                              static_cast<TriggerDirection>(SimpleSwipeDirection::Left)});
        QTest::addRow("swipe up/down")
            << static_cast<TriggerDirection>(SimpleSwipeDirection::UpDown)
            << std::vector<uint32_t>({static_cast<TriggerDirection>(SimpleSwipeDirection::Up), static_cast<TriggerDirection>(SimpleSwipeDirection::Down)})
            << std::vector<uint32_t>({static_cast<TriggerDirection>(SimpleSwipeDirection::Left), static_cast<TriggerDirection>(SimpleSwipeDirection::Right)});
        QTest::addRow("swipe any") << static_cast<TriggerDirection>(SimpleSwipeDirection::Any)
                                   << std::vector<uint32_t>({static_cast<TriggerDirection>(SimpleSwipeDirection::Up),
                                                             static_cast<TriggerDirection>(SimpleSwipeDirection::Down),
                                                             static_cast<TriggerDirection>(SimpleSwipeDirection::Left),
                                                             static_cast<TriggerDirection>(SimpleSwipeDirection::Right)})
                                   << std::vector<uint32_t>({});
    }

    void canUpdate()
    {
        QFETCH(uint32_t, direction);
        QFETCH(std::vector<uint32_t>, valid);
        QFETCH(std::vector<uint32_t>, invalid);

        DirectionalMotionTriggerCore core(direction);
        for (const auto &validDirection : valid) {
            DirectionalMotionTriggerUpdateEvent event;
            event.setDirection(validDirection);
            QVERIFY(core.canUpdate(event));
        }
        for (const auto &invalidDirection : invalid) {
            DirectionalMotionTriggerUpdateEvent event;
            event.setDirection(invalidDirection);
            QVERIFY(!core.canUpdate(event));
        }
    }
};

}

QTEST_MAIN(InputActions::TestDirectionalMotionTriggerCore)
#include "TestDirectionalMotionTriggerCore.moc"