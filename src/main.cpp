#include "glwidget.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QStatusBar>

int main(int argc, char *argv[])
{
    // 必须在创建 QApplication 前设置渲染属性。
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    QApplication application(argc, argv);
    application.setApplicationName(QStringLiteral("QGLWidgetDemo"));
    application.setOrganizationName(QStringLiteral("QGLWidgetDemo"));

    // 创建主窗口，并将 OpenGL 控件设为中心区域。
    QMainWindow window;
    auto *glWidget = new GLWidget(&window);
    window.setCentralWidget(glWidget);
    window.setWindowTitle(QStringLiteral("QGLWidget Demo - Qt 5 / MSVC 2026"));
    window.statusBar()->showMessage(
        QStringLiteral("左键/右键拖动旋转 · 滚轮缩放 · Space 暂停 · R 重置 · Esc 退出"));
    window.resize(glWidget->sizeHint());
    window.show();

    return QApplication::exec();
}
