#include "solarsystemwidget.h"

#include <QtGui/QFont>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QWidget>

#include <algorithm>
#include <cmath>
#include <random>

namespace
{
constexpr int kFrameIntervalMs = 16;
constexpr double kPi = 3.14159265358979323846;
constexpr float kSunRadius = 1.25F;
constexpr float kNearPlane = 0.8F;
constexpr float kFarPlane = 120.0F;
constexpr float kFieldOfViewDegrees = 48.0F;

void setGlColor(const QColor &color, float alpha = 1.0F)
{
    glColor4f(static_cast<GLfloat>(color.redF()), static_cast<GLfloat>(color.greenF()),
              static_cast<GLfloat>(color.blueF()), alpha);
}
} // namespace

QGLFormat SolarSystemWidget::requestedFormat()
{
    // 固定管线需要 OpenGL 2.1 兼容模式。
    QGLFormat format = QGLFormat::defaultFormat();
    format.setVersion(2, 1);
    format.setProfile(QGLFormat::CompatibilityProfile);
    format.setDepth(true);
    format.setDoubleBuffer(true);
    format.setSampleBuffers(true);
    format.setSamples(8);
    return format;
}

SolarSystemWidget::SolarSystemWidget(QWidget *parent)
    : QGLWidget(requestedFormat(), parent)
    , planets_({
          {QStringLiteral("水星"), 0.32F, 3.0F, 4.0F, 82.0F, 7.0F, QColor(150, 157, 166), false,
           false},
          {QStringLiteral("金星"), 0.52F, 4.5F, 6.3F, -34.0F, 3.4F, QColor(225, 174, 92), false,
           false},
          {QStringLiteral("地球"), 0.58F, 6.2F, 8.8F, 96.0F, 23.4F, QColor(54, 137, 226), false,
           true},
          {QStringLiteral("火星"), 0.44F, 8.0F, 12.0F, 88.0F, 25.2F, QColor(205, 82, 52), false,
           false},
          {QStringLiteral("木星"), 1.12F, 10.8F, 18.5F, 138.0F, 3.1F, QColor(211, 157, 106), false,
           false},
          {QStringLiteral("土星"), 0.94F, 14.1F, 26.0F, 122.0F, 26.7F, QColor(222, 195, 130), true,
           false},
      })
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAutoFillBackground(false);

    generateSceneData();
    frameClock_.start();
    fpsClock_.start();

    animationTimer_.setTimerType(Qt::PreciseTimer);
    connect(&animationTimer_, &QTimer::timeout, this, &SolarSystemWidget::advanceScene);
    animationTimer_.start(kFrameIntervalMs);
}

QSize SolarSystemWidget::minimumSizeHint() const
{
    return {640, 480};
}

QSize SolarSystemWidget::sizeHint() const
{
    return {980, 700};
}

bool SolarSystemWidget::animationEnabled() const noexcept
{
    return animationEnabled_;
}

bool SolarSystemWidget::lightingEnabled() const noexcept
{
    return lightingEnabled_;
}

bool SolarSystemWidget::wireframeEnabled() const noexcept
{
    return wireframeEnabled_;
}

bool SolarSystemWidget::orbitsVisible() const noexcept
{
    return orbitsVisible_;
}

bool SolarSystemWidget::starsVisible() const noexcept
{
    return starsVisible_;
}

int SolarSystemWidget::simulationSpeedPercent() const noexcept
{
    return static_cast<int>(std::lround(simulationSpeed_ * 100.0F));
}

void SolarSystemWidget::setAnimationEnabled(bool enabled)
{
    if (animationEnabled_ == enabled) {
        return;
    }

    animationEnabled_ = enabled;
    emit animationEnabledChanged(enabled);
    emit messageChanged(enabled ? QStringLiteral("动画已继续") : QStringLiteral("动画已暂停"));
    updateGL();
}

void SolarSystemWidget::setLightingEnabled(bool enabled)
{
    if (lightingEnabled_ == enabled) {
        return;
    }

    lightingEnabled_ = enabled;
    emit lightingEnabledChanged(enabled);
    emit messageChanged(enabled ? QStringLiteral("动态光照：开启")
                                : QStringLiteral("动态光照：关闭"));
    updateGL();
}

void SolarSystemWidget::setWireframeEnabled(bool enabled)
{
    if (wireframeEnabled_ == enabled) {
        return;
    }

    wireframeEnabled_ = enabled;
    emit wireframeEnabledChanged(enabled);
    emit messageChanged(enabled ? QStringLiteral("线框模式：开启")
                                : QStringLiteral("线框模式：关闭"));
    updateGL();
}

void SolarSystemWidget::setOrbitsVisible(bool visible)
{
    if (orbitsVisible_ == visible) {
        return;
    }

    orbitsVisible_ = visible;
    emit orbitsVisibleChanged(visible);
    updateGL();
}

void SolarSystemWidget::setStarsVisible(bool visible)
{
    if (starsVisible_ == visible) {
        return;
    }

    starsVisible_ = visible;
    emit starsVisibleChanged(visible);
    updateGL();
}

void SolarSystemWidget::setSimulationSpeed(int percent)
{
    const int clampedPercent = std::clamp(percent, 10, 300);
    if (simulationSpeedPercent() == clampedPercent) {
        return;
    }

    simulationSpeed_ = static_cast<float>(clampedPercent) / 100.0F;
    emit simulationSpeedChanged(clampedPercent);
    updateGL();
}

void SolarSystemWidget::resetView()
{
    cameraDistance_ = -24.0F;
    cameraPitch_ = 24.0F;
    cameraYaw_ = -28.0F;
    cameraPanX_ = 0.0F;
    cameraPanY_ = 0.0F;
    emit messageChanged(QStringLiteral("相机视角已重置"));
    updateGL();
}

void SolarSystemWidget::initializeGL()
{
    qglClearColor(QColor(3, 7, 18));

    glClearDepth(1.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void SolarSystemWidget::resizeGL(int width, int height)
{
    const int safeHeight = std::max(height, 1);
    const double aspect = static_cast<double>(width) / static_cast<double>(safeHeight);
    const double halfHeight =
        static_cast<double>(kNearPlane) * std::tan(kFieldOfViewDegrees * kPi / 360.0);
    const double halfWidth = halfHeight * aspect;

    glViewport(0, 0, width, safeHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-halfWidth, halfWidth, -halfHeight, halfHeight, static_cast<double>(kNearPlane),
              static_cast<double>(kFarPlane));
    glMatrixMode(GL_MODELVIEW);
}

void SolarSystemWidget::paintGL()
{
    // QPainter 与原生 OpenGL 共用同一帧，最后绘制 HUD。
    QPainter painter(this);
    painter.beginNativePainting();

    // QPainter 会修改投影和裁剪状态，原生绘制前必须恢复。
    resizeGL(width(), height());
    glDisable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glPolygonMode(GL_FRONT_AND_BACK, wireframeEnabled_ ? GL_LINE : GL_FILL);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(cameraPanX_, cameraPanY_, cameraDistance_);
    glRotatef(cameraPitch_, 1.0F, 0.0F, 0.0F);
    glRotatef(cameraYaw_, 0.0F, 1.0F, 0.0F);

    drawStars();
    configureLighting();

    if (orbitsVisible_) {
        for (const Planet &planet : planets_) {
            drawOrbit(planet.orbitRadius, planet.color.darker(155));
        }
    }

    drawSun();
    drawFlares();
    configureLighting();

    for (const Planet &planet : planets_) {
        drawPlanet(planet);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    // QPainter 的文字和抗锯齿图元依赖纹理状态。
    glEnable(GL_TEXTURE_2D);
    glFlush();

    painter.endNativePainting();
    painter.resetTransform();
    painter.setClipping(false);
    drawHud(painter);
}

void SolarSystemWidget::mousePressEvent(QMouseEvent *event)
{
    lastMousePosition_ = event->pos();
    setFocus(Qt::MouseFocusReason);
    event->accept();
}

void SolarSystemWidget::mouseMoveEvent(QMouseEvent *event)
{
    const QPoint delta = event->pos() - lastMousePosition_;
    lastMousePosition_ = event->pos();

    if ((event->buttons() & Qt::LeftButton) != 0) {
        cameraYaw_ += static_cast<float>(delta.x()) * 0.45F;
        cameraPitch_ =
            std::clamp(cameraPitch_ + static_cast<float>(delta.y()) * 0.35F, -85.0F, 85.0F);
        updateGL();
    } else if ((event->buttons() & Qt::RightButton) != 0) {
        cameraPanX_ += static_cast<float>(delta.x()) * 0.018F;
        cameraPanY_ -= static_cast<float>(delta.y()) * 0.018F;
        updateGL();
    } else if ((event->buttons() & Qt::MiddleButton) != 0) {
        cameraDistance_ =
            std::clamp(cameraDistance_ + static_cast<float>(delta.y()) * 0.06F, -48.0F, -8.0F);
        updateGL();
    }

    event->accept();
}

void SolarSystemWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    resetView();
    event->accept();
}

void SolarSystemWidget::wheelEvent(QWheelEvent *event)
{
    const float steps = static_cast<float>(event->angleDelta().y()) / 120.0F;
    cameraDistance_ = std::clamp(cameraDistance_ + steps * 1.4F, -48.0F, -8.0F);
    updateGL();
    event->accept();
}

void SolarSystemWidget::advanceScene()
{
    const double deltaSeconds =
        std::clamp(static_cast<double>(frameClock_.restart()) / 1000.0, 0.0, 0.05);

    if (animationEnabled_) {
        simulationSeconds_ += deltaSeconds * static_cast<double>(simulationSpeed_);
    }

    ++framesSinceSample_;
    const qint64 sampleMilliseconds = fpsClock_.elapsed();
    if (sampleMilliseconds >= 500) {
        const double fps = static_cast<double>(framesSinceSample_) * 1000.0 /
                           static_cast<double>(sampleMilliseconds);
        emit framesPerSecondChanged(fps);
        framesSinceSample_ = 0;
        fpsClock_.restart();
    }

    updateGL();
}

void SolarSystemWidget::generateSceneData()
{
    // 固定随机种子保证每次运行得到相同星空。
    std::mt19937 randomEngine(2026U);
    std::uniform_real_distribution<float> coordinate(-1.0F, 1.0F);
    std::uniform_real_distribution<float> starRadius(34.0F, 48.0F);
    std::uniform_real_distribution<float> brightness(0.45F, 1.0F);

    stars_.reserve(850);
    while (stars_.size() < 850U) {
        QVector3D direction(coordinate(randomEngine), coordinate(randomEngine),
                            coordinate(randomEngine));
        if (direction.lengthSquared() < 0.05F) {
            continue;
        }
        stars_.push_back(
            {direction.normalized() * starRadius(randomEngine), brightness(randomEngine)});
    }

    std::uniform_real_distribution<float> angle(0.0F, 360.0F);
    std::uniform_real_distribution<float> radius(kSunRadius * 1.05F, kSunRadius * 2.15F);
    std::uniform_real_distribution<float> height(-0.95F, 0.95F);
    std::uniform_real_distribution<float> speed(18.0F, 72.0F);

    flares_.reserve(100);
    for (int index = 0; index < 100; ++index) {
        flares_.push_back({
            angle(randomEngine),
            radius(randomEngine),
            height(randomEngine),
            speed(randomEngine),
        });
    }
}

void SolarSystemWidget::configureLighting()
{
    if (!lightingEnabled_) {
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
        return;
    }

    constexpr GLfloat lightPosition[] = {0.0F, 0.0F, 0.0F, 1.0F};
    constexpr GLfloat diffuse[] = {1.0F, 0.88F, 0.68F, 1.0F};
    constexpr GLfloat ambient[] = {0.07F, 0.09F, 0.15F, 1.0F};
    constexpr GLfloat specular[] = {0.9F, 0.8F, 0.65F, 1.0F};

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
}

void SolarSystemWidget::drawStars()
{
    if (!starsVisible_) {
        return;
    }

    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glPointSize(1.7F);
    glBegin(GL_POINTS);
    for (const Star &star : stars_) {
        const float blue = std::min(1.0F, star.brightness + 0.12F);
        glColor4f(star.brightness * 0.78F, star.brightness * 0.88F, blue, 1.0F);
        glVertex3f(star.position.x(), star.position.y(), star.position.z());
    }
    glEnd();
    glDepthMask(GL_TRUE);
}

void SolarSystemWidget::drawOrbit(float radius, const QColor &color) const
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    setGlColor(color, 0.52F);
    glLineWidth(1.0F);

    glBegin(GL_LINE_LOOP);
    for (int segment = 0; segment < 160; ++segment) {
        const double angle = 2.0 * kPi * static_cast<double>(segment) / 160.0;
        glVertex3f(radius * static_cast<float>(std::cos(angle)), 0.0F,
                   radius * static_cast<float>(std::sin(angle)));
    }
    glEnd();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    if (lightingEnabled_) {
        glEnable(GL_LIGHTING);
    }
}

void SolarSystemWidget::drawSun()
{
    glDisable(GL_LIGHTING);
    drawSphere(kSunRadius, QColor(255, 150, 35), 24, 36);

    // 多层透明光环模拟太阳辉光。
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    for (int layer = 0; layer < 4; ++layer) {
        const float radius = kSunRadius + 0.12F + static_cast<float>(layer) * 0.12F;
        glColor4f(1.0F, 0.45F, 0.08F, 0.34F - static_cast<float>(layer) * 0.06F);
        glLineWidth(2.0F);
        glBegin(GL_LINE_LOOP);
        for (int segment = 0; segment < 96; ++segment) {
            const double angle = 2.0 * kPi * static_cast<double>(segment) / 96.0;
            glVertex3f(radius * static_cast<float>(std::cos(angle)),
                       radius * static_cast<float>(std::sin(angle)), 0.0F);
        }
        glEnd();
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void SolarSystemWidget::drawPlanet(const Planet &planet)
{
    glPushMatrix();
    glRotatef(angleForPeriod(planet.orbitSeconds), 0.0F, 1.0F, 0.0F);
    glTranslatef(planet.orbitRadius, 0.0F, 0.0F);

    glPushMatrix();
    glRotatef(planet.axialTilt, 0.0F, 0.0F, 1.0F);
    glRotatef(static_cast<float>(std::fmod(
                  simulationSeconds_ * static_cast<double>(planet.spinDegreesPerSecond), 360.0)),
              0.0F, 1.0F, 0.0F);
    drawSphere(planet.radius, planet.color, 20, 30);
    glPopMatrix();

    if (planet.hasRing) {
        glPushMatrix();
        glRotatef(planet.axialTilt, 0.0F, 0.0F, 1.0F);
        drawRing(planet.radius * 1.28F, planet.radius * 2.15F, QColor(224, 198, 145));
        glPopMatrix();
    }

    if (planet.hasMoon) {
        glPushMatrix();
        glRotatef(angleForPeriod(2.1F), 0.0F, 1.0F, 0.0F);
        glTranslatef(planet.radius * 2.25F, 0.0F, 0.0F);
        drawSphere(planet.radius * 0.27F, QColor(190, 196, 205), 12, 18);
        glPopMatrix();
    }

    glPopMatrix();
}

void SolarSystemWidget::drawSphere(float radius, const QColor &color, int stacks, int slices) const
{
    const float red = static_cast<float>(color.redF());
    const float green = static_cast<float>(color.greenF());
    const float blue = static_cast<float>(color.blueF());

    auto emitVertex = [radius, red, green, blue](float latitude, float longitude) {
        const float cosLatitude = std::cos(latitude);
        const float x = cosLatitude * std::cos(longitude);
        const float y = std::sin(latitude);
        const float z = cosLatitude * std::sin(longitude);
        const float shade = 0.82F + (y + 1.0F) * 0.09F;

        glColor4f(red * shade, green * shade, blue * shade, 1.0F);
        glNormal3f(x, y, z);
        glVertex3f(radius * x, radius * y, radius * z);
    };

    for (int stack = 0; stack < stacks; ++stack) {
        const float latitude0 = static_cast<float>(-kPi / 2.0 + kPi * stack / stacks);
        const float latitude1 = static_cast<float>(-kPi / 2.0 + kPi * (stack + 1) / stacks);

        glBegin(GL_QUAD_STRIP);
        for (int slice = 0; slice <= slices; ++slice) {
            const float longitude = static_cast<float>(2.0 * kPi * slice / slices);
            emitVertex(latitude1, longitude);
            emitVertex(latitude0, longitude);
        }
        glEnd();
    }
}

void SolarSystemWidget::drawRing(float innerRadius, float outerRadius, const QColor &color) const
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    setGlColor(color, 0.68F);

    glBegin(GL_QUAD_STRIP);
    for (int segment = 0; segment <= 128; ++segment) {
        const double angle = 2.0 * kPi * static_cast<double>(segment) / 128.0;
        const float cosine = static_cast<float>(std::cos(angle));
        const float sine = static_cast<float>(std::sin(angle));
        glNormal3f(0.0F, 1.0F, 0.0F);
        glVertex3f(innerRadius * cosine, 0.0F, innerRadius * sine);
        glVertex3f(outerRadius * cosine, 0.0F, outerRadius * sine);
    }
    glEnd();

    glDisable(GL_BLEND);
    if (lightingEnabled_) {
        glEnable(GL_LIGHTING);
    }
}

void SolarSystemWidget::drawFlares()
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    glPointSize(3.2F);

    glBegin(GL_POINTS);
    for (const FlareParticle &particle : flares_) {
        const double angleDegrees = static_cast<double>(particle.baseAngle) +
                                    simulationSeconds_ * static_cast<double>(particle.angularSpeed);
        const double angleRadians = angleDegrees * kPi / 180.0;
        const float pulse =
            0.11F * static_cast<float>(std::sin(simulationSeconds_ * 3.0 + angleRadians));
        const float radius = particle.radius + pulse;
        const float brightness = std::clamp(1.25F - radius / (kSunRadius * 2.4F), 0.25F, 0.8F);

        glColor4f(1.0F, 0.48F + brightness * 0.35F, 0.08F, brightness);
        glVertex3f(radius * static_cast<float>(std::cos(angleRadians)),
                   particle.height + 0.15F * static_cast<float>(std::sin(angleRadians * 1.7)),
                   radius * static_cast<float>(std::sin(angleRadians)));
    }
    glEnd();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void SolarSystemWidget::drawHud(QPainter &painter) const
{
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QColor panelColor(5, 12, 26, 188);
    const QColor borderColor(89, 155, 220, 105);
    const QColor textColor(225, 236, 248);
    const QColor secondaryText(158, 178, 202);

    const QRectF infoRect(16.0, 16.0, 286.0, 88.0);
    painter.setPen(QPen(borderColor, 1.0));
    painter.setBrush(panelColor);
    painter.drawRoundedRect(infoRect, 9.0, 9.0);

    QFont titleFont = painter.font();
    titleFont.setPointSize(11);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.setPen(textColor);
    painter.drawText(QPointF(30.0, 43.0), QStringLiteral("QGLWidget 太阳系实验室"));

    QFont detailFont = painter.font();
    detailFont.setPointSize(9);
    detailFont.setBold(false);
    painter.setFont(detailFont);
    painter.setPen(secondaryText);
    painter.drawText(QPointF(30.0, 68.0), QStringLiteral("速度 %1x  ·  相机距离 %2")
                                              .arg(simulationSpeed_, 0, 'f', 2)
                                              .arg(-cameraDistance_, 0, 'f', 1));
    painter.drawText(
        QPointF(30.0, 90.0),
        QStringLiteral("%1  ·  光照 %2  ·  %3")
            .arg(animationEnabled_ ? QStringLiteral("运行中") : QStringLiteral("已暂停"))
            .arg(lightingEnabled_ ? QStringLiteral("开") : QStringLiteral("关"))
            .arg(wireframeEnabled_ ? QStringLiteral("线框") : QStringLiteral("实体")));

    const qreal legendWidth = 142.0;
    const qreal legendHeight = 40.0 + static_cast<qreal>(planets_.size()) * 22.0;
    const QRectF legendRect(static_cast<qreal>(width()) - legendWidth - 16.0, 16.0, legendWidth,
                            legendHeight);
    painter.setPen(QPen(borderColor, 1.0));
    painter.setBrush(panelColor);
    painter.drawRoundedRect(legendRect, 9.0, 9.0);

    painter.setPen(textColor);
    painter.drawText(
        QRectF(legendRect.left() + 14.0, legendRect.top() + 9.0, legendWidth - 28.0, 22.0),
        Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("行星图例"));

    qreal y = legendRect.top() + 40.0;
    for (const Planet &planet : planets_) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(planet.color);
        painter.drawEllipse(QPointF(legendRect.left() + 20.0, y), 5.0, 5.0);
        painter.setPen(secondaryText);
        painter.drawText(QPointF(legendRect.left() + 34.0, y + 5.0), planet.name);
        y += 22.0;
    }
}

float SolarSystemWidget::angleForPeriod(float periodSeconds) const
{
    if (periodSeconds <= 0.0F) {
        return 0.0F;
    }

    return static_cast<float>(
        std::fmod(simulationSeconds_ / static_cast<double>(periodSeconds) * 360.0, 360.0));
}
