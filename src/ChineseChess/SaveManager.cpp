#include "SaveManager.h"
#include <fstream>

namespace ChineseChess {

    bool SaveManager::SaveGame(const BoardArray& board, PieceColor currentPlayer, const std::string& filename) {
        std::ofstream file(filename);
        if (!file) return false;

        file << (currentPlayer == PieceColor::RED ? "RED" : "BLACK") << '\n';
        for (int y = 0; y < BOARD_ROWS; ++y) {
            for (int x = 0; x < BOARD_COLS; ++x) {
                file << (board[y][x] ? static_cast<int>(board[y][x]->GetType()) : 0) << ' ';
            }
            file << '\n';
        }
        return true;
    }

    bool SaveManager::LoadGame(BoardArray& board, PieceColor& currentPlayer, const std::string& filename) {
        std::ifstream file(filename);
        if (!file) return false;

        // Çå¿ÕÆåÅÌ
        for (auto& row : board) {
            for (auto& cell : row) {
                cell.reset();
            }
        }

        std::string playerStr;
        file >> playerStr;
        currentPlayer = (playerStr == "RED") ? PieceColor::RED : PieceColor::BLACK;

        for (int y = 0; y < BOARD_ROWS; ++y) {
            for (int x = 0; x < BOARD_COLS; ++x) {
                int typeInt;
                file >> typeInt;
                if (typeInt != 0) {
                    auto type = static_cast<PieceType>(typeInt);
                    PieceColor color = (typeInt >= static_cast<int>(PieceType::R_GENERAL) &&
                        typeInt <= static_cast<int>(PieceType::R_SOLDIER))
                        ? PieceColor::RED : PieceColor::BLACK;
                    board[y][x] = CreatePiece(type, color);
                }
            }
        }
        return true;
    }

} // namespace ChineseChess