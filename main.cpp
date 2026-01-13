#include <QApplication>
#include <QStyleFactory>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置应用程序属性（兼容Qt 5/6）
    QApplication::setStyle(QStyleFactory::create("Fusion")); // 统一风格
    QApplication::setApplicationName("TaskManager");
    QApplication::setOrganizationName("QtCourseDesign");

    // 启动主窗口
    MainWindow w;
    w.show();

    return a.exec();
}
