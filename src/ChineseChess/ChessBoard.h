#pragma once
#include "ChineseChess.h"
#include "Piece.h"
#include "MessageBar.h"
#include <array>
#include <string>

namespace ChineseChess {

    class ChessBoard {
    public:
        explicit ChessBoard(int winMode);
        ChessBoard(int winMode, const std::string& filename); // º”‘ÿ¥Êµµ

        void Display() const;
        bool MovePiece(Position from, Position to, bool enforceRules = true);
        bool IsGameOver() const { return gameOver_; }
        PieceColor GetCurrentPlayer() const { return currentPlayer_; }
        PieceColor GetWinner() const { return winner_; }
        MessageBar& GetMessageBar() { return msgBar_; }
        bool SaveGame(const std::string& filename = "savegame.dat");

    private:
        using BoardArray = std::array<std::array<std::unique_ptr<Piece>, BOARD_COLS>, BOARD_ROWS>;

        void InitializeBoard();
        void PlacePieces(PieceColor color, int backRow, int cannonRow, int soldierRow);
        void CheckGameOver();
        void DrawGrid(int y, int x) const;
        void DrawSpecialLines(int row) const;

        BoardArray board_;
        PieceColor currentPlayer_;
        bool gameOver_;
        PieceColor winner_;
        int winMode_;
        MessageBar msgBar_;
    };

} // namespace ChineseChess