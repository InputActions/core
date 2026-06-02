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

#include <QObject>
#include <QPointF>

namespace InputActions
{

class PointF
{
    Q_GADGET

    Q_PROPERTY(double x READ x WRITE setX)
    Q_PROPERTY(double y READ y WRITE setY)

public:
    Q_INVOKABLE PointF();
    Q_INVOKABLE PointF(double x, double y);
    PointF(QPointF point);

    double x() const { return m_x; }
    void setX(double value) { m_x = value; }

    double y() const { return m_y; }
    void setY(double value) { m_y = value; }

    Q_INVOKABLE bool isNull() const;

    Q_INVOKABLE double hypot() const;

    bool operator==(const PointF &other) const;
    bool operator!=(const PointF &other) const;

    PointF operator+(const PointF &other) const;
    PointF operator-(const PointF &other) const;

    PointF &operator+=(const PointF &other);
    PointF &operator-=(const PointF &other);

    PointF operator*(double x) const;
    PointF operator/(double x) const;

    PointF &operator*=(double x);
    PointF &operator/=(double x);

    operator QPointF() const;

private:
    double m_x;
    double m_y;
};

}

Q_DECLARE_METATYPE(InputActions::PointF)