# 核心模块（Qt 5/6通用）
QT += core gui widgets sql

# C++标准（Qt 5/6均支持C++17）
CONFIG += c++17
DEFINES += QT_DEPRECATED_WARNINGS

# 目标程序名称
TARGET = TaskManager
# 生成可执行文件
TEMPLATE = app

# 源文件列表（所有.cpp文件）
SOURCES += main.cpp \
           mainwindow.cpp \
           addtaskdialog.cpp \
           categorydialog.cpp \
           sqlrepository.cpp \
           taskmanager.cpp \
           reminderthread.cpp \
           fileexporter.cpp \
           taskmodel.cpp

# 头文件列表（所有.h文件）
HEADERS += mainwindow.h \
           addtaskdialog.h \
           categorydialog.h \
           sqlrepository.h \
           taskmanager.h \
           reminderthread.h \
           fileexporter.h \
           taskmodel.h

# UI文件列表（.ui文件）
FORMS += mainwindow.ui \
         addtaskdialog.ui

# 输出目录配置（可选，整理编译文件）
DESTDIR = ./bin
OBJECTS_DIR = ./build/obj
MOC_DIR = ./build/moc
RCC_DIR = ./build/rcc
UI_DIR = ./build/ui

# Qt 6兼容配置（自动适配Qt版本）
greaterThan(QT_MAJOR_VERSION, 5) {
    QT += core5compat  # Qt 6需要这个模块兼容Qt 5的部分API
    DEFINES += QT6_COMPAT
}
