#include "ChineseChess.h"
#include "Console.h"
#include "Game.h"

int main() {
    ChineseChess::Console::Instance().Initialize();
    ChineseChess::GameConfig config;

    while (true) {
        bool isNewGame = ChineseChess::ShowMainMenu(config);
        ChineseChess::GameLoop(config, isNewGame);
    }
    return 0;
}