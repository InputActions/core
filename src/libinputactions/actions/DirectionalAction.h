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

#pragma once

#include "Action.h"
#include <array>

namespace InputActions
{

class Condition;

/**
 * Converts two-axis motion into discrete actions. Every time the motion along the dominant axis reaches the distance, the action for that direction is
 * executed.
 */
class DirectionalAction : public Action
{
public:
    /**
     * Actions ordered left, right, up, down. Each may be nullptr.
     */
    using Bindings = std::array<std::unique_ptr<Action>, 4>;

    /**
     * @param distance Motion required per action. Must be greater than zero.
     * @param initial Executed on the first update with motion instead of waiting for the distance, using the swipe angle if available.
     */
    DirectionalAction(qreal distance, Bindings actions, Bindings initial = {});

    qreal distance() const { return m_distance; }
    const Bindings &actions() const { return m_actions; }
    const Bindings &initial() const { return m_initial; }

    /**
     * Checked before executing each action. While not satisfied, motion is discarded. If satisfied on the first update with motion, the initial action is not
     * executed.
     * May be nullptr.
     */
    const std::shared_ptr<Condition> &repeatCondition() const { return m_repeatCondition; }
    void setRepeatCondition(std::shared_ptr<Condition> value) { m_repeatCondition = std::move(value); }

    void reset() override;

protected:
    void doExecute(const ActionExecutionArguments &args) override;

private:
    void move(const ActionExecutionArguments &args);
    /**
     * Must be called from the main thread.
     */
    static void executeAction(Action *action, const ActionExecutionArguments &args);

    qreal m_distance;
    Bindings m_actions;
    Bindings m_initial;
    std::shared_ptr<Condition> m_repeatCondition;

    /**
     * Expires when the action is destroyed, for motion queued on the main thread.
     */
    std::shared_ptr<bool> m_alive = std::make_shared<bool>();

    bool m_firstUpdate = true;
    qreal m_x{};
    qreal m_y{};
};

}