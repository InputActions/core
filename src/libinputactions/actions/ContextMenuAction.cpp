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

#include "ContextMenuAction.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QUuid>
#include <libinputactions/actions/ActionExecutor.h>
#include <libinputactions/helpers/QThread.h>
#include <libinputactions/interfaces/OverlayManager.h>

namespace InputActions
{

ActionMenuItem::ActionMenuItem(std::vector<std::unique_ptr<Action>> actions)
    : m_actions(std::move(actions))
    , m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
{
}

QJsonObject ActionMenuItem::toJson() const
{
    return QJsonObject{
        {"type", "action"},
        {"text", m_text},
        {"icon", m_icon},
        {"id", m_id},
    };
}

QJsonObject SectionMenuItem::toJson() const
{
    return QJsonObject{
        {"type", "section"},
        {"text", m_text},
    };
}

QJsonObject SeparatorMenuItem::toJson() const
{
    return QJsonObject{
        {"type", "separator"},
    };
}

SubMenuMenuItem::SubMenuMenuItem(std::vector<std::unique_ptr<MenuItem>> items)
    : m_items(std::move(items))
{
}

ContextMenuAction::~ContextMenuAction()
{
    for (const auto &future : m_futures) {
        future->cancel();
    }
}

QJsonObject SubMenuMenuItem::toJson() const
{
    QJsonArray items;
    for (const auto &item : m_items) {
        items.append(item->toJson());
    }
    return QJsonObject{
        {"type", "menu"},
        {"text", m_text},
        {"icon", m_icon},
        {"items", items},
    };
}

ContextMenuAction::ContextMenuAction(std::vector<std::unique_ptr<MenuItem>> items)
    : m_items(std::move(items))
{
    QJsonArray jsonItems;
    std::vector<ActionMenuItem *> actions;
    for (const auto &item : m_items) {
        extractActions(*item, actions);
        jsonItems.append(item->toJson());
    }
    m_json = QJsonDocument(jsonItems).toJson(QJsonDocument::Compact);

    for (auto *action : actions) {
        m_actions[action->id()] = action;
    }
}

void ContextMenuAction::doExecute(const ActionExecutionArguments &args)
{
    QThreadHelpers::runOnThread(QThreadHelpers::mainThread(), [this]() {
        auto future = std::make_shared<QFuture<QString>>(g_overlayManager->showContextMenu(m_json));
        m_futures.push_back(future);
        future->then([this, future = future.get()](const QString &actionId) {
            if (m_actions.contains(actionId)) {
                for (auto &action : m_actions.at(actionId)->actions()) {
                    g_actionExecutor->execute(*action);
                }
            }

            std::erase_if(m_futures, [future](const auto &value) {
                return value.get() == future;
            });
        });
    });
}

void ContextMenuAction::extractActions(MenuItem &item, std::vector<ActionMenuItem *> &result)
{
    if (auto *actionItem = dynamic_cast<ActionMenuItem *>(&item)) {
        result.push_back(actionItem);
    } else if (auto *subMenuItem = dynamic_cast<SubMenuMenuItem *>(&item)) {
        for (const auto &item : subMenuItem->items()) {
            extractActions(*item, result);
        }
    }
}

}