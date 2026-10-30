#pragma once

// MSVC 2026 已移除 Qt 5.15.2 仍使用的 stdext 数组迭代器。
// 改用 Qt 在其他编译器上的原始指针兼容方案。
#if defined(_MSC_VER) && _MSC_VER >= 1950
#include <QtCore/qcompilerdetection.h>

#undef QT_MAKE_CHECKED_ARRAY_ITERATOR
#define QT_MAKE_CHECKED_ARRAY_ITERATOR(pointer, size) (pointer)

#undef QT_MAKE_UNCHECKED_ARRAY_ITERATOR
#define QT_MAKE_UNCHECKED_ARRAY_ITERATOR(pointer) (pointer)
#endif
