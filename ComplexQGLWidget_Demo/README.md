# Complex QGLWidget Demo

一个基于 Qt 5 `QGLWidget` 和 OpenGL 2.1 固定管线的复杂交互示例，使用 Visual Studio 2026 / MSVC 19.51 构建。

## 功能

- 六颗行星、独立公转与自转
- 地球月球系统和土星环
- 太阳点光源、辉光与动态粒子
- 程序化生成的 850 颗背景星
- 轨道线、实体/线框切换和实时 FPS
- 鼠标旋转、平移、缩放及侧边控制面板
- HUD 行星图例和运行状态

## 构建

```powershell
cd ComplexQGLWidget_Demo
cmake --workflow --preset msvc2026-debug
```

Release：

```powershell
cmake --workflow --preset msvc2026-release
```

程序输出：

```text
out/build/msvc2026-debug/Debug/ComplexQGLWidgetDemo.exe
out/build/msvc2026-release/Release/ComplexQGLWidgetDemo.exe
```

构建完成后会自动运行 `windeployqt` 部署 Qt DLL 和平台插件。

## 操作

- 左键拖动：旋转相机
- 右键拖动：平移相机
- 中键拖动或滚轮：缩放
- 双击或 `R`：重置相机
- `Space`：暂停/继续
- `L`：动态光照
- `W`：实体/线框模式
- `O`：显示/隐藏轨道
- `S`：显示/隐藏星空
- `+` / `-`：调整模拟速度
- `Esc`：退出

> `QGLWidget` 是 Qt 5 的旧版兼容 API。新项目通常应使用 `QOpenGLWidget`。

