#pragma once
#include "ChineseChess.h"
#include <windows.h>

namespace ChineseChess {

    class Console {
    public:
        Console(const Console&) = delete;
        Console& operator=(const Console&) = delete;

        void Initialize();
        void Gotoxy(short x, short y) const;
        void SetColor(ConsoleColor background, ConsoleColor foreground) const;
        void ClearScreen() const;
        void AdjustPointForVirtualConsole(int cols, int rows);

        HWND GetWindowHandle() const { return window_; }
        const CONSOLE_FONT_INFO& GetFontInfo() const { return fontInfo_; }

        // ตฅภýทรฮส
        static Console& Instance();

        bool isVC = false;
        POINT pa{};
        POINT pb{};

    private:
        Console() = default;
        void SetupConsoleWindow();
        void SetupConsoleMode();

        HWND window_ = nullptr;
        HANDLE outputHandle_ = nullptr;
        CONSOLE_FONT_INFO fontInfo_{};
        CONSOLE_CURSOR_INFO cursorInfo_{};
    };

} // namespace ChineseChess