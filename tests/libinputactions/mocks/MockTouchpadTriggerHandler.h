#pragma once

#include <gmock/gmock.h>
#include <libinputactions/handlers/TouchpadTriggerHandler.h>

namespace InputActions
{

class MockTouchpadTriggerHandler : public TouchpadTriggerHandler
{
public:
    MockTouchpadTriggerHandler(InputDevice *device)
        : TouchpadTriggerHandler(device)
    {
        ON_CALL(*this, activatingTriggers).WillByDefault([this](const auto types) {
            return this->InputTriggerHandler::activatingTriggers(types);
        });
        ON_CALL(*this, handleMotion).WillByDefault([this](const auto *device, const auto &delta) {
            return this->TouchpadTriggerHandler::handleMotion(device, delta);
        });
        ON_CALL(*this, doCancelTriggers).WillByDefault([this](const auto types) {
            return this->TouchpadTriggerHandler::doCancelTriggers(types);
        });
        ON_CALL(*this, doEndTriggers).WillByDefault([this](const auto types) {
            return this->TouchpadTriggerHandler::doEndTriggers(types);
        });
        ON_CALL(*this, triggerActivated).WillByDefault([this](const auto *trigger) {
            this->TouchpadTriggerHandler::triggerActivated(trigger);
        });
    }

    MOCK_METHOD(void, activatingTriggers, (TriggerTypes types), (override));
    MOCK_METHOD(bool, handleMotion, (const InputDevice *device, const PointDelta &delta), (override));
    MOCK_METHOD(TriggerManagementOperationResult, doCancelTriggers, (TriggerTypes types), (override));
    MOCK_METHOD(TriggerManagementOperationResult, doEndTriggers, (TriggerTypes types), (override));
    MOCK_METHOD(void, triggerActivated, (const Trigger *trigger), (override));
};

}