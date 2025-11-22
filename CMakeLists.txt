cmake_minimum_required(VERSION 3.31)
project(clion_test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "D:/Qt/6.9.3/mingw_64")

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

# 设置源文件目录
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# 收集所有源文件
file(GLOB_RECURSE SOURCES
        ${SRC_DIR}/*.cpp
        ${SRC_DIR}/*.h
)

# Windows平台特定资源文件
if(WIN32)
    # 添加Windows资源文件用于应用图标
    set(RESOURCE_FILE ${SRC_DIR}/resource/app_icon.rc)
endif()

# 创建可执行文件
add_executable(clion_test ${SOURCES}
        src/widget/ui_mainwindow.cpp
        src/widget/bookdetaildialog.cpp  # 确保这行存在
        src/widget/bookdetaildialog.h    # 确保这行存在
        src/widget/borrowdialog.cpp      # 新增借书对话框
        src/widget/borrowdialog.h        # 新增借书对话框头文件
        ${RESOURCE_FILE}                 # Windows资源文件
)

# 链接Qt库
target_link_libraries(clion_test
        Qt::Core
        Qt::Gui
        Qt::Widgets
)

# Windows平台特定设置
if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()

    # 复制Qt DLL到输出目录
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()

    foreach (QT_LIB Core Gui Widgets)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()

# 设置包含目录
target_include_directories(clion_test PRIVATE ${SRC_DIR})

# 编译器特定设置
if (MSVC)
    target_compile_options(clion_test PRIVATE /W4)
else()
    target_compile_options(clion_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

