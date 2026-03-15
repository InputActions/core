#pragma once

#include <gmock/gmock.h>
#include <libinputactions/handlers/KeyboardTriggerHandler.h>

namespace InputActions
{

class MockKeyboardTriggerHandler : public KeyboardTriggerHandler
{
public:
    MockKeyboardTriggerHandler()
    {
        ON_CALL(*this, triggerActivated).WillByDefault([this](const auto *trigger) {
            this->KeyboardTriggerHandler::triggerActivated(trigger);
        });
        ON_CALL(*this, doEndTriggers).WillByDefault([this](const auto types) {
            return this->KeyboardTriggerHandler::doEndTriggers(types);
        });
    }

    MOCK_METHOD(void, triggerActivated, (const Trigger *trigger), (override));
    MOCK_METHOD(TriggerManagementOperationResult, doEndTriggers, (TriggerTypes types), (override));
};

}