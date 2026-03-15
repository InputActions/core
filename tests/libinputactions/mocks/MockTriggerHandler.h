#pragma once

#include <gmock/gmock.h>
#include <libinputactions/handlers/TriggerHandler.h>

namespace InputActions
{

class MockTriggerHandler : public TriggerHandler
{
public:
    MockTriggerHandler()
    {
        ON_CALL(*this, doCancelTriggers).WillByDefault([this](const auto types) {
            return this->TriggerHandler::doCancelTriggers(types);
        });
    }

    MOCK_METHOD(TriggerManagementOperationResult, doCancelTriggers, (TriggerTypes types), (override));
};

}