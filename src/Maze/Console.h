#pragma once
#include <windows.h>

namespace Console {
    // 获取标准输出句柄（惰性初始化，确保只调用一次）
    HANDLE getOutputHandle();
    // 启用虚拟终端序列（ANSI）
    void enableVirtualTerminal();
    // 检查按键是否按下
    bool isKeyDown(int virtualKey);
    // 设置控制台窗口缓冲区大小
    void setWindowSize(int cols, int rows);
}