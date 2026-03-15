include_directories(.)

add_library(inputactions-test STATIC
    Test.cpp
    utils.cpp
)
target_link_libraries(inputactions-test PUBLIC
    libinputactions
    Qt::Test
)
target_precompile_headers(inputactions-test
    PUBLIC
        <QSignalSpy>
        <QTest>
        Test.h
        utils.h
)

macro(libinputactions_add_test TARGET_NAME)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    qt_add_executable(${TARGET_NAME} ${TEST_SOURCES})
    add_test(NAME "${TARGET_NAME}" COMMAND ${TARGET_NAME})
    target_link_libraries(${TARGET_NAME} PRIVATE
        inputactions-test
        GTest::gmock_main
    )
    target_precompile_headers(${TARGET_NAME} REUSE_FROM inputactions-test)
endmacro()

libinputactions_add_test(TestAction SOURCES actions/TestAction.cpp)
libinputactions_add_test(TestActionExecutor SOURCES actions/TestActionExecutor.cpp)
libinputactions_add_test(TestActionGroup SOURCES actions/TestActionGroup.cpp)
libinputactions_add_test(TestActionGroupNodeParser SOURCES config/parsers/actions/TestActionGroupNodeParser.cpp)
libinputactions_add_test(TestActionInterval SOURCES actions/TestActionInterval.cpp)
libinputactions_add_test(TestBaseNodeParser SOURCES config/parsers/TestBaseNodeParser.cpp)
libinputactions_add_test(TestCanReplaceTextConditionNodeParser SOURCES config/parsers/conditions/TestCanReplaceTextConditionNodeParser.cpp)
libinputactions_add_test(TestConditionGroup SOURCES conditions/TestConditionGroup.cpp)
libinputactions_add_test(TestConditionGroupNodeParser SOURCES config/parsers/conditions/TestConditionGroupNodeParser.cpp)
libinputactions_add_test(TestConditionNodeParser SOURCES config/parsers/conditions/TestConditionNodeParser.cpp)
libinputactions_add_test(TestDeviceRuleNodeParser SOURCES config/parsers/TestDeviceRuleNodeParser.cpp)
libinputactions_add_test(TestDirectionalMotionTriggerCore SOURCES triggers/core/TestDirectionalMotionTriggerCore.cpp)
libinputactions_add_test(TestEnumNodeParser SOURCES config/parsers/TestEnumNodeParser.cpp)
libinputactions_add_test(TestFlagsNodeParser SOURCES config/parsers/TestFlagsNodeParser.cpp)
libinputactions_add_test(TestInputActionNodeParser SOURCES config/parsers/actions/TestInputActionNodeParser.cpp)
libinputactions_add_test(TestInputTriggerHandler SOURCES handlers/TestInputTriggerHandler.cpp)
libinputactions_add_test(TestKeyboardTriggerHandler SOURCES handlers/TestKeyboardTriggerHandler.cpp)
libinputactions_add_test(TestKeyboardTriggerNodeParser SOURCES config/parsers/triggers/TestKeyboardTriggerNodeParser.cpp)
libinputactions_add_test(TestMotionTriggerCore SOURCES triggers/core/TestMotionTriggerCore.cpp)
libinputactions_add_test(TestMotionTriggerHandler SOURCES handlers/TestMotionTriggerHandler.cpp)
libinputactions_add_test(TestMouseTrigger SOURCES triggers/TestMouseTrigger.cpp)
libinputactions_add_test(TestMouseTriggerNodeParser SOURCES config/parsers/triggers/TestMouseTriggerNodeParser.cpp)
libinputactions_add_test(TestPlasmaShortcutActionNodeParser SOURCES config/parsers/actions/TestPlasmaShortcutActionNodeParser.cpp)
libinputactions_add_test(TestPointerTriggerNodeParser SOURCES config/parsers/triggers/TestPointerTriggerNodeParser.cpp)
libinputactions_add_test(TestQPointFNodeParser SOURCES config/parsers/qt/TestQPointFNodeParser.cpp)
libinputactions_add_test(TestQRegularExpressionNodeParser SOURCES config/parsers/qt/TestQRegularExpressionNodeParser.cpp)
libinputactions_add_test(TestQStringNodeParser SOURCES config/parsers/qt/TestQStringNodeParser.cpp)
libinputactions_add_test(TestQStringListNodeParser SOURCES config/parsers/qt/TestQStringListNodeParser.cpp)
libinputactions_add_test(TestRange SOURCES TestRange.cpp)
libinputactions_add_test(TestRangeNodeParser SOURCES config/parsers/TestRangeNodeParser.cpp)
libinputactions_add_test(TestReplaceTextActionNodeParser SOURCES config/parsers/actions/TestReplaceTextActionNodeParser.cpp)
libinputactions_add_test(TestSeparatedStringNodeParser SOURCES config/parsers/TestSeparatedStringNodeParser.cpp)
libinputactions_add_test(TestSetNodeParser SOURCES config/parsers/containers/TestSetNodeParser.cpp)
libinputactions_add_test(TestStrokeTriggerCoreNodeParser SOURCES config/parsers/triggers/TestStrokeTriggerCoreNodeParser.cpp)
libinputactions_add_test(TestSwipeTriggerCoreNodeParser SOURCES config/parsers/triggers/TestSwipeTriggerCoreNodeParser.cpp)
libinputactions_add_test(TestSwipeTriggerCore SOURCES triggers/core/TestSwipeTriggerCore.cpp)
libinputactions_add_test(TestTouchpadTriggerHandler SOURCES handlers/TestTouchpadTriggerHandler.cpp)
libinputactions_add_test(TestTouchpadTriggerNodeParser SOURCES config/parsers/triggers/TestTouchpadTriggerNodeParser.cpp)
libinputactions_add_test(TestTouchscreenTriggerNodeParser SOURCES config/parsers/triggers/TestTouchscreenTriggerNodeParser.cpp)
libinputactions_add_test(TestTrigger SOURCES triggers/TestTrigger.cpp)
libinputactions_add_test(TestTriggerAction SOURCES actions/TestTriggerAction.cpp)
libinputactions_add_test(TestTriggerActionNodeParser SOURCES config/parsers/TestTriggerActionNodeParser.cpp)
libinputactions_add_test(TestTriggerGroupNodeParser SOURCES config/parsers/TestTriggerGroupNodeParser.cpp)
libinputactions_add_test(TestTriggerHandler SOURCES handlers/TestTriggerHandler.cpp)
libinputactions_add_test(TestTriggerNodeParser SOURCES config/parsers/triggers/TestTriggerNodeParser.cpp)
libinputactions_add_test(TestValue SOURCES TestValue.cpp)
libinputactions_add_test(TestVariableConditionNodeParser SOURCES config/parsers/conditions/TestVariableConditionNodeParser.cpp)
libinputactions_add_test(TestVectorNodeParser SOURCES config/parsers/containers/TestVectorNodeParser.cpp)