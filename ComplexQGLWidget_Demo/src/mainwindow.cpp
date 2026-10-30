#include "mainwindow.h"

#include "solarsystemwidget.h"

#include <QtGui/QKeySequence>
#include <QtWidgets/QAction>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , glWidget_(new SolarSystemWidget(this))
{
    setCentralWidget(glWidget_);
    createControlDock();
    createShortcuts();

    setWindowTitle(QStringLiteral("Complex QGLWidget Demo - Solar System Lab"));
    resize(1280, 760);
    setMinimumSize(900, 600);

    statusBar()->showMessage(QStringLiteral("左键旋转 · 右键平移 · 中键/滚轮缩放 · 双击重置"));

    auto *fpsLabel = new QLabel(QStringLiteral("FPS --"), this);
    fpsLabel->setMinimumWidth(78);
    statusBar()->addPermanentWidget(fpsLabel);

    connect(glWidget_, &SolarSystemWidget::framesPerSecondChanged, fpsLabel,
            [fpsLabel](double fps) {
                fpsLabel->setText(QStringLiteral("FPS %1").arg(fps, 0, 'f', 1));
            });
    connect(glWidget_, &SolarSystemWidget::messageChanged, this,
            [this](const QString &message) { statusBar()->showMessage(message, 2200); });

    glWidget_->setFocus(Qt::OtherFocusReason);
}

void MainWindow::createControlDock()
{
    auto *dock = new QDockWidget(QStringLiteral("场景控制"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    dock->setMinimumWidth(252);

    auto *panel = new QWidget(dock);
    auto *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(14, 14, 14, 14);
    panelLayout->setSpacing(12);

    auto *title = new QLabel(QStringLiteral("太阳系实验室"), panel);
    QFont titleFont = title->font();
    titleFont.setPointSize(13);
    titleFont.setBold(true);
    title->setFont(titleFont);
    panelLayout->addWidget(title);

    auto *description = new QLabel(QStringLiteral("实时光照、粒子、轨道和多层球体渲染。"), panel);
    description->setWordWrap(true);
    description->setStyleSheet(QStringLiteral("color: #9fb1c8;"));
    panelLayout->addWidget(description);

    auto *simulationGroup = new QGroupBox(QStringLiteral("模拟"), panel);
    auto *simulationLayout = new QVBoxLayout(simulationGroup);
    auto *animationCheck = new QCheckBox(QStringLiteral("运行动画"), simulationGroup);
    animationCheck->setChecked(glWidget_->animationEnabled());
    simulationLayout->addWidget(animationCheck);

    auto *speedRow = new QHBoxLayout();
    auto *speedTitle = new QLabel(QStringLiteral("速度"), simulationGroup);
    auto *speedSlider = new QSlider(Qt::Horizontal, simulationGroup);
    auto *speedValue = new QLabel(QStringLiteral("1.00x"), simulationGroup);
    speedSlider->setRange(10, 300);
    speedSlider->setValue(glWidget_->simulationSpeedPercent());
    speedSlider->setSingleStep(10);
    speedValue->setMinimumWidth(48);
    speedRow->addWidget(speedTitle);
    speedRow->addWidget(speedSlider, 1);
    speedRow->addWidget(speedValue);
    simulationLayout->addLayout(speedRow);
    panelLayout->addWidget(simulationGroup);

    auto *renderGroup = new QGroupBox(QStringLiteral("渲染"), panel);
    auto *renderLayout = new QVBoxLayout(renderGroup);
    auto *lightingCheck = new QCheckBox(QStringLiteral("动态光照"), renderGroup);
    auto *wireframeCheck = new QCheckBox(QStringLiteral("线框模式"), renderGroup);
    auto *orbitsCheck = new QCheckBox(QStringLiteral("显示轨道"), renderGroup);
    auto *starsCheck = new QCheckBox(QStringLiteral("显示星空"), renderGroup);
    lightingCheck->setChecked(glWidget_->lightingEnabled());
    wireframeCheck->setChecked(glWidget_->wireframeEnabled());
    orbitsCheck->setChecked(glWidget_->orbitsVisible());
    starsCheck->setChecked(glWidget_->starsVisible());
    renderLayout->addWidget(lightingCheck);
    renderLayout->addWidget(wireframeCheck);
    renderLayout->addWidget(orbitsCheck);
    renderLayout->addWidget(starsCheck);
    panelLayout->addWidget(renderGroup);

    auto *resetButton = new QPushButton(QStringLiteral("重置相机"), panel);
    resetButton->setMinimumHeight(34);
    panelLayout->addWidget(resetButton);

    auto *help = new QLabel(QStringLiteral("快捷键\n"
                                           "Space  暂停/继续\n"
                                           "L  光照 · W  线框\n"
                                           "O  轨道 · S  星空\n"
                                           "+/-  调整速度 · R  重置"),
                            panel);
    help->setStyleSheet(
        QStringLiteral("QLabel { color: #9fb1c8; background: #111827; "
                       "border: 1px solid #2f4058; border-radius: 6px; padding: 9px; }"));
    panelLayout->addWidget(help);
    panelLayout->addStretch(1);

    connect(animationCheck, &QCheckBox::toggled, glWidget_,
            &SolarSystemWidget::setAnimationEnabled);
    connect(lightingCheck, &QCheckBox::toggled, glWidget_, &SolarSystemWidget::setLightingEnabled);
    connect(wireframeCheck, &QCheckBox::toggled, glWidget_,
            &SolarSystemWidget::setWireframeEnabled);
    connect(orbitsCheck, &QCheckBox::toggled, glWidget_, &SolarSystemWidget::setOrbitsVisible);
    connect(starsCheck, &QCheckBox::toggled, glWidget_, &SolarSystemWidget::setStarsVisible);
    connect(speedSlider, &QSlider::valueChanged, glWidget_, &SolarSystemWidget::setSimulationSpeed);
    connect(resetButton, &QPushButton::clicked, glWidget_, &SolarSystemWidget::resetView);

    // 键盘切换时同步更新控制面板。
    connect(glWidget_, &SolarSystemWidget::animationEnabledChanged, animationCheck,
            &QCheckBox::setChecked);
    connect(glWidget_, &SolarSystemWidget::lightingEnabledChanged, lightingCheck,
            &QCheckBox::setChecked);
    connect(glWidget_, &SolarSystemWidget::wireframeEnabledChanged, wireframeCheck,
            &QCheckBox::setChecked);
    connect(glWidget_, &SolarSystemWidget::orbitsVisibleChanged, orbitsCheck,
            &QCheckBox::setChecked);
    connect(glWidget_, &SolarSystemWidget::starsVisibleChanged, starsCheck, &QCheckBox::setChecked);
    connect(glWidget_, &SolarSystemWidget::simulationSpeedChanged, speedSlider, &QSlider::setValue);
    connect(speedSlider, &QSlider::valueChanged, speedValue, [speedValue](int percent) {
        speedValue->setText(QStringLiteral("%1x").arg(percent / 100.0, 0, 'f', 2));
    });

    dock->setWidget(panel);
    addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::createShortcuts()
{
    auto createAction = [this](const QKeySequence &shortcut) {
        auto *action = new QAction(this);
        action->setShortcut(shortcut);
        action->setShortcutContext(Qt::ApplicationShortcut);
        addAction(action);
        return action;
    };

    QAction *animationAction = createAction(QKeySequence(Qt::Key_Space));
    QAction *lightingAction = createAction(QKeySequence(QStringLiteral("L")));
    QAction *wireframeAction = createAction(QKeySequence(QStringLiteral("W")));
    QAction *orbitsAction = createAction(QKeySequence(QStringLiteral("O")));
    QAction *starsAction = createAction(QKeySequence(QStringLiteral("S")));
    QAction *resetAction = createAction(QKeySequence(QStringLiteral("R")));
    QAction *fasterAction = createAction(QKeySequence(QStringLiteral("+")));
    QAction *slowerAction = createAction(QKeySequence(QStringLiteral("-")));
    QAction *closeAction = createAction(QKeySequence(Qt::Key_Escape));

    connect(animationAction, &QAction::triggered, this,
            [this]() { glWidget_->setAnimationEnabled(!glWidget_->animationEnabled()); });
    connect(lightingAction, &QAction::triggered, this,
            [this]() { glWidget_->setLightingEnabled(!glWidget_->lightingEnabled()); });
    connect(wireframeAction, &QAction::triggered, this,
            [this]() { glWidget_->setWireframeEnabled(!glWidget_->wireframeEnabled()); });
    connect(orbitsAction, &QAction::triggered, this,
            [this]() { glWidget_->setOrbitsVisible(!glWidget_->orbitsVisible()); });
    connect(starsAction, &QAction::triggered, this,
            [this]() { glWidget_->setStarsVisible(!glWidget_->starsVisible()); });
    connect(resetAction, &QAction::triggered, glWidget_, &SolarSystemWidget::resetView);
    connect(fasterAction, &QAction::triggered, this,
            [this]() { glWidget_->setSimulationSpeed(glWidget_->simulationSpeedPercent() + 10); });
    connect(slowerAction, &QAction::triggered, this,
            [this]() { glWidget_->setSimulationSpeed(glWidget_->simulationSpeedPercent() - 10); });
    connect(closeAction, &QAction::triggered, this, &QWidget::close);
}
