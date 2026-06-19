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

#include "Variable.h"

namespace InputActions
{

template<typename T>
class TypedVariable
{
public:
    TypedVariable(Variable *variable)
        : m_variable(variable)
    {
    }

    std::optional<T> value() const
    {
        const auto value = m_variable->value();
        if (value.isNull()) {
            return {};
        }
        return value.value<T>();
    }

    void setValue(const std::optional<T> &value)
    {
        if (!value.has_value()) {
            m_variable->setValue({});
            return;
        }
        m_variable->setValue(QVariant::fromValue(value.value()));
    }

private:
    Variable *m_variable;
};

}