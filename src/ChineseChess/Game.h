#pragma once
#include "ChineseChess.h"
#include "ChessBoard.h"
#include <optional>

namespace ChineseChess {

    struct GameConfig {
        int ctrlMode = 1; // 1:Êó±ê 2:¼üÅÌ 3:Êó¼ü 4:¼üÊó
        int ruleMode = 1; // 1:¿ªÆô 2:¹Ø±Õ
        int winMode = 2;  // 1:¡Ü7 2:¡İ10
    };

    bool ShowMainMenu(GameConfig& config);
    void GameLoop(const GameConfig& config, bool isNewGame = true);

    // ÊäÈë¸¨Öú
    std::optional<Position> GetMousePos(const std::vector<int>& keyInterrupts = {});
    std::optional<Position> GetKeyboardPos(MessageBar& msgBar);

} // namespace ChineseChess