#include "Test.h"
#include <libinputactions/InputActionsMain.h>
#include <libinputactions/config/GlobalConfig.h>
#include <libinputactions/interfaces/ConfigProvider.h>

namespace InputActions
{

Test::~Test()
{
    delete g_inputActions;
}

void Test::initMain()
{
    int argc = 0;
    QCoreApplication app(argc, nullptr);

    auto *inputActions = new InputActionsMain;
    g_configProvider = std::make_shared<ConfigProvider>(); // don't watch config
    inputActions->setMissingImplementations();
    inputActions->initialize();
    g_globalConfig->setSendNotificationOnError(false);
}

}