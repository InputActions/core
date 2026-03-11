#include "Test.h"
#include <libinputactions/Value.h>
#include <libinputactions/variables/VariableManager.h>

namespace InputActions
{

class TestValue : public Test
{
    Q_OBJECT

private slots:
    void init() { g_variableManager = std::make_shared<VariableManager>(); }

    void get_defaultConstructor_returnsNullopt()
    {
        const Value<bool> value;
        QVERIFY(!value.get().has_value());
    }

    void get_valueConstructor()
    {
        const Value<bool> value(true);
        QVERIFY(value.get().value());
    }

    void get_command()
    {
        const auto value = Value<QString>::command(Value<QString>("echo -n a"));
        QCOMPARE(value.get().value(), "a");
    }

    void get_commandNullValue_returnsNullopt()
    {
        const auto value = Value<QString>::command({});
        QVERIFY(!value.get().has_value());
    }

    void get_function()
    {
        const auto value = Value<bool>::function([]() {
            return true;
        });
        QVERIFY(value.get().value());
    }

    void get_existentVariable()
    {
        g_variableManager->registerRemoteVariable<bool>("_test", [](auto &value) {
            value = true;
        });
        const auto value = Value<bool>::variable("_test");
        QVERIFY(value.get().value());
    }

    void get_nonExistentVariable_returnsNullopt()
    {
        const auto value = Value<bool>::variable("_test");
        QVERIFY(!value.get().has_value());
    }
};

}

QTEST_MAIN(InputActions::TestValue)
#include "TestValue.moc"