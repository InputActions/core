#pragma once

#include <gmock/gmock.h>
#include <libinputactions/triggers/core/SwipeTriggerCore.h>

namespace InputActions
{

class MockSwipeTriggerCore : public SwipeTriggerCore
{
public:
    MockSwipeTriggerCore(qreal angleMin, qreal angleMax)
        : SwipeTriggerCore(angleMin, angleMax)
    {
        ON_CALL(*this, doUpdateActions).WillByDefault([this](const auto &actions, const auto &event) {
            this->SwipeTriggerCore::doUpdateActions(actions, event);
        });
    }

    MOCK_METHOD(void, doUpdateActions, (const std::vector<std::unique_ptr<TriggerAction>> &actions, const TriggerUpdateEvent &event), (const, override));
};

}