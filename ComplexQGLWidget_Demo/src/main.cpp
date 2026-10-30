#include "mainwindow.h"

#include <QtCore/QCoreApplication>
#include <QtGui/QColor>
#include <QtGui/QPalette>
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleFactory>

int main(int argc, char *argv[])
{
    // OpenGL 和高分屏属性必须在 QApplication 前设置。
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    QApplication application(argc, argv);
    application.setApplicationName(QStringLiteral("ComplexQGLWidgetDemo"));
    application.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));

    // 深色界面与太空场景保持一致。
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(25, 30, 42));
    palette.setColor(QPalette::WindowText, QColor(226, 232, 240));
    palette.setColor(QPalette::Base, QColor(16, 21, 32));
    palette.setColor(QPalette::AlternateBase, QColor(32, 39, 54));
    palette.setColor(QPalette::Text, QColor(226, 232, 240));
    palette.setColor(QPalette::Button, QColor(40, 48, 65));
    palette.setColor(QPalette::ButtonText, QColor(235, 240, 248));
    palette.setColor(QPalette::Highlight, QColor(42, 144, 255));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    application.setPalette(palette);

    MainWindow window;
    window.show();

    return QApplication::exec();
}
