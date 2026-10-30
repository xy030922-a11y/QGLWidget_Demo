#pragma once

#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtCore/QTimer>
#include <QtOpenGL/QGLWidget>

class QKeyEvent;
class QMouseEvent;
class QWheelEvent;

// 使用经典 QGLWidget 绘制可交互的旋转立方体。
class GLWidget final : public QGLWidget
{
public:
    explicit GLWidget(QWidget *parent = nullptr);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    // QGLWidget 的初始化、缩放和绘制回调。
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;

    // 键盘、鼠标旋转和滚轮缩放。
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    // 固定管线需要 OpenGL 2.1 兼容模式。
    static QGLFormat requestedFormat();

    void drawCube() const;
    void drawGrid() const;
    void resetView();
    void toggleAnimation();

    QTimer animationTimer_;
    QPoint lastMousePosition_;

    // 当前视角和相机距离。
    float xRotation_ = 24.0F;
    float yRotation_ = -32.0F;
    float zRotation_ = 0.0F;
    float cameraDistance_ = -7.0F;
    bool animationEnabled_ = true;
};
