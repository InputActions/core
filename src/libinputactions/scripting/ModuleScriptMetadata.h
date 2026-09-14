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

namespace InputActions
{

class ModuleScriptMetadata
{
    Q_GADGET

    Q_PROPERTY(QString id READ id)
    Q_PROPERTY(QString mainModule READ mainModule)

public:
    ModuleScriptMetadata() = default;

    /**
     * Unique script identifier in reverse domain name notation.
     */
    const QString &id() const { return m_id; }
    void setId(QString value) { m_id = std::move(value); }

    /**
     * Path to the main module relative to the metadata file's path.
     */
    const QString &mainModule() const { return m_mainModule; }
    void setMainModule(QString value) { m_mainModule = std::move(value); }

private:
    QString m_id;
    QString m_mainModule;
};

}