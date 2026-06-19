#include "Console.h"
#include <iostream>

namespace ChineseChess {

    Console& Console::Instance() {
        static Console instance;
        return instance;
    }

    void Console::Initialize() {
        window_ = GetForegroundWindow();
        outputHandle_ = GetStdHandle(STD_OUTPUT_HANDLE);
        GetCurrentConsoleFont(outputHandle_, 0, &fontInfo_);
        SetupConsoleWindow();
        SetupConsoleMode();

        GetConsoleCursorInfo(outputHandle_, &cursorInfo_);
        cursorInfo_.bVisible = false;
        SetConsoleCursorInfo(outputHandle_, &cursorInfo_);
    }

    void Console::SetupConsoleWindow() {
        // 禁止缩放、设置窗口大小
        SetWindowLong(window_, GWL_STYLE,
            GetWindowLong(window_, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);

        SMALL_RECT windowRect = { 0, 0, 37, 26 };
        CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
        GetConsoleScreenBufferInfo(outputHandle_, &bufferInfo);
        COORD bufferSize = { windowRect.Right + 1, windowRect.Bottom + 1 };
        SetConsoleScreenBufferSize(outputHandle_, bufferSize);
        SetConsoleWindowInfo(outputHandle_, true, &windowRect);
    }

    void Console::SetupConsoleMode() {
        HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);
        DWORD mode;
        GetConsoleMode(inputHandle, &mode);
        mode &= ~ENABLE_QUICK_EDIT_MODE;
        SetConsoleMode(inputHandle, mode);
    }

    void Console::Gotoxy(short x, short y) const {
        COORD coord = { x, y };
        SetConsoleCursorPosition(outputHandle_, coord);
    }

    void Console::SetColor(ConsoleColor background, ConsoleColor foreground) const {
        SetConsoleTextAttribute(outputHandle_,
            FOREGROUND_INTENSITY | BACKGROUND_INTENSITY |
            static_cast<int>(background) * 16 +
            static_cast<int>(foreground));
    }

    void Console::ClearScreen() const {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        DWORD written;
        COORD topLeft = { 0, 0 };

        GetConsoleScreenBufferInfo(outputHandle_, &csbi);
        FillConsoleOutputCharacter(outputHandle_, L' ',
            csbi.dwSize.X * csbi.dwSize.Y, topLeft, &written);
        FillConsoleOutputAttribute(outputHandle_,
            static_cast<WORD>(ConsoleColor::YELLOW) * 16 +
            static_cast<WORD>(ConsoleColor::BLACK),
            csbi.dwSize.X * csbi.dwSize.Y, topLeft, &written);
        Gotoxy(0, 0);
    }

    void Console::AdjustPointForVirtualConsole(int cols, int rows) {
        isVC = true;
        auto getPos = [&](POINT& p) {
            while (true) {
                if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
                    GetCursorPos(&p);
                    ScreenToClient(window_, &p);
                    Sleep(100);
                    if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) break;
                }
            }
            };

        Gotoxy(0, 0);
        SetColor(ConsoleColor::YELLOW, ConsoleColor::RED);
        std::cout << "@";
        SetColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
        std::cout << "点击";
        getPos(pa);

        Gotoxy(cols - 5, rows);
        SetColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
        std::cout << "点击";
        SetColor(ConsoleColor::YELLOW, ConsoleColor::RED);
        std::cout << "@";
        getPos(pb);
    }

} // namespace ChineseChess