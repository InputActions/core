#include "Test.h"
#include <libinputactions/InputActionsMain.h>
#include <libinputactions/config/ConfigProvider.h>
#include <libinputactions/config/GlobalConfig.h>
#include <libinputactions/scripting/ScriptingEngine.h>

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
    inputActions->setInTestEnvironment(true);
    inputActions->setMissingImplementations();
    inputActions->initialize();
}

}