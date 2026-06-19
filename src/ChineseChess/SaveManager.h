#pragma once
#include "ChineseChess.h"
#include "Piece.h"
#include <array>
#include <string>

namespace ChineseChess {

    class SaveManager {
    public:
        using BoardArray = std::array<std::array<std::unique_ptr<Piece>, BOARD_COLS>, BOARD_ROWS>;

        static bool SaveGame(const BoardArray& board, PieceColor currentPlayer, const std::string& filename);
        static bool LoadGame(BoardArray& board, PieceColor& currentPlayer, const std::string& filename);
    };

} // namespace ChineseChess