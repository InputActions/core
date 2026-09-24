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
#include "ConfigProvider.h"
#include "GlobalConfig.h"
#include "InputActionsMain.h"
#include "Node.h"
#include "parsers/containers.h"
#include "parsers/core.h"
#include "parsers/utils.h"
#include <QDir>
#include <QFileInfo>
#include <libinputactions/InputActionsMain.h>
#include <libinputactions/actions/ActionExecutor.h>
#include <libinputactions/handlers/KeyboardTriggerHandler.h>
#include <libinputactions/handlers/MouseTriggerHandler.h>
#include <libinputactions/handlers/PointerTriggerHandler.h>
#include <libinputactions/handlers/TouchpadTriggerHandler.h>
#include <libinputactions/handlers/TouchscreenTriggerHandler.h>
#include <libinputactions/helpers/QThread.h>
#include <libinputactions/input/backends/LibevdevComplementaryInputBackend.h>
#include <libinputactions/input/devices/InputDeviceRule.h>
#include <libinputactions/interfaces/NotificationManager.h>
#include <libinputactions/interfaces/OverlayManager.h>
#include <libinputactions/scripting/ModuleScriptMetadata.h>
#include <libinputactions/scripting/ScriptingEngine.h>
#include <libinputactions/scripting/modules/core/Config.h>
#include <libinputactions/scripting/modules/core/CoreModule.h>
#include <libinputactions/scripting/modules/main/ModuleScript.h>
#include <libinputactions/scripting/promises/PromiseException.h>
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

    bool enableMouseStrokeOverlay{};
};

ConfigLoader::ConfigLoader()
{
    if (!g_inputActions->inTestEnvironment()) {
        connect(&m_configProvider, &ConfigProvider::configChanged, this, &ConfigLoader::onConfigChanged);
    }
}

QFuture<void> ConfigLoader::load(const ConfigLoadSettings &settings)
{
    QFuture<void> future;
    if (m_currentFuture.isFinished()) {
        future = doLoad(settings);
    } else {
        future = m_currentFuture
                     .then([this, settings]() {
                         return doLoad(settings);
                     })
                     .unwrap();
    }

    // Wrap future to allow multiple continuations
    auto promise = std::make_shared<QPromise<void>>();
    promise->start();
    m_currentFuture = future.then([promise]() {
        promise->finish();
    });
    return promise->future();
}

QFuture<void> ConfigLoader::doLoad(const ConfigLoadSettings &settings)
{
    if (!settings.empty && !m_allowNonEmptyConfigs) {
        return QtFuture::makeReadyVoidFuture();
    }

    const auto destroyEngine = [](ScriptingEngine *engine) {
        return engine->coreModule().config()->aboutToBeDestroyedSignal().emitAsync(false).then([engine]() {
            engine->deleteLater();
        });
    };

    auto currentEngine = g_scriptingEngine.release();
    auto currentVariableRegistry = g_variableRegistry;

    qCDebug(INPUTACTIONS, "Reloading config");
    const auto rawConfig = settings.empty ? "" : m_configProvider.currentConfig();

    g_configIssueManager->clearIssues();
    g_variableRegistry = std::make_shared<VariableRegistry>();
    g_inputActions->registerGlobalVariables(g_variableRegistry.get());
    g_scriptingEngine = std::make_unique<ScriptingEngine>(g_inputBackend, g_variableRegistry);

    auto promise = std::make_shared<QPromise<void>>();
    promise->start();
    createConfig(rawConfig)
        .then([this, promise, destroyEngine, currentEngine](const std::shared_ptr<ConfigData> config) {
            destroyEngine(currentEngine).then([this, promise, config] {
                activateConfig(config, true).then([promise, config]() {
                    promise->finish();
                });
            });
        })
        .onFailed([destroyEngine, currentEngine, currentVariableRegistry, settings, promise](const std::exception &e) {
            const auto &error = static_cast<const ConfigException &>(e);

            destroyEngine(g_scriptingEngine.release());
            g_scriptingEngine = std::unique_ptr<ScriptingEngine>(currentEngine);
            g_variableRegistry = currentVariableRegistry;
            g_configIssueManager->addIssue(error);

            if (g_globalConfig->sendNotificationOnError() && !settings.manual) {
                g_notificationManager->sendNotification("Failed to load configuration",
                                                        error.toString(false) + " Run 'inputactions config issues' for more information.");
            }

            promise->finish();
        });
    return promise->future();
}

QFuture<std::shared_ptr<ConfigData>> ConfigLoader::createConfig(const QString &raw)
{
    // Everything is in QFuture::then() because QPromise::setException doesn't work with 'const ConfigException &' in a catch block
    return QtFuture::makeReadyVoidFuture()
        .then([this, raw]() {
            const auto root = Node::create(raw, std::make_unique<NodeSourceFile>(m_configProvider.currentPath(), raw));
            if (root->isNull()) {
                return QtFuture::makeReadyValueFuture(std::make_shared<ConfigData>());
            } else if (!root->isMap()) {
                throw InvalidNodeTypeConfigException(root.get(), NodeType::Map);
            }

            if (const auto *scriptingNode = root->mapAt("scripting")) {
                if (const auto *scriptsNode = scriptingNode->at("scripts")) {
                    for (const auto *scriptNode : scriptsNode->sequenceItems()) {
                        if (const auto *sourceNode = scriptNode->at("source")) {
                            const auto source = sourceNode->as<QString>();
                            const auto result = g_scriptingEngine->evaluate(sourceNode->as<QString>());
                            if (result.isError()) {
                                throw UncaughtScriptErrorConfigException(sourceNode, result);
                            }
                        } else if (const auto *packageNode = scriptNode->at("package", true)) {
                            const QDir packageDir(packageNode->as<QString>());
                            if (!packageDir.exists()) {
                                throw InvalidValueConfigException(packageNode, "The specified script package directory does not exist.");
                            }

                            const auto metadataFilePath = packageDir.absolutePath() + "/metadata.yaml";
                            QFile metadataFile(metadataFilePath);
                            if (!metadataFile.open(QIODeviceBase::ReadOnly | QIODeviceBase::Text)) {
                                throw InvalidValueConfigException(packageNode,
                                                                  QString("Failed to open the metadata file: %1.").arg(metadataFile.errorString()));
                            }

                            const auto rawMetadata = QString::fromUtf8(metadataFile.readAll());
                            metadataFile.close();

                            const auto metadataNode = Node::create(rawMetadata, std::make_shared<NodeSourceFile>(metadataFilePath, rawMetadata));
                            const auto metadata = metadataNode->as<ModuleScriptMetadata>();

                            const QFileInfo mainModuleFileInfo(packageDir.absolutePath() + "/" + metadata.mainModule());
                            if (!mainModuleFileInfo.absoluteDir().absolutePath().startsWith(packageDir.absolutePath())) {
                                throw InvalidValueConfigException(metadataNode.get(),
                                                                  "The main module file cannot be located outside of the package directory.");
                            }
                            if (!mainModuleFileInfo.exists()) {
                                throw InvalidValueConfigException(metadataNode.get(), "The specified main module file does not exist.");
                            }

                            const auto mainModule = g_scriptingEngine->importModule(mainModuleFileInfo.absoluteFilePath());
                            if (mainModule.isError()) {
                                throw UncaughtScriptErrorConfigException(packageNode, mainModule);
                            }

                            const auto defaultFunc = mainModule.property("default");
                            if (defaultFunc.isError()) {
                                throw UncaughtScriptErrorConfigException(packageNode, defaultFunc);
                            } else if (!defaultFunc.isCallable()) {
                                continue;
                            }

                            const auto defaultFuncResult = ScriptingEngine::call(defaultFunc,
                                                                                 {g_scriptingEngine->qtEngine()
                                                                                      .newQObject(new ModuleScript(packageDir.absolutePath()))});
                            if (defaultFuncResult.isError()) {
                                throw UncaughtScriptErrorConfigException(packageNode, defaultFuncResult);
                            }
                        }
                    }
                }
            }

            return g_scriptingEngine->coreModule()
                .config()
                ->aboutToBeLoadedSignal()
                .emitAsync(true)
                .onFailed([root](const PromiseException &error) {
                    throw UncaughtScriptErrorConfigException(root.get(), error.value()); // TODO correct node
                })
                .then([root]() {
                    g_scriptingEngine->coreModule().variableRegistry()->disableRegistration();

                    auto config = std::make_shared<ConfigData>();
                    loadMember(config->autoReload, root->at("autoreload"));
                    loadMember(config->allowExternalVariableAccess, root->at("external_variable_access"));
                    if (const auto *notificationsNode = root->mapAt("notifications")) {
                        loadMember(config->sendNotificationOnError, notificationsNode->at("config_error"));
                    }
                    loadMember(config->libevdevEnabled, root->at("__libevdev_enabled"));
                    loadMember(config->deviceRules, root.get());
                    loadMember(config->emergencyCombination, root->at("emergency_combination"));

                    loadMember(config->keyboardTriggerHandler, root->mapAt("keyboard"));
                    loadMember(config->mouseTriggerHandler, root->mapAt("mouse"));
                    loadMember(config->pointerTriggerHandler, root->mapAt("pointer"));

                    if (const auto *touchpadNode = root->mapAt("touchpad")) {
                        config->touchpadTriggerHandlerFactory = [touchpadNode = touchpadNode->shared_from_this()](auto *device) {
                            return parseTouchpadTriggerHandler(touchpadNode.get(), device);
                        };
                        config->touchpadTriggerHandlerFactory(nullptr); // Make sure it doesn't throw
                    }
                    if (const auto *touchscreenNode = root->mapAt("touchscreen")) {
                        config->touchscreenTriggerHandlerFactory = [touchscreenNode = touchscreenNode->shared_from_this()](auto *device) {
                            return parseTouchscreenTriggerHandler(touchscreenNode.get(), device);
                        };
                        config->touchscreenTriggerHandlerFactory(nullptr);
                    }

                    if (const auto *overlayNode = root->mapAt("overlay")) {
                        if (const auto *mouseStrokeNode = overlayNode->mapAt("mouse_stroke")) {
                            loadMember(config->enableMouseStrokeOverlay, mouseStrokeNode->at("enable"));
                            if (config->enableMouseStrokeOverlay && !g_overlayManager->overlayProgramExists()) {
                                throw OverlayProgramDoesNotExistConfigException(overlayNode);
                            }
                        }
                    }

                    root->at("anchors"); // Allow users to define anchors somewhere without unused property issues
                    root->addUnusedMapPropertyIssues();
                    return config;
                });
        })
        .unwrap();
}

QFuture<void> ConfigLoader::activateConfig(std::shared_ptr<ConfigData> config, bool initialize)
{
    g_inputBackend->reset(); // Okay because required keys are not cleared
    g_actionExecutor->clearQueue();
    g_actionExecutor->waitForDone();

    auto *scriptingConfig = g_scriptingEngine->coreModule().config();

    return scriptingConfig->aboutToBeActivatedSignal().emitAsync(false).then([initialize, scriptingConfig, config]() {
        g_overlayManager->setEnabled(config->enableMouseStrokeOverlay);
        g_overlayManager->setMouseStrokeOverlayEnabled(config->enableMouseStrokeOverlay);

        g_globalConfig->setAllowExternalVariableAccess(config->allowExternalVariableAccess);
        g_globalConfig->setAutoReload(config->autoReload);
        g_globalConfig->setSendNotificationOnError(config->sendNotificationOnError);

        if (auto *libevdev = dynamic_cast<LibevdevComplementaryInputBackend *>(g_inputBackend.get())) {
            libevdev->setEnabled(config->libevdevEnabled);
        }

        g_inputBackend->setKeyboardTriggerHandler(std::move(config->keyboardTriggerHandler));
        g_inputBackend->setMouseTriggerHandler(std::move(config->mouseTriggerHandler));
        g_inputBackend->setPointerTriggerHandler(std::move(config->pointerTriggerHandler));
        g_inputBackend->setTouchpadTriggerHandlerFactory(config->touchpadTriggerHandlerFactory);
        g_inputBackend->setTouchscreenTriggerHandlerFactory(config->touchscreenTriggerHandlerFactory);
        g_inputBackend->setDeviceRules(config->deviceRules);
        g_inputBackend->setEmergencyCombination(config->emergencyCombination);

        if (initialize) {
            g_inputBackend->initialize();
        }

        scriptingConfig->activatedSignal().emit();
    });
}

void ConfigLoader::onConfigChanged()
{
    if (!m_allowNonEmptyConfigs || !g_globalConfig->autoReload()) {
        return;
    }
    load();
}

}