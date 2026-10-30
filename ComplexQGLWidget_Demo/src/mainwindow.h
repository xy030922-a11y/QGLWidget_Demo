#pragma once

#include <QtWidgets/QMainWindow>

class SolarSystemWidget;

// 组合 OpenGL 视口、控制面板和状态栏。
class MainWindow final : public QMainWindow
{
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void createControlDock();
    void createShortcuts();

    SolarSystemWidget *glWidget_ = nullptr;
};
