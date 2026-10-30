cmake_minimum_required(VERSION 3.25)

project(
    QGLWidgetDemo
    VERSION 1.0.0
    DESCRIPTION "A classic QGLWidget demo built with Qt 5 and MSVC 2026"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 启用 Qt 的 MOC、资源和 UI 自动处理。
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 5.15 REQUIRED COMPONENTS Widgets OpenGL)
find_package(OpenGL REQUIRED)

add_executable(QGLWidgetDemo WIN32)

target_sources(
    QGLWidgetDemo
    PRIVATE
        src/main.cpp
        src/glwidget.cpp
        src/glwidget.h
        src/msvc2026_qt5_compat.h
)

target_link_libraries(
    QGLWidgetDemo
    PRIVATE
        Qt5::Widgets
        Qt5::OpenGL
        OpenGL::GL
)

target_compile_definitions(
    QGLWidgetDemo
    PRIVATE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
)

if(MSVC)
    target_compile_options(
        QGLWidgetDemo
        PRIVATE
            /W4
            /permissive-
            /Zc:__cplusplus
            /utf-8
            /FI"${CMAKE_CURRENT_SOURCE_DIR}/src/msvc2026_qt5_compat.h"
    )
endif()

set_target_properties(
    QGLWidgetDemo
    PROPERTIES
        OUTPUT_NAME "QGLWidgetDemo"
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:QGLWidgetDemo>"
)

set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT QGLWidgetDemo)

# 构建后自动部署 Qt DLL 和平台插件，使程序可直接运行。
if(WIN32)
    get_target_property(QT_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIRECTORY "${QT_QMAKE_EXECUTABLE}" DIRECTORY)

    find_program(
        WINDEPLOYQT_EXECUTABLE
        NAMES windeployqt
        HINTS "${QT_BIN_DIRECTORY}"
        NO_DEFAULT_PATH
    )

    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(
            TARGET QGLWidgetDemo
            POST_BUILD
            COMMAND
                "${WINDEPLOYQT_EXECUTABLE}"
                --no-translations
                --no-compiler-runtime
                --dir "$<TARGET_FILE_DIR:QGLWidgetDemo>"
                "$<TARGET_FILE:QGLWidgetDemo>"
            COMMENT "Deploying the Qt 5 runtime"
            VERBATIM
        )
    else()
        message(WARNING "windeployqt was not found; the executable may need Qt on PATH.")
    endif()
endif()

