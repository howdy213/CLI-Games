#include "ChessBoard.h"
#include "Console.h"
#include "SaveManager.h"
#include <format>
#include <iostream>

namespace ChineseChess {

    ChessBoard::ChessBoard(int winMode)
        : currentPlayer_(PieceColor::RED), gameOver_(false), winner_(PieceColor::NONE),
        winMode_(winMode), msgBar_(MSG_BAR_TOP_ROW, MSG_BAR_MAX_LINES) {
        InitializeBoard();
        msgBar_.AddMessage("游戏开始！红方先行");
    }

    ChessBoard::ChessBoard(int winMode, const std::string& filename)
        : winMode_(winMode), msgBar_(MSG_BAR_TOP_ROW, MSG_BAR_MAX_LINES) {
        if (SaveManager::LoadGame(board_, currentPlayer_, filename)) {
            gameOver_ = false;
            winner_ = PieceColor::NONE;
            msgBar_.AddMessage(std::format("(已加载存档) 当前回合: {}",
                currentPlayer_ == PieceColor::RED ? "红方" : "黑方"));
        }
        else {
            InitializeBoard();
            currentPlayer_ = PieceColor::RED;
            gameOver_ = false;
            winner_ = PieceColor::NONE;
            msgBar_.AddMessage("(无存档) 新游戏: 红方先行");
        }
    }

    void ChessBoard::InitializeBoard() {
        PlacePieces(PieceColor::BLACK, 0, 2, 3);
        PlacePieces(PieceColor::RED, 9, 7, 6);
    }

    void ChessBoard::PlacePieces(PieceColor color, int backRow, int cannonRow, int soldierRow) {
        board_[backRow][0] = CreatePiece(PieceType::R_CHARIOT, color);
        board_[backRow][1] = CreatePiece(PieceType::R_HORSE, color);
        board_[backRow][2] = CreatePiece(PieceType::R_ELEPHANT, color);
        board_[backRow][3] = CreatePiece(PieceType::R_ADVISOR, color);
        board_[backRow][4] = CreatePiece(PieceType::R_GENERAL, color);
        board_[backRow][5] = CreatePiece(PieceType::R_ADVISOR, color);
        board_[backRow][6] = CreatePiece(PieceType::R_ELEPHANT, color);
        board_[backRow][7] = CreatePiece(PieceType::R_HORSE, color);
        board_[backRow][8] = CreatePiece(PieceType::R_CHARIOT, color);

        board_[cannonRow][1] = CreatePiece(PieceType::R_CANNON, color);
        board_[cannonRow][7] = CreatePiece(PieceType::R_CANNON, color);

        board_[soldierRow][0] = CreatePiece(PieceType::R_SOLDIER, color);
        board_[soldierRow][2] = CreatePiece(PieceType::R_SOLDIER, color);
        board_[soldierRow][4] = CreatePiece(PieceType::R_SOLDIER, color);
        board_[soldierRow][6] = CreatePiece(PieceType::R_SOLDIER, color);
        board_[soldierRow][8] = CreatePiece(PieceType::R_SOLDIER, color);
    }

    void ChessBoard::Display() const {
        auto& console = Console::Instance();
        console.Gotoxy(0, 0);
        console.SetColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
        std::cout << "保存                             返回\n";
        std::cout << "   0   1   2   3   4   5   6   7   8\n";

        for (int y = 0; y < BOARD_ROWS; ++y) {
            console.Gotoxy(0, 2 * y + 2);
            console.SetColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
            std::cout << ' ' << y;

            for (int x = 0; x < BOARD_COLS; ++x) {
                std::cout << ' ';
                const auto& piece = board_[y][x];
                if (piece) {
                    console.SetColor(ConsoleColor::YELLOW,
                        piece->GetColor() == PieceColor::RED ? ConsoleColor::RED : ConsoleColor::BLACK);
                    std::cout << piece->GetChineseChar();
                    console.SetColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
                }
                else {
                    DrawGrid(y, x);
                }
                if (x != 8) std::cout << "─";
            }
            std::cout << " \n";
            console.SetColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
            DrawSpecialLines(y + 2);
        }

        console.Gotoxy(0, MSG_BAR_TOP_ROW - 2);
        console.SetColor(ConsoleColor::YELLOW, currentPlayer_ == PieceColor::BLACK ? ConsoleColor::BLACK : ConsoleColor::RED);
        std::cout << (currentPlayer_ == PieceColor::BLACK ? "黑" : "红");
        msgBar_.Display();
    }

    void ChessBoard::DrawGrid(int y, int x) const {
        if (y == 0 && x == 0) std::cout << "┌─";
        else if (y == 0 && x == 8) std::cout << "┐";
        else if (y == 9 && x == 0) std::cout << "└─";
        else if (y == 9 && x == 8) std::cout << "┘";
        else if (x == 0) std::cout << "├─";
        else if (x == 8) std::cout << "┤";
        else if ((y == 9 || y == 4) && x < 8) std::cout << "┴─";
        else if ((y == 0 || y == 5) && x < 8) std::cout << "┬─";
        else std::cout << "┼─";
    }

    void ChessBoard::DrawSpecialLines(int row) const {
        switch (row) {
        case 2: // 黑方九宫斜线
            if (winMode_ == 1)
                std::cout << "  │  │  │  │╲│╱│  │  │  │ \n";
            else
                std::cout << "   │   │   │   │ ╲ │ ╱ │   │   │   │ \n";
            break;
        case 3:
            if (winMode_ == 1)
                std::cout << "  │  │  │  │╱│╲│  │  │  │ \n";
            else
                std::cout << "   │   │   │   │ ╱ │ ╲ │   │   │   │ \n";
            break;
        case 6: // 楚河汉界
            if (winMode_ == 1)
                std::cout << "  │       楚河       汉界        │ \n";
            else
                std::cout << "   │        楚河       汉界        │ \n";
            break;
        case 9: // 红方九宫斜线
            if (winMode_ == 1)
                std::cout << "  │  │  │  │╲│╱│  │  │  │ \n";
            else
                std::cout << "   │   │   │   │ ╲ │ ╱ │   │   │   │ \n";
            break;
        case 10:
            if (winMode_ == 1)
                std::cout << "  │  │  │  │╱│╲│  │  │  │ \n";
            else
                std::cout << "   │   │   │   │ ╱ │ ╲ │   │   │   │ \n";
            break;
        default: // 普通横线
            if (row >= 11) return;
            if (winMode_ == 1)
                std::cout << "  │  │  │  │  │  │  │  │  │ \n";
            else
                std::cout << "   │   │   │   │   │   │   │   │   │ \n";
            break;
        }
    }

    bool ChessBoard::MovePiece(Position from, Position to, bool enforceRules) {
        if (!board_[from.y][from.x]) {
            msgBar_.AddMessage("起始位置没有棋子！", true);
            return false;
        }
        if (board_[from.y][from.x]->GetColor() != currentPlayer_) {
            msgBar_.AddMessage(std::format("请选择{}方棋子！", currentPlayer_ == PieceColor::RED ? "红" : "黑"), true);
            return false;
        }
        if (enforceRules && !board_[from.y][from.x]->IsValidMove(board_, from, to)) {
            msgBar_.AddMessage("此移动不符合棋子规则！", true);
            return false;
        }

        std::string pieceName = board_[from.y][from.x]->GetChineseChar();
        msgBar_.AddMessage(std::format("已移动{}：({},{}) → ({},{})", pieceName, from.x, from.y, to.x, to.y));

        board_[to.y][to.x] = std::move(board_[from.y][from.x]);
        CheckGameOver();
        currentPlayer_ = (currentPlayer_ == PieceColor::RED) ? PieceColor::BLACK : PieceColor::RED;
        return true;
    }

    void ChessBoard::CheckGameOver() {
        bool redAlive = false, blackAlive = false;
        for (const auto& row : board_) {
            for (const auto& piece : row) {
                if (piece) {
                    if (piece->GetType() == PieceType::R_GENERAL) redAlive = true;
                    else if (piece->GetType() == PieceType::B_GENERAL) blackAlive = true;
                }
            }
        }
        if (!redAlive) { gameOver_ = true; winner_ = PieceColor::BLACK; }
        else if (!blackAlive) { gameOver_ = true; winner_ = PieceColor::RED; }
    }

    bool ChessBoard::SaveGame(const std::string& filename) {
        bool ok = SaveManager::SaveGame(board_, currentPlayer_, filename);
        msgBar_.AddMessage(ok ? "游戏已保存至 " + filename : "保存失败！", !ok);
        return ok;
    }

} // namespace ChineseChess