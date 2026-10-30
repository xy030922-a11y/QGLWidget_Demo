#include "glwidget.h"

#include <QtGui/QColor>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QWidget>

#include <algorithm>
#include <cmath>

namespace
{
// 动画刷新和透视投影参数。
    constexpr int kFrameIntervalMs = 16;
    constexpr float kFullTurnDegrees = 360.0F;
    constexpr float kNearPlane = 1.0F;
    constexpr float kFarPlane = 40.0F;
    constexpr float kFieldOfViewDegrees = 45.0F;
    constexpr double kPi = 3.14159265358979323846;
}

QGLFormat GLWidget::requestedFormat()
{
    // 请求支持固定管线的 OpenGL 2.1 上下文。
    QGLFormat format = QGLFormat::defaultFormat();
    format.setVersion(2, 1);
    format.setProfile(QGLFormat::CompatibilityProfile);
    format.setDepth(true);
    format.setDoubleBuffer(true);
    format.setSampleBuffers(true);
    format.setSamples(4);
    return format;
}

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(requestedFormat(), parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    // 约 60 FPS 自动旋转。
    animationTimer_.setTimerType(Qt::PreciseTimer);
    connect(&animationTimer_, &QTimer::timeout, this, [this]()
            {
        yRotation_ = std::fmod(yRotation_ + 0.6F, kFullTurnDegrees);
        updateGL(); });
    animationTimer_.start(kFrameIntervalMs);
}

QSize GLWidget::minimumSizeHint() const
{
    return {320, 240};
}

QSize GLWidget::sizeHint() const
{
    return {960, 640};
}

void GLWidget::initializeGL()
{
    // 设置背景色、深度测试和平滑着色。
    qglClearColor(QColor(10, 16, 30));

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void GLWidget::resizeGL(int width, int height)
{
    // 根据窗口比例更新透视投影，避免画面拉伸。
    const int safeHeight = std::max(height, 1);
    const double aspectRatio =
        static_cast<double>(width) / static_cast<double>(safeHeight);
    const double halfHeight =
        static_cast<double>(kNearPlane) * std::tan(kFieldOfViewDegrees * kPi / 360.0);
    const double halfWidth = halfHeight * aspectRatio;

    glViewport(0, 0, width, safeHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(
        -halfWidth,
        halfWidth,
        -halfHeight,
        halfHeight,
        static_cast<double>(kNearPlane),
        static_cast<double>(kFarPlane));
    glMatrixMode(GL_MODELVIEW);
}

void GLWidget::paintGL()
{
    // 清屏后应用相机平移和三轴旋转。
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0F, 0.0F, cameraDistance_);
    glRotatef(xRotation_, 1.0F, 0.0F, 0.0F);
    glRotatef(yRotation_, 0.0F, 1.0F, 0.0F);
    glRotatef(zRotation_, 0.0F, 0.0F, 1.0F);

    drawGrid();
    drawCube();
}

void GLWidget::drawGrid() const
{
    // 绘制地面参考网格，中心线使用高亮色。
    glLineWidth(1.0F);
    glBegin(GL_LINES);

    for (int line = -4; line <= 4; ++line)
    {
        const float position = static_cast<float>(line);
        const bool isAxis = line == 0;
        if (isAxis)
        {
            glColor3f(0.25F, 0.55F, 0.80F);
        }
        else
        {
            glColor3f(0.16F, 0.22F, 0.32F);
        }

        glVertex3f(position, -1.45F, -4.0F);
        glVertex3f(position, -1.45F, 4.0F);
        glVertex3f(-4.0F, -1.45F, position);
        glVertex3f(4.0F, -1.45F, position);
    }

    glEnd();
}

void GLWidget::drawCube() const
{
    glBegin(GL_QUADS);

    // 前面：红橙色。
    glColor3f(1.00F, 0.25F, 0.20F);
    glVertex3f(-1.0F, -1.0F, 1.0F);
    glColor3f(1.00F, 0.55F, 0.15F);
    glVertex3f(1.0F, -1.0F, 1.0F);
    glColor3f(1.00F, 0.75F, 0.20F);
    glVertex3f(1.0F, 1.0F, 1.0F);
    glColor3f(1.00F, 0.35F, 0.35F);
    glVertex3f(-1.0F, 1.0F, 1.0F);

    // 后面：紫色。
    glColor3f(0.45F, 0.20F, 0.85F);
    glVertex3f(1.0F, -1.0F, -1.0F);
    glColor3f(0.28F, 0.20F, 0.65F);
    glVertex3f(-1.0F, -1.0F, -1.0F);
    glColor3f(0.55F, 0.35F, 0.95F);
    glVertex3f(-1.0F, 1.0F, -1.0F);
    glColor3f(0.65F, 0.30F, 0.90F);
    glVertex3f(1.0F, 1.0F, -1.0F);

    // 顶面：绿色。
    glColor3f(0.20F, 0.85F, 0.45F);
    glVertex3f(-1.0F, 1.0F, 1.0F);
    glColor3f(0.35F, 1.00F, 0.62F);
    glVertex3f(1.0F, 1.0F, 1.0F);
    glColor3f(0.12F, 0.68F, 0.45F);
    glVertex3f(1.0F, 1.0F, -1.0F);
    glColor3f(0.15F, 0.60F, 0.35F);
    glVertex3f(-1.0F, 1.0F, -1.0F);

    // 底面：深蓝色。
    glColor3f(0.08F, 0.20F, 0.48F);
    glVertex3f(-1.0F, -1.0F, -1.0F);
    glColor3f(0.12F, 0.30F, 0.68F);
    glVertex3f(1.0F, -1.0F, -1.0F);
    glColor3f(0.08F, 0.35F, 0.75F);
    glVertex3f(1.0F, -1.0F, 1.0F);
    glColor3f(0.10F, 0.25F, 0.58F);
    glVertex3f(-1.0F, -1.0F, 1.0F);

    // 右面：青色。
    glColor3f(0.12F, 0.72F, 0.82F);
    glVertex3f(1.0F, -1.0F, 1.0F);
    glColor3f(0.16F, 0.62F, 0.90F);
    glVertex3f(1.0F, -1.0F, -1.0F);
    glColor3f(0.28F, 0.82F, 1.00F);
    glVertex3f(1.0F, 1.0F, -1.0F);
    glColor3f(0.22F, 0.92F, 0.92F);
    glVertex3f(1.0F, 1.0F, 1.0F);

    // 左面：金色。
    glColor3f(0.88F, 0.58F, 0.12F);
    glVertex3f(-1.0F, -1.0F, -1.0F);
    glColor3f(0.95F, 0.70F, 0.15F);
    glVertex3f(-1.0F, -1.0F, 1.0F);
    glColor3f(1.00F, 0.82F, 0.25F);
    glVertex3f(-1.0F, 1.0F, 1.0F);
    glColor3f(0.82F, 0.50F, 0.10F);
    glVertex3f(-1.0F, 1.0F, -1.0F);

    glEnd();

    // 用浅色线条勾勒立方体边缘。
    glColor3f(0.92F, 0.96F, 1.0F);
    glLineWidth(1.5F);
    glBegin(GL_LINES);
    constexpr float vertices[8][3] = {
        {-1.0F, -1.0F, -1.0F},
        {1.0F, -1.0F, -1.0F},
        {1.0F, 1.0F, -1.0F},
        {-1.0F, 1.0F, -1.0F},
        {-1.0F, -1.0F, 1.0F},
        {1.0F, -1.0F, 1.0F},
        {1.0F, 1.0F, 1.0F},
        {-1.0F, 1.0F, 1.0F},
    };
    constexpr int edges[12][2] = {
        {0, 1},
        {1, 2},
        {2, 3},
        {3, 0},
        {4, 5},
        {5, 6},
        {6, 7},
        {7, 4},
        {0, 4},
        {1, 5},
        {2, 6},
        {3, 7},
    };
    for (const auto &edge : edges)
    {
        glVertex3fv(vertices[edge[0]]);
        glVertex3fv(vertices[edge[1]]);
    }
    glEnd();
}

void GLWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Space:
        toggleAnimation();
        event->accept();
        break;
    case Qt::Key_R:
        resetView();
        event->accept();
        break;
    case Qt::Key_Escape:
        window()->close();
        event->accept();
        break;
    default:
        QGLWidget::keyPressEvent(event);
        break;
    }
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    lastMousePosition_ = event->pos();
    event->accept();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    const QPoint delta = event->pos() - lastMousePosition_;
    lastMousePosition_ = event->pos();

    if ((event->buttons() & Qt::LeftButton) != 0)
    {
        xRotation_ += static_cast<float>(delta.y()) * 0.5F;
        yRotation_ += static_cast<float>(delta.x()) * 0.5F;
        updateGL();
    }
    else if ((event->buttons() & Qt::RightButton) != 0)
    {
        xRotation_ += static_cast<float>(delta.y()) * 0.5F;
        zRotation_ += static_cast<float>(delta.x()) * 0.5F;
        updateGL();
    }

    event->accept();
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    const float steps = static_cast<float>(event->angleDelta().y()) / 120.0F;
    cameraDistance_ = std::clamp(cameraDistance_ + steps * 0.4F, -12.0F, -3.2F);
    updateGL();
    event->accept();
}

void GLWidget::resetView()
{
    xRotation_ = 24.0F;
    yRotation_ = -32.0F;
    zRotation_ = 0.0F;
    cameraDistance_ = -7.0F;
    updateGL();
}

void GLWidget::toggleAnimation()
{
    animationEnabled_ = !animationEnabled_;
    if (animationEnabled_)
    {
        animationTimer_.start(kFrameIntervalMs);
    }
    else
    {
        animationTimer_.stop();
    }
    updateGL();
}
