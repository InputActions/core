#pragma once

#include <gmock/gmock.h>
#include <libinputactions/handlers/PointerTriggerHandler.h>

namespace InputActions
{

class MockPointerTriggerHandler : public PointerTriggerHandler
{
public:
    MockPointerTriggerHandler()
    {
        ON_CALL(*this, triggerActivated).WillByDefault([this](const auto &trigger) {
            return this->PointerTriggerHandler::triggerActivated(trigger);
        });
    }

    MOCK_METHOD(void, triggerActivated, (const Trigger &trigger), (override));
};

}