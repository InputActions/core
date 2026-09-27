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

#include "Action.h"

namespace InputActions
{

class MenuItem
{
public:
    virtual ~MenuItem() = default;

    virtual QJsonObject toJson() const = 0;

protected:
    MenuItem() = default;
};

class ActionMenuItem : public MenuItem
{
public:
    ActionMenuItem(std::vector<std::unique_ptr<Action>> actions);

    const std::vector<std::unique_ptr<Action>> &actions() const { return m_actions; }

    const QString &text() const { return m_text; }
    void setText(QString value) { m_text = std::move(value); }

    const QString &icon() const { return m_icon; }
    void setIcon(QString value) { m_icon = std::move(value); }

    const QString &id() const { return m_id; }

    virtual QJsonObject toJson() const override;

private:
    std::vector<std::unique_ptr<Action>> m_actions;
    QString m_text;
    QString m_icon;
    QString m_id;
};

class SectionMenuItem : public MenuItem
{
public:
    SectionMenuItem() = default;

    const QString &text() const { return m_text; }
    void setText(QString value) { m_text = std::move(value); }

    virtual QJsonObject toJson() const override;

private:
    QString m_text;
};

class SeparatorMenuItem : public MenuItem
{
public:
    SeparatorMenuItem() = default;

    virtual QJsonObject toJson() const override;
};

class SubMenuMenuItem : public MenuItem
{
public:
    SubMenuMenuItem(std::vector<std::unique_ptr<MenuItem>> items);

    const std::vector<std::unique_ptr<MenuItem>> &items() const { return m_items; }

    const QString &text() const { return m_text; }
    void setText(QString value) { m_text = std::move(value); }

    const QString &icon() const { return m_icon; }
    void setIcon(QString value) { m_icon = std::move(value); }

    virtual QJsonObject toJson() const override;

private:
    std::vector<std::unique_ptr<MenuItem>> m_items;
    QString m_text;
    QString m_icon;
};

class ContextMenuAction : public Action
{
public:
    ContextMenuAction(std::vector<std::unique_ptr<MenuItem>> items);
    ~ContextMenuAction() override;

protected:
    void doExecute(const ActionExecutionArguments &args) override;

private:
    static void extractActions(MenuItem &item, std::vector<ActionMenuItem *> &result);

    std::vector<std::unique_ptr<MenuItem>> m_items;
    std::map<QString, ActionMenuItem *> m_actions;
    QString m_json;

    std::vector<std::shared_ptr<QFuture<QString>>> m_futures;
};

}