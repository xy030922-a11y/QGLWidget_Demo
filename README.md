# QGLWidget Demo

这是一个使用经典 `QGLWidget` API 的 Qt 5 / OpenGL 示例，已针对本机的以下工具链配置：

- Visual Studio 2026 / MSVC 19.51（x64）
- CMake 4.2 或更高版本
- Qt 5.15.2 `msvc2019_64`
- Windows SDK 10.0.26100.0

> `QGLWidget` 是 Qt 5 的旧版兼容 API；Qt 6 项目应优先使用 `QOpenGLWidget`。

项目通过强制预包含 `src/msvc2026_qt5_compat.h` 处理 MSVC 2026 已移除、但 Qt 5.15.2 仍引用的 `stdext` 数组迭代器，因此不需要修改 Qt 安装目录中的头文件。

## 配置和编译

在项目根目录执行：

```powershell
cmake --workflow --preset msvc2026-debug
```

或者分步执行：

```powershell
cmake --preset msvc2026-debug
cmake --build --preset msvc2026-debug
```

Release 构建：

```powershell
cmake --workflow --preset msvc2026-release
```

生成的程序位于：

```text
out/build/msvc2026-debug/Debug/QGLWidgetDemo.exe
out/build/msvc2026-release/Release/QGLWidgetDemo.exe
```

构建完成后，CMake 会调用 `windeployqt`，把 Qt DLL 和 `platforms/qwindows*.dll` 自动部署到程序目录。

## VS Code

安装 Microsoft 的 **C/C++** 和 **CMake Tools** 扩展，然后打开本目录。选择 `MSVC 2026 x64 - Debug` 配置预设即可构建；按 `F5` 可使用 `cppvsdbg` 启动调试。

## 操作

- 左键拖动：绕 X/Y 轴旋转
- 右键拖动：绕 X/Z 轴旋转
- 鼠标滚轮：缩放
- `Space`：暂停/继续动画
- `R`：重置视角
- `Esc`：退出

