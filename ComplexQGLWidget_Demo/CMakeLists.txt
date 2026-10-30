cmake_minimum_required(VERSION 3.25)

project(
    ComplexQGLWidgetDemo
    VERSION 1.0.0
    DESCRIPTION "Interactive solar-system demo based on Qt 5 QGLWidget"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 自动处理 Qt 的元对象、资源和 UI 文件。
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 5.15 REQUIRED COMPONENTS Widgets OpenGL)
find_package(OpenGL REQUIRED)

add_executable(ComplexQGLWidgetDemo WIN32)

target_sources(
    ComplexQGLWidgetDemo
    PRIVATE
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/solarsystemwidget.cpp
        src/solarsystemwidget.h
        src/msvc2026_qt5_compat.h
)

target_link_libraries(
    ComplexQGLWidgetDemo
    PRIVATE
        Qt5::Widgets
        Qt5::OpenGL
        OpenGL::GL
)

target_compile_definitions(
    ComplexQGLWidgetDemo
    PRIVATE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
)

if(MSVC)
    target_compile_options(
        ComplexQGLWidgetDemo
        PRIVATE
            /W4
            /permissive-
            /Zc:__cplusplus
            /utf-8
            /FI"${CMAKE_CURRENT_SOURCE_DIR}/src/msvc2026_qt5_compat.h"
    )
endif()

set_target_properties(
    ComplexQGLWidgetDemo
    PROPERTIES
        OUTPUT_NAME "ComplexQGLWidgetDemo"
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:ComplexQGLWidgetDemo>"
)

set_property(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    PROPERTY VS_STARTUP_PROJECT ComplexQGLWidgetDemo
)

# 构建后自动部署 Qt 运行库和平台插件。
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
            TARGET ComplexQGLWidgetDemo
            POST_BUILD
            COMMAND
                "${WINDEPLOYQT_EXECUTABLE}"
                --no-translations
                --no-compiler-runtime
                --dir "$<TARGET_FILE_DIR:ComplexQGLWidgetDemo>"
                "$<TARGET_FILE:ComplexQGLWidgetDemo>"
            COMMENT "Deploying the Qt 5 runtime"
            VERBATIM
        )
    else()
        message(WARNING "windeployqt was not found; Qt must be available on PATH.")
    endif()
endif()

