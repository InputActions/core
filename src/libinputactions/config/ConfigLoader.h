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

#include "ConfigProvider.h"
#include <QObject>
#include <QString>
#include <memory>
#include <optional>

namespace InputActions
{

struct ConfigData;

struct ConfigLoadSettings
{
    /**
     * Whether to load an empty configuration. If no configuration has been loaded previously, this is an instant operation.
     */
    bool empty{};
    /**
     * Whether the reload was manually initiated using the control tool.
     */
    bool manual{};
};

class ConfigLoader : public QObject
{
    Q_OBJECT

public:
    ConfigLoader();

    /**
     * The returned future never fails. Errors may be obtained from ConfigIssueManager.
     */
    QFuture<void> load(const ConfigLoadSettings &settings = {});

    /**
     * Whether loading non-empty configurations should be permitted. This is only used in the standalone implementation.
     */
    void setAllowNonEmptyConfigs(bool value) { m_allowNonEmptyConfigs = value; }

private slots:
    void onConfigChanged();

private:
    QFuture<void> doLoad(const ConfigLoadSettings &settings);
    QFuture<std::shared_ptr<ConfigData>> createConfig(const QString &raw);
    QFuture<void> activateConfig(std::shared_ptr<ConfigData> config, bool initialize);

    QFuture<void> m_currentFuture;

    ConfigProvider m_configProvider;
    bool m_allowNonEmptyConfigs = true;
};

inline std::shared_ptr<ConfigLoader> g_configLoader;

}