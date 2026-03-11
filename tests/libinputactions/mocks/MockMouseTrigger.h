#pragma once

#include <libinputactions/triggers/Trigger.h>
#include <libinputactions/triggers/core/TriggerCore.h>
#include <libinputactions/triggers/mouse/MouseTrigger.h>

namespace InputActions
{

class MockMouseTrigger : public MouseTrigger
{
public:
    MockMouseTrigger()
        : MouseTrigger(TriggerType::None, std::make_unique<TriggerCore>())
    {
    }
};

}