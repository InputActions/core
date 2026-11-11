/*
    Input Actions - Input handler that executes user-defined actions
    Copyright (C) 2026 Owen271

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

#include "DirectionalAction.h"
#include "ActionExecutor.h"
#include <QCoreApplication>
#include <QThread>
#include <algorithm>
#include <cmath>
#include <libinputactions/conditions/Condition.h>
#include <libinputactions/helpers/QThread.h>
#include <numbers>

namespace InputActions
{

/**
 * Limits the actions executed for a single large motion event.
 */
static const int MAX_ACTIONS_PER_UPDATE = 8;

/**
 * @return Index into Bindings for the dominant axis.
 */
static size_t direction(qreal x, qreal y)
{
    if (std::abs(x) >= std::abs(y)) {
        return x < 0 ? 0 : 1;
    }
    return y < 0 ? 2 : 3;
}

/**
 * Motion against the accumulated direction restarts accumulation.
 */
static void accumulate(qreal &total, qreal delta)
{
    total = delta != 0 && total * delta < 0 ? delta : total + delta;
}

DirectionalAction::DirectionalAction(qreal distance, Bindings actions, Bindings initial)
    : m_distance(distance)
    , m_actions(std::move(actions))
    , m_initial(std::move(initial))
{
}

void DirectionalAction::reset()
{
    Action::reset();
    m_firstUpdate = true;
    m_x = m_y = 0;
    for (const auto *bindings : {&m_actions, &m_initial}) {
        for (const auto &action : *bindings) {
            if (action) {
                action->reset();
            }
        }
    }
}

void DirectionalAction::doExecute(const ActionExecutionArguments &args)
{
    if (QThread::currentThread() == QThreadHelpers::mainThread()) {
        move(args);
        return;
    }

    // The repeat condition must be checked when the actions are executed, otherwise input could reach an application after the controlled UI has closed.
    // Not blocking, as the main thread may be waiting for this thread, for example while reloading the configuration.
    QMetaObject::invokeMethod(
        QCoreApplication::instance(),
        [this, alive = std::weak_ptr(m_alive), args] {
            // The action may have been destroyed while the motion was queued.
            if (alive.lock()) {
                move(args);
            }
        },
        Qt::QueuedConnection);
}

void DirectionalAction::move(const ActionExecutionArguments &args)
{
    const auto delta = args.inputActionArgs.motionPointDelta;

    // The initial action waits for motion. Only swipe triggers provide two-axis motion.
    if (m_firstUpdate && delta.x() == 0 && delta.y() == 0) {
        return;
    }

    const auto repeatAllowed = !m_repeatCondition || m_repeatCondition->satisfied();
    if (m_firstUpdate) {
        m_firstUpdate = false;
        const auto hasInitial = std::ranges::any_of(m_initial, [](const auto &action) {
            return action != nullptr;
        });
        // If the controlled UI is already open, navigate it instead of opening it again.
        if (hasInitial && !(m_repeatCondition && repeatAllowed)) {
            auto initialDirection = direction(delta.x(), delta.y());
            if (args.inputActionArgs.motionAngle) {
                const auto angle = *args.inputActionArgs.motionAngle * std::numbers::pi / 180;
                initialDirection = direction(std::cos(angle), -std::sin(angle));
            }
            executeAction(m_initial[initialDirection].get(), args);
            // The motion that executed the initial action does not count towards the next one.
            return;
        }
    }

    if (!repeatAllowed) {
        m_x = m_y = 0;
        return;
    }

    accumulate(m_x, delta.x());
    accumulate(m_y, delta.y());
    for (auto executed = 0; executed < MAX_ACTIONS_PER_UPDATE && std::max(std::abs(m_x), std::abs(m_y)) >= m_distance; executed++) {
        // The previous action may have closed the controlled UI.
        if (executed && m_repeatCondition && !m_repeatCondition->satisfied()) {
            m_x = m_y = 0;
            return;
        }

        const auto selected = direction(m_x, m_y);
        executeAction(m_actions[selected].get(), args);
        if (selected <= 1) {
            m_x += m_x < 0 ? m_distance : -m_distance;
            m_y = 0;
        } else {
            m_y += m_y < 0 ? m_distance : -m_distance;
            m_x = 0;
        }
    }
}

void DirectionalAction::executeAction(Action *action, const ActionExecutionArguments &args)
{
    if (!action || !action->canExecute()) {
        return;
    }
    g_actionExecutor->execute(*action,
                              {
                                  // Synchronous actions are executed immediately after the repeat condition has been checked. Asynchronous actions would
                                  // block the main thread.
                                  .thread = action->async() ? ActionThread::Auto : ActionThread::Current,
                                  .actionArgs = {
                                      .inputActionArgs = args.inputActionArgs,
                                  },
                              });
}

}