cmake_minimum_required(VERSION 3.5)

project(cuteFPGA VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

get_target_property(target_type Qt6::Core TYPE)
if (target_type STREQUAL STATIC_LIBRARY)
    set(CMAKE_EXE_LINKER_FLAGS "-static")
endif()

set(PROJECT_SOURCES
        main.cpp
        utils.h utils.cpp
        installer.h installer.cpp
        downloader.h downloader.cpp
        mainwindow.h mainwindow.cpp mainwindow.ui
        synthesis.h synthesis.cpp synthesis.ui
        fancybutton.h fancybutton.cpp fancybutton.ui
        programmer.h programmer.cpp programmer.ui
        settingsdialog.h settingsdialog.cpp settingsdialog.ui
        spinnertask.h spinnertask.cpp spinnertask.ui
        spinnerdialog.h spinnerdialog.cpp spinnerdialog.ui
        warndialog.h warndialog.cpp warndialog.ui

        data.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(cuteFPGA
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET cuteFPGA APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(cuteFPGA SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(cuteFPGA
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(cuteFPGA PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)

set_target_properties(cuteFPGA PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(cuteFPGA)
endif()
