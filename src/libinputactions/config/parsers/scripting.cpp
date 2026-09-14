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

#include "scripting.h"
#include "utils.h"
#include <QRegularExpression>
#include <libinputactions/config/Node.h>
#include <libinputactions/scripting/ModuleScriptMetadata.h>
#include <libinputactions/scripting/ScriptingEngine.h>

namespace InputActions
{

QJSValue parseFunction(const Node *node)
{
    auto result = g_scriptingEngine->evaluate(node->as<QString>());
    if (result.isError()) {
        throw UncaughtScriptErrorConfigException(node, result);
    }
    if (!result.isCallable()) {
        throw InvalidValueConfigException(node, "Expression is not a function.");
    }

    return result;
}

template<>
void NodeParser<ModuleScriptMetadata>::parse(const Node *node, ModuleScriptMetadata &result)
{
    loadSetter(result, &ModuleScriptMetadata::setId, node->at("id", true));
    loadSetter(result, &ModuleScriptMetadata::setMainModule, node->at("main_module", true));

    // Version will be used in the future
    const auto *versionNode = node->at("version", true);
    const auto rawVersion = versionNode->as<QString>();

    static const QRegularExpression versionValidationRegex("^(\\d+)\\.(\\d+)\\.(\\d+)$");
    const auto match = versionValidationRegex.match(rawVersion);
    if (!match.hasMatch()) {
        throw InvalidValueConfigException(versionNode, "Invalid version.");
    }

    if (match.capturedTexts()[1].toInt() > 255 || match.capturedTexts()[2].toInt() > 255 || match.capturedTexts()[3].toInt() > 255) {
        throw InvalidValueConfigException(versionNode, "Version out of range.");
    }
}

}