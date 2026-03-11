#pragma once

#include <libinputactions/triggers/Trigger.h>
#include <libinputactions/triggers/core/TriggerCore.h>

namespace InputActions
{

class MockTrigger : public Trigger
{
public:
    MockTrigger()
        : Trigger(TriggerType::None, std::make_unique<TriggerCore>())
    {
    }
};

}