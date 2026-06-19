#include "Console.h"

HANDLE Console::getOutputHandle() {
    static HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    return out;
}

void Console::enableVirtualTerminal() {
    DWORD mode = 0;
    GetConsoleMode(getOutputHandle(), &mode);
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(getOutputHandle(), mode);
}

bool Console::isKeyDown(int virtualKey) {
    return GetAsyncKeyState(virtualKey) & 0x8000;
}

void Console::setWindowSize(int cols, int rows) {
    HANDLE out = getOutputHandle();
    COORD buf = { static_cast<SHORT>(cols), static_cast<SHORT>(rows) };
    SetConsoleScreenBufferSize(out, buf);
    SMALL_RECT rect = { 0, 0, static_cast<SHORT>(cols - 1), static_cast<SHORT>(rows - 1) };
    SetConsoleWindowInfo(out, TRUE, &rect);
}