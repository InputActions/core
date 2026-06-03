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

#include "VariableOperations.h"
#include <QObject>
#include <QString>

namespace InputActions
{

class Variable : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ value)

public:
    Variable(QMetaType type);
    virtual ~Variable() = default;

    /**
     * @return May be empty.
     */
    virtual QVariant value() const { return {}; }
    /**
     * @param value Must be the same as the variable's type or empty.
     */
    virtual void setValue(QVariant value) {}

    /**
     * @return Operations for this variable's type.
     */
    const VariableOperationsBase *operations() const;

    const QMetaType &type() const;

    /**
     * Whether the value should not be shown in the DBus interface.
     */
    bool hidden() const { return m_hidden; }
    void setHidden(bool value) { m_hidden = value; }

private:
    QMetaType m_type;
    std::variant<bool, QString> m_value;
    std::unique_ptr<VariableOperationsBase> m_operations;
    bool m_hidden{};
};

}