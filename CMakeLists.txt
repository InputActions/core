cmake_minimum_required(VERSION 3.16.0)

project(inputactions)
set(PROJECT_VERSION "0.9.0")

set(QT_MIN_VERSION "6.6.0")
set(QT_MAJOR_VERSION 6)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

option(BUILD_TESTS "Build tests" OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Bsymbolic-functions -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-missing-field-initializers -Wno-switch -Wl,--no-undefined")

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    DBus
)

find_package(PkgConfig REQUIRED)
find_package(yaml-cpp REQUIRED)
pkg_search_module(LIBEVDEV REQUIRED libevdev)

add_subdirectory(src)
if (BUILD_TESTS)
    add_subdirectory(tests)
endif()
