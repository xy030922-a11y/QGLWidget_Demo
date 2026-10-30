#pragma once

#include <QtCore/QElapsedTimer>
#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtGui/QColor>
#include <QtGui/QVector3D>
#include <QtOpenGL/QGLWidget>

#include <vector>

class QMouseEvent;
class QPainter;
class QWheelEvent;

// 负责三维场景绘制、动画和相机交互。
class SolarSystemWidget final : public QGLWidget
{
    Q_OBJECT

public:
    explicit SolarSystemWidget(QWidget *parent = nullptr);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    bool animationEnabled() const noexcept;
    bool lightingEnabled() const noexcept;
    bool wireframeEnabled() const noexcept;
    bool orbitsVisible() const noexcept;
    bool starsVisible() const noexcept;
    int simulationSpeedPercent() const noexcept;

public slots:
    void setAnimationEnabled(bool enabled);
    void setLightingEnabled(bool enabled);
    void setWireframeEnabled(bool enabled);
    void setOrbitsVisible(bool visible);
    void setStarsVisible(bool visible);
    void setSimulationSpeed(int percent);
    void resetView();

signals:
    void framesPerSecondChanged(double fps);
    void animationEnabledChanged(bool enabled);
    void lightingEnabledChanged(bool enabled);
    void wireframeEnabledChanged(bool enabled);
    void orbitsVisibleChanged(bool visible);
    void starsVisibleChanged(bool visible);
    void simulationSpeedChanged(int percent);
    void messageChanged(const QString &message);

protected:
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    struct Planet
    {
        QString name;
        float radius;
        float orbitRadius;
        float orbitSeconds;
        float spinDegreesPerSecond;
        float axialTilt;
        QColor color;
        bool hasRing;
        bool hasMoon;
    };

    struct Star
    {
        QVector3D position;
        float brightness;
    };

    struct FlareParticle
    {
        float baseAngle;
        float radius;
        float height;
        float angularSpeed;
    };

    static QGLFormat requestedFormat();

    void advanceScene();
    void generateSceneData();
    void configureLighting();
    void drawStars();
    void drawOrbit(float radius, const QColor &color) const;
    void drawSun();
    void drawPlanet(const Planet &planet);
    void drawSphere(float radius, const QColor &color, int stacks, int slices) const;
    void drawRing(float innerRadius, float outerRadius, const QColor &color) const;
    void drawFlares();
    void drawHud(QPainter &painter) const;
    float angleForPeriod(float periodSeconds) const;

    std::vector<Planet> planets_;
    std::vector<Star> stars_;
    std::vector<FlareParticle> flares_;

    QTimer animationTimer_;
    QElapsedTimer frameClock_;
    QElapsedTimer fpsClock_;
    QPoint lastMousePosition_;

    double simulationSeconds_ = 0.0;
    float simulationSpeed_ = 1.0F;
    float cameraDistance_ = -24.0F;
    float cameraPitch_ = 24.0F;
    float cameraYaw_ = -28.0F;
    float cameraPanX_ = 0.0F;
    float cameraPanY_ = 0.0F;
    int framesSinceSample_ = 0;

    bool animationEnabled_ = true;
    bool lightingEnabled_ = true;
    bool wireframeEnabled_ = false;
    bool orbitsVisible_ = true;
    bool starsVisible_ = true;
};
