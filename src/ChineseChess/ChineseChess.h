#pragma once
#include <cstdint>
#include <string>
#include <string_view>

namespace ChineseChess {

    // 棋盘尺寸常量
    inline constexpr int BOARD_ROWS = 10;
    inline constexpr int BOARD_COLS = 9;

    // 九宫边界
    inline constexpr int RED_GENERAL_ROW_MIN = 7;
    inline constexpr int RED_GENERAL_ROW_MAX = 9;
    inline constexpr int RED_GENERAL_COL_MIN = 3;
    inline constexpr int RED_GENERAL_COL_MAX = 5;
    inline constexpr int BLACK_GENERAL_ROW_MIN = 0;
    inline constexpr int BLACK_GENERAL_ROW_MAX = 2;
    inline constexpr int BLACK_GENERAL_COL_MIN = 3;
    inline constexpr int BLACK_GENERAL_COL_MAX = 5;

    // 楚河汉界行
    inline constexpr int RIVER_ROW = 4;

    // 界面布局常量
    inline constexpr int MSG_BAR_TOP_ROW = 24;
    inline constexpr int MSG_BAR_MAX_LINES = 3;
    inline constexpr int MENU_SETTINGS_Y = 10;
    inline constexpr int MENU_START_Y = 12;
    inline constexpr int MENU_LOAD_Y = 14;
    inline constexpr int MENU_EXIT_Y = 16;

    // 颜色枚举
    enum class ConsoleColor : uint16_t {
        BLACK = 0,
        RED = 4,
        YELLOW = 14
    };

    // 坐标结构
    struct Position {
        int x = 0; // 列
        int y = 0; // 行
        int keyInterrupt = -1; // 键盘中断键值，-1表示无
    };

    // 棋子类型与颜色
    enum class PieceType : int {
        EMPTY = 0,
        // 红方
        R_GENERAL, R_ADVISOR, R_ELEPHANT, R_HORSE, R_CHARIOT, R_CANNON, R_SOLDIER,
        // 黑方
        B_GENERAL, B_ADVISOR, B_ELEPHANT, B_HORSE, B_CHARIOT, B_CANNON, B_SOLDIER
    };

    enum class PieceColor {
        RED,
        BLACK,
        NONE
    };

    // 工具函数
    void Repeat(std::string_view content, int times);

} // namespace ChineseChess