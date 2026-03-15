#pragma once

#include <gmock/gmock.h>
#include <libinputactions/handlers/InputTriggerHandler.h>

namespace InputActions
{

class MockInputTriggerHandler : public InputTriggerHandler
{
public:
    MockInputTriggerHandler()
    {
        ON_CALL(*this, activatingTriggers).WillByDefault([this](const auto types) {
            return this->InputTriggerHandler::activatingTriggers(types);
        });
        ON_CALL(*this, doEndTriggers).WillByDefault([this](const auto types) {
            return this->InputTriggerHandler::doEndTriggers(types);
        });
    }

    MOCK_METHOD(void, activatingTriggers, (TriggerTypes types), (override));
    MOCK_METHOD(TriggerManagementOperationResult, doEndTriggers, (TriggerTypes types), (override));
};

}