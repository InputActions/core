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

#include "ConfigLoader.h"
#include "ConfigIssue.h"
#include "ConfigIssueManager.h"
#include "GlobalConfig.h"
#include "InputActionsMain.h"
#include "Node.h"
#include "parsers/containers.h"
#include "parsers/core.h"
#include "parsers/utils.h"
#include <QDir>
#include <QFileInfo>
#include <libinputactions/actions/ActionExecutor.h>
#include <libinputactions/handlers/KeyboardTriggerHandler.h>
#include <libinputactions/handlers/MouseTriggerHandler.h>
#include <libinputactions/handlers/PointerTriggerHandler.h>
#include <libinputactions/handlers/TouchpadTriggerHandler.h>
#include <libinputactions/handlers/TouchscreenTriggerHandler.h>
#include <libinputactions/input/backends/LibevdevComplementaryInputBackend.h>
#include <libinputactions/input/devices/InputDeviceRule.h>
#include <libinputactions/interfaces/ConfigProvider.h>
#include <libinputactions/interfaces/NotificationManager.h>
#include <libinputactions/scripting/ScriptingEngine.h>
#include <libinputactions/scripting/modules/core/Config.h>
#include <libinputactions/scripting/modules/core/CoreModule.h>
#include <libinputactions/scripting/modules/main/Script.h>
#include <libinputactions/variables/VariableRegistry.h>

namespace InputActions
{

struct ConfigData
{
    bool allowExternalVariableAccess = true;
    bool autoReload = true;
    bool libevdevEnabled = true;
    bool sendNotificationOnError = true;

    std::unique_ptr<KeyboardTriggerHandler> keyboardTriggerHandler;
    std::unique_ptr<MouseTriggerHandler> mouseTriggerHandler;
    std::unique_ptr<PointerTriggerHandler> pointerTriggerHandler;
    std::function<std::unique_ptr<TouchpadTriggerHandler>(InputDevice *device)> touchpadTriggerHandlerFactory;
    std::function<std::unique_ptr<TouchscreenTriggerHandler>(InputDevice *device)> touchscreenTriggerHandlerFactory;

    std::vector<InputDeviceRule> deviceRules;
    std::set<KeyboardKey> emergencyCombination = {KEY_BACKSPACE, KEY_SPACE, KEY_ENTER};
};

void ConfigLoader::loadEmpty()
{
    activateConfig({}, false);
}

bool ConfigLoader::load(const ConfigLoadSettings &settings)
{
    static const auto destroyEngine = [](std::shared_ptr<ScriptingEngine> &engine) {
        auto oldEngine = g_scriptingEngine;
        g_scriptingEngine = engine;
        if (auto *coreModule = engine->coreModule()) {
            Q_EMIT coreModule->config()->aboutToBeDestroyed();
        }
        g_scriptingEngine = oldEngine;
        engine.reset();
    };

    auto currentEngine = g_scriptingEngine;
    auto currentVariableRegistry = g_variableRegistry;
    try {
        qCDebug(INPUTACTIONS, "Reloading config");
        const auto rawConfig = settings.config.value_or(g_configProvider->currentConfig());

        g_configIssueManager = std::make_shared<ConfigIssueManager>(rawConfig);
        g_variableRegistry = std::make_shared<VariableRegistry>();
        g_inputActions->registerGlobalVariables(g_variableRegistry.get());
        g_scriptingEngine = std::make_shared<ScriptingEngine>(*g_variableRegistry.get());
        auto config = createConfig(rawConfig);
        destroyEngine(currentEngine);
        activateConfig(std::move(config), true);
    } catch (const ConfigException &e) {
        destroyEngine(g_scriptingEngine);
        g_scriptingEngine = currentEngine;
        g_variableRegistry = currentVariableRegistry;
        g_configIssueManager->addIssue(e);
    }

    const auto issues = g_configIssueManager->issues();
    const auto error = std::ranges::find_if(issues, [](const auto *issue) {
        return issue->severity() == ConfigIssueSeverity::Error;
    });

    if (error != issues.end()) {
        if (g_globalConfig->sendNotificationOnError() && !settings.manual) {
            g_notificationManager->sendNotification("Failed to load configuration",
                                                    (*error)->toString(false) + " Run 'inputactions config issues' for more information.");
        }

        return false;
    }

    return true;
}

ConfigData ConfigLoader::createConfig(const QString &raw)
{
    const auto root = Node::create(raw);
    if (root->isNull()) {
        return {};
    } else if (!root->isMap()) {
        throw InvalidNodeTypeConfigException(root.get(), NodeType::Map);
    }

    ConfigData config;

    if (const auto *scriptingNode = root->mapAt("scripting")) {
        if (const auto *scriptsNode = scriptingNode->at("scripts")) {
            for (const auto *scriptNode : scriptsNode->sequenceItems()) {
                if (const auto *sourceNode = scriptNode->at("source")) {
                    const auto source = sourceNode->as<QString>();
                    const auto result = g_scriptingEngine->evaluate(sourceNode->as<QString>());
                    if (result.isError()) {
                        throw UncaughtScriptErrorConfigException(sourceNode, result);
                    }
                } else if (const auto *fileNode = scriptNode->at("file", true)) {
                    const QFileInfo file(fileNode->as<QString>());
                    if (!file.exists()) {
                        throw InvalidValueConfigException(fileNode, "File does not exist.");
                    }

                    const auto result = g_scriptingEngine->importModule(file.canonicalFilePath());
                    if (result.isError()) {
                        throw UncaughtScriptErrorConfigException(fileNode, result);
                    }

                    const auto defaultFunc = result.property("default");
                    if (!defaultFunc.isCallable()) {
                        continue;
                    }

                    const auto rootDirectory = file.dir().absolutePath();
                    const auto defaultFuncResult = ScriptingEngine::call(defaultFunc,
                                                                         {g_scriptingEngine->ensureEngine().newQObject(new Script(rootDirectory))});
                    if (defaultFuncResult.isError()) {
                        throw UncaughtScriptErrorConfigException(fileNode, defaultFuncResult);
                    }
                }
            }
        }
    }

    if (auto *coreModule = g_scriptingEngine->coreModule()) {
        coreModule->variableRegistry()->disableRegistration();
    }

    loadMember(config.autoReload, root->at("autoreload"));
    loadMember(config.allowExternalVariableAccess, root->at("external_variable_access"));
    if (const auto *notificationsNode = root->mapAt("notifications")) {
        loadMember(config.sendNotificationOnError, notificationsNode->at("config_error"));
    }
    loadMember(config.libevdevEnabled, root->at("__libevdev_enabled"));
    loadMember(config.deviceRules, root.get());
    loadMember(config.emergencyCombination, root->at("emergency_combination"));

    loadMember(config.keyboardTriggerHandler, root->mapAt("keyboard"));
    loadMember(config.mouseTriggerHandler, root->mapAt("mouse"));
    loadMember(config.pointerTriggerHandler, root->mapAt("pointer"));

    if (const auto *touchpadNode = root->mapAt("touchpad")) {
        config.touchpadTriggerHandlerFactory = [touchpadNode = touchpadNode->shared_from_this()](auto *device) {
            return parseTouchpadTriggerHandler(touchpadNode.get(), device);
        };
        config.touchpadTriggerHandlerFactory(nullptr); // Make sure it doesn't throw
    }
    if (const auto *touchscreenNode = root->mapAt("touchscreen")) {
        config.touchscreenTriggerHandlerFactory = [touchscreenNode = touchscreenNode->shared_from_this()](auto *device) {
            return parseTouchscreenTriggerHandler(touchscreenNode.get(), device);
        };
        config.touchscreenTriggerHandlerFactory(nullptr);
    }

    root->at("anchors"); // Allow users to define anchors somewhere without unused property issues
    root->addUnusedMapPropertyIssues();
    return config;
}

void ConfigLoader::activateConfig(ConfigData config, bool initialize)
{
    g_inputBackend->reset(); // Okay because required keys are not cleared
    g_actionExecutor->clearQueue();
    g_actionExecutor->waitForDone();

    auto *coreModule = g_scriptingEngine->coreModule();
    if (coreModule) {
        Q_EMIT coreModule->config()->aboutToBeActivated();
    }

    g_globalConfig->setAllowExternalVariableAccess(config.allowExternalVariableAccess);
    g_globalConfig->setAutoReload(config.autoReload);
    g_globalConfig->setSendNotificationOnError(config.sendNotificationOnError);

    if (auto *libevdev = dynamic_cast<LibevdevComplementaryInputBackend *>(g_inputBackend.get())) {
        libevdev->setEnabled(config.libevdevEnabled);
    }

    g_inputBackend->setKeyboardTriggerHandler(std::move(config.keyboardTriggerHandler));
    g_inputBackend->setMouseTriggerHandler(std::move(config.mouseTriggerHandler));
    g_inputBackend->setPointerTriggerHandler(std::move(config.pointerTriggerHandler));
    g_inputBackend->setTouchpadTriggerHandlerFactory(config.touchpadTriggerHandlerFactory);
    g_inputBackend->setTouchscreenTriggerHandlerFactory(config.touchscreenTriggerHandlerFactory);
    g_inputBackend->setDeviceRules(config.deviceRules);
    g_inputBackend->setEmergencyCombination(config.emergencyCombination);

    if (initialize) {
        g_inputBackend->initialize();
    }

    if (coreModule) {
        Q_EMIT coreModule->config()->activated();
    }
}

}