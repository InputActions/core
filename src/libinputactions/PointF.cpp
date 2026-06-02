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

#include "PointF.h"

namespace InputActions
{

PointF::PointF()
    : PointF(0, 0)
{
}

PointF::PointF(double x, double y)
    : m_x(x)
    , m_y(y)
{
}

PointF::PointF(QPointF point)
    : m_x(point.x())
    , m_y(point.y())
{
}

double PointF::hypot() const
{
    return std::hypot(m_x, m_y);
}

bool PointF::isNull() const
{
    return !m_x && !m_y;
}

bool PointF::operator==(const PointF &other) const
{
    return m_x == other.m_x && m_y == other.m_y;
}

bool PointF::operator!=(const PointF &other) const
{
    return m_x != other.m_x || m_y != other.m_y;
}

PointF PointF::operator+(const PointF &other) const
{
    return {m_x + other.m_x, m_y + other.m_y};
}

PointF PointF::operator-(const PointF &other) const
{
    return {m_x - other.m_x, m_y - other.m_y};
}

PointF &PointF::operator+=(const PointF &other)
{
    m_x += other.m_x;
    m_y += other.m_y;
    return *this;
}

PointF &PointF::operator-=(const PointF &other)
{
    m_x -= other.m_x;
    m_y -= other.m_y;
    return *this;
}

PointF PointF::operator*(double x) const
{
    return {m_x * x, m_y * x};
}

PointF PointF::operator/(double x) const
{
    return {m_x / x, m_y / x};
}

PointF &PointF::operator*=(double x)
{
    m_x *= x;
    m_y *= x;
    return *this;
}

PointF &PointF::operator/=(double x)
{
    m_x /= x;
    m_y /= x;
    return *this;
}

PointF::operator QPointF() const
{
    return QPointF(m_x, m_y);
}

}