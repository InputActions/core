#include "InputActionsMain.h"
#include "actions/ActionExecutor.h"
#include "config/ConfigIssueManager.h"
#include "config/ConfigLoader.h"
#include "config/GlobalConfig.h"
#include "input/StrokeRecorder.h"
#include "input/backends/InputBackend.h"
#include "interfaces/ConfigProvider.h"
#include "interfaces/CursorShapeProvider.h"
#include "interfaces/NotificationManager.h"
#include "interfaces/OnScreenMessageManager.h"
#include "interfaces/PointerPositionGetter.h"
#include "interfaces/PointerPositionSetter.h"
#include "interfaces/ProcessRunner.h"
#include "interfaces/SessionLock.h"
#include "interfaces/TextInput.h"
#include "interfaces/Window.h"
#include "interfaces/WindowProvider.h"
#include "interfaces/implementations/DBusNotificationManager.h"
#include "interfaces/implementations/DBusPlasmaGlobalShortcutInvoker.h"
#include "interfaces/implementations/FileConfigProvider.h"
#include "interfaces/implementations/ProcessRunnerImpl.h"
#include "scripting/ScriptingEngine.h"
#include "variables/VariableRegistry.h"
#include <QFile>
#include <QStandardPaths>

namespace InputActions
{

InputActionsMain::InputActionsMain()
{
    g_inputActions = this;
}

InputActionsMain::~InputActionsMain()
{
    // Release as many resources as possible when the compositor plugin is disabled (KWin doesn't unload plugins from the address space)
    g_cursorShapeProvider.reset();
    g_notificationManager.reset();
    g_onScreenMessageManager.reset();
    g_pointerPositionGetter.reset();
    g_pointerPositionSetter.reset();
    g_processRunner.reset();
    g_textInput.reset();
    g_sessionLock.reset();
    g_windowProvider.reset();

    g_actionExecutor.reset();
    g_configIssueManager.reset();
    g_configLoader.reset();
    g_globalConfig.reset();
    g_configProvider.reset();
    g_inputBackend.reset();
    g_scriptingEngine.reset();
    g_strokeRecorder.reset();
    g_variableRegistry.reset();
}

void InputActionsMain::suspend()
{
    g_inputBackend->reset();
}

void InputActionsMain::initialize()
{
    connect(g_configProvider.get(), &ConfigProvider::configChanged, this, &InputActionsMain::onConfigChanged);
    registerGlobalVariables(g_variableRegistry.get());

    g_configLoader->loadEmpty(); // Initialize default values
}

void InputActionsMain::onConfigChanged(const QString &config)
{
    if (g_globalConfig->autoReload()) {
        g_configLoader->load({
            .config = config,
        });
    }
}

void InputActionsMain::setMissingImplementations()
{
    setMissingImplementation<ConfigProvider, FileConfigProvider>(g_configProvider);
    setMissingImplementation(g_cursorShapeProvider);
    setMissingImplementation<NotificationManager, DBusNotificationManager>(g_notificationManager);
    setMissingImplementation(g_onScreenMessageManager);
    setMissingImplementation(g_pointerPositionGetter);
    setMissingImplementation(g_pointerPositionSetter);
    setMissingImplementation<PlasmaGlobalShortcutInvoker, DBusPlasmaGlobalShortcutInvoker>(g_plasmaGlobalShortcutInvoker);
    setMissingImplementation<ProcessRunner, ProcessRunnerImpl>(g_processRunner);
    setMissingImplementation(g_textInput);
    setMissingImplementation(g_sessionLock);
    setMissingImplementation(g_windowProvider);

    setMissingImplementation(g_actionExecutor);
    setMissingImplementation(g_configIssueManager);
    setMissingImplementation(g_configLoader);
    setMissingImplementation(g_globalConfig);
    setMissingImplementation(g_inputBackend);
    setMissingImplementation(g_scriptingEngine);
    setMissingImplementation(g_strokeRecorder);
    setMissingImplementation(g_variableRegistry);
}

void InputActionsMain::registerGlobalVariables(VariableRegistry *variableRegistry, std::shared_ptr<PointerPositionGetter> pointerPositionGetter,
                                               std::shared_ptr<WindowProvider> windowProvider)
{
    if (!pointerPositionGetter) {
        pointerPositionGetter = g_pointerPositionGetter;
    }
    if (!windowProvider) {
        windowProvider = g_windowProvider;
    }

    variableRegistry->registerComputed<CursorShape>("cursor_shape", [](auto &value) {
        value = g_cursorShapeProvider->cursorShape();
    });
    variableRegistry->registerStored(BuiltinVariables::DeviceName);
    for (auto i = 1; i <= FINGER_VARIABLE_COUNT; i++) {
        variableRegistry->registerStored<PointF>(QString("finger_%1_initial_position_percentage").arg(i));
        variableRegistry->registerStored<PointF>(QString("finger_%1_position_percentage").arg(i));
        variableRegistry->registerStored<qreal>(QString("finger_%1_pressure").arg(i));
    }
    variableRegistry->registerStored(BuiltinVariables::Fingers);
    variableRegistry->registerStored(BuiltinVariables::InitialWindowId);
    variableRegistry->registerStored(BuiltinVariables::InitialWindowUnderFingersId);
    variableRegistry->registerStored(BuiltinVariables::InitialWindowUnderPointerId);
    variableRegistry->registerComputed<Qt::KeyboardModifiers>(BuiltinVariables::KeyboardModifiers, [](auto &value) {
        value = g_inputBackend->keyboardModifiers();
    });
    for (auto i = 0; i < REGEX_MATCH_VARIABLE_COUNT; i++) {
        variableRegistry->registerStored<QString>(QString("match_%1").arg(i));
    }
    variableRegistry->registerStored(BuiltinVariables::LastTriggerId);
    variableRegistry->registerStored(BuiltinVariables::LastTriggerTimestamp, true);
    variableRegistry->registerStored(BuiltinVariables::MaxFingerDistancePercentage);
    variableRegistry->registerComputed<PointF>("pointer_position_screen_percentage", [pointerPositionGetter](auto &value) {
        value = pointerPositionGetter->screenPointerPosition();
    });
    variableRegistry->registerComputed<PointF>("pointer_position_window_percentage", [pointerPositionGetter, windowProvider](auto &value) {
        const auto window = windowProvider->windowUnderPointer();
        if (!window) {
            return;
        }

        const auto windowGeometry = window->geometry();
        const auto pointerPos = pointerPositionGetter->globalPointerPosition();
        if (!pointerPos || !windowGeometry) {
            return;
        }
        const auto translatedPosition = pointerPos.value() - static_cast<PointF>(windowGeometry->topLeft());
        value = PointF(translatedPosition.x() / windowGeometry->width(), translatedPosition.y() / windowGeometry->height());
    });
    variableRegistry->registerStored(BuiltinVariables::PreviousWindowId);
    variableRegistry->registerStored(BuiltinVariables::ThumbInitialPositionPercentage);
    variableRegistry->registerStored(BuiltinVariables::ThumbPositionPercentage);
    variableRegistry->registerStored(BuiltinVariables::ThumbPresent);
    variableRegistry->registerComputed<qreal>("time_since_last_trigger", [variableRegistry](auto &value) {
        value = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()
              - variableRegistry->variable(BuiltinVariables::LastTriggerTimestamp)->value().value_or(0);
    });
    variableRegistry->registerComputed<QString>("window_class", [windowProvider](auto &value) {
        if (const auto window = windowProvider->activeWindow()) {
            value = window->resourceClass();
        }
    });
    variableRegistry->registerComputed<bool>("window_fullscreen", [windowProvider](auto &value) {
        if (const auto window = windowProvider->activeWindow()) {
            value = window->fullscreen();
        }
    });
    variableRegistry->registerComputed<QString>(BuiltinVariables::WindowId, [windowProvider](auto &value) {
        if (const auto window = windowProvider->activeWindow()) {
            value = window->id();
        }
    });
    variableRegistry->registerComputed<bool>("window_maximized", [windowProvider](auto &value) {
        if (const auto window = windowProvider->activeWindow()) {
            value = window->maximized();
        }
    });
    variableRegistry->registerComputed<QString>("window_name", [windowProvider](auto &value) {
        if (const auto window = windowProvider->activeWindow()) {
            value = window->resourceName();
        }
    });
    variableRegistry->registerComputed<qreal>("window_pid", [windowProvider](auto &value) {
        if (const auto window = windowProvider->activeWindow()) {
            value = window->pid();
        }
    });
    variableRegistry->registerComputed<QString>("window_title", [windowProvider](auto &value) {
        if (const auto window = windowProvider->activeWindow()) {
            value = window->title();
        }
    });
    variableRegistry->registerComputed<QString>("window_under_pointer_class", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderPointer()) {
            value = window->resourceClass();
        }
    });
    variableRegistry->registerAlias("window_under_class", "window_under_pointer_class");
    variableRegistry->registerComputed<bool>("window_under_pointer_fullscreen", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderPointer()) {
            value = window->fullscreen();
        }
    });
    variableRegistry->registerAlias("window_under_fullscreen", "window_under_pointer_fullscreen");
    variableRegistry->registerComputed<QString>(BuiltinVariables::WindowUnderPointerId, [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderPointer()) {
            value = window->id();
        }
    });
    variableRegistry->registerAlias("window_under_id", BuiltinVariables::WindowUnderPointerId);
    variableRegistry->registerComputed<bool>("window_under_pointer_maximized", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderPointer()) {
            value = window->maximized();
        }
    });
    variableRegistry->registerAlias("window_under_maximized", "window_under_pointer_maximized");
    variableRegistry->registerComputed<QString>("window_under_pointer_name", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderPointer()) {
            value = window->resourceName();
        }
    });
    variableRegistry->registerAlias("window_under_name", "window_under_pointer_name");
    variableRegistry->registerComputed<qreal>("window_under_pointer_pid", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderPointer()) {
            value = window->pid();
        }
    });
    variableRegistry->registerAlias("window_under_pid", "window_under_pointer_pid");
    variableRegistry->registerComputed<QString>("window_under_pointer_title", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderPointer()) {
            value = window->title();
        }
    });
    variableRegistry->registerAlias("window_under_title", "window_under_pointer_title");
    variableRegistry->registerComputed<QString>("window_under_fingers_class", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderFingers()) {
            value = window->resourceClass();
        }
    });
    variableRegistry->registerComputed<bool>("window_under_fingers_fullscreen", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderFingers()) {
            value = window->fullscreen();
        }
    });
    variableRegistry->registerComputed<QString>(BuiltinVariables::WindowUnderFingersId, [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderFingers()) {
            value = window->id();
        }
    });
    variableRegistry->registerComputed<bool>("window_under_fingers_maximized", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderFingers()) {
            value = window->maximized();
        }
    });
    variableRegistry->registerComputed<QString>("window_under_fingers_name", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderFingers()) {
            value = window->resourceName();
        }
    });
    variableRegistry->registerComputed<qreal>("window_under_fingers_pid", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderFingers()) {
            value = window->pid();
        }
    });
    variableRegistry->registerComputed<QString>("window_under_fingers_title", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderFingers()) {
            value = window->title();
        }
    });
}

}