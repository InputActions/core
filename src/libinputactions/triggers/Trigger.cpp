/*
    Input Actions - Input handler that executes user-defined actions
    Copyright (C) 2024-2026 Marcin Woźniak

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "Trigger.h"
#include "core/TriggerCore.h"
#include <libinputactions/actions/InputAction.h>
#include <libinputactions/input/backends/InputBackend.h>
#include <libinputactions/variables/VariableManager.h>

namespace InputActions
{

static const std::chrono::milliseconds TICK_INTERVAL{5L};

Trigger::Trigger(TriggerType type, std::unique_ptr<TriggerCore> core)
    : m_type(type)
    , m_core(std::move(core))
{
    m_tickTimer.setTimerType(Qt::TimerType::PreciseTimer);
    connect(&m_tickTimer, &QTimer::timeout, this, &Trigger::onTick);

    m_resumeTimeoutTimer.setTimerType(Qt::TimerType::PreciseTimer);
    m_resumeTimeoutTimer.setSingleShot(true);
    connect(&m_resumeTimeoutTimer, &QTimer::timeout, this, &Trigger::onResumeTimeoutTimerTimeout);
}

Trigger::~Trigger() = default;

void Trigger::addAction(std::unique_ptr<TriggerAction> action)
{
    actionAdded(action.get());
    m_actions.push_back(std::move(action));
}

bool Trigger::canActivate(const TriggerActivationEvent &event) const
{
    return !m_activationCondition || m_activationCondition->satisfied();
}

bool Trigger::canUpdate(const TriggerUpdateEvent &event) const
{
    return m_core->canUpdate(event);
}

bool Trigger::endIfCannotUpdate() const
{
    return false;
}

void Trigger::update(const TriggerUpdateEvent &event)
{
    const auto delta = event.delta().unaccelerated();
    m_absoluteAccumulatedDelta += std::abs(delta);
    m_withinThreshold = !m_threshold || m_threshold->contains(m_absoluteAccumulatedDelta);
    if (!m_withinThreshold) {
        return;
    }

    if (!m_started) {
        m_started = true;
        m_tickTimer.start(TICK_INTERVAL);

        if (m_clearModifiers && *m_clearModifiers) {
            g_inputBackend->clearKeyboardModifiers();
        }

        for (const auto &action : m_actions) {
            action->triggerStarted();
        }
    }

    setLastTrigger();
    m_core->updateActions(m_actions, event);
}

bool Trigger::canEnd() const
{
    return m_withinThreshold && (!m_endCondition || m_endCondition->satisfied());
}

void Trigger::end(bool allowResuming)
{
    if (!m_started) {
        reset();
        return;
    }

    if (allowResuming && m_resumeTimeout.count()) {
        m_resumeTimeoutTimer.start(m_resumeTimeout.count());
        return;
    }

    setLastTrigger();
    for (const auto &action : m_actions) {
        action->triggerEnded();
    }
    reset();
}

void Trigger::cancel()
{
    if (!m_started) {
        reset();
        return;
    }

    for (const auto &action : m_actions) {
        action->triggerCancelled();
    }
    reset();
}

bool Trigger::overridesOtherTriggersOnEnd()
{
    if (!m_withinThreshold) {
        return false;
    }

    return std::ranges::any_of(m_actions, [](const auto &action) {
        return action->conflicting() && (action->on() == On::End || action->on() == On::EndCancel) && action->canExecute();
    });
}

bool Trigger::overridesOtherTriggersOnUpdate()
{
    if (!m_withinThreshold) {
        return false;
    }

    return std::ranges::any_of(m_actions, [](const auto &action) {
        return action->conflicting() && (action->action()->executions() || (action->on() == On::Update && action->canExecute()));
    });
}

bool Trigger::isResumeTimeoutTimerActive()
{
    return m_resumeTimeoutTimer.isActive();
}

void Trigger::stopResumeTimeoutTimer()
{
    m_resumeTimeoutTimer.stop();
}

void Trigger::actionAdded(TriggerAction *action)
{
    if (dynamic_cast<const InputAction *>(action->action())) {
        if (!m_clearModifiers) {
            m_clearModifiers = true;
        }
    }
}

void Trigger::onTick()
{
    if (!m_withinThreshold) {
        return;
    }

    for (const auto &action : m_actions) {
        action->triggerTick(TICK_INTERVAL.count());
    }
}

void Trigger::onResumeTimeoutTimerTimeout()
{
    if (canEnd()) {
        end(false);
    } else {
        cancel();
    }
}

void Trigger::setLastTrigger()
{
    if (m_setLastTrigger) {
        g_variableManager->getVariable(BuiltinVariables::LastTriggerId)->set(m_id);
        g_variableManager->getVariable(BuiltinVariables::LastTriggerTimestamp)
            ->set(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
    }
}

void Trigger::reset()
{
    m_started = false;
    m_absoluteAccumulatedDelta = 0;
    m_withinThreshold = false;
    m_tickTimer.stop();
}

const TriggerCore &Trigger::core() const
{
    return *m_core;
}

TriggerCore &Trigger::core()
{
    return *m_core;
}

}