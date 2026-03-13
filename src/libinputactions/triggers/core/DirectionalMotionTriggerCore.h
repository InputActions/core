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

#pragma once

#include "MotionTriggerCore.h"

namespace InputActions
{

typedef uint32_t TriggerDirection;
enum class PinchDirection : TriggerDirection
{
    In = 1u << 0,
    Out = 1u << 1,
    Any = UINT32_MAX
};
enum class RotateDirection : TriggerDirection
{
    Clockwise = 1u << 2,
    Counterclockwise = 1u << 3,
    Any = UINT32_MAX
};
enum class SimpleSwipeDirection : TriggerDirection
{
    Left = 1u << 4,
    Right = 1u << 5,
    Up = 1u << 6,
    Down = 1u << 7,
    LeftRight = Left | Right,
    UpDown = Up | Down,
    Any = UINT32_MAX
};

class DirectionalMotionTriggerUpdateEvent : public MotionTriggerUpdateEvent
{
public:
    DirectionalMotionTriggerUpdateEvent() = default;

    TriggerDirection direction() const { return m_direction; }
    void setDirection(TriggerDirection value) { m_direction = value; }

private:
    TriggerDirection m_direction = UINT32_MAX;
};

/**
 * An input action that involves directional motion.
 */
class DirectionalMotionTriggerCore : public MotionTriggerCore
{
public:
    bool canUpdate(const TriggerUpdateEvent &event) const override;
    void updateActions(const std::vector<std::unique_ptr<TriggerAction>> &actions, const TriggerUpdateEvent &event) const override;

protected:
    DirectionalMotionTriggerCore(TriggerDirection direction);

private:
    TriggerDirection m_direction;

    friend class TestDirectionalMotionTriggerCore;
};

class PinchTriggerCore : public DirectionalMotionTriggerCore
{
public:
    PinchTriggerCore(PinchDirection direction);
};

class RotateTriggerCore : public DirectionalMotionTriggerCore
{
public:
    RotateTriggerCore(RotateDirection direction);
};

/**
 * Unlike SwipeTriggerCore, is not based on angles and does not support diagonal directions.
 */
class SimpleSwipeTriggerCore : public DirectionalMotionTriggerCore
{
public:
    SimpleSwipeTriggerCore(SimpleSwipeDirection direction);
};

}