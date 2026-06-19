#pragma once
#include "ChineseChess.h"
#include <array>
#include <memory>
#include <string>

namespace ChineseChess {

    // 前向声明棋盘类型
    class ChessBoard;

    class Piece {
    public:
        explicit Piece(PieceType type, PieceColor color);
        virtual ~Piece() = default;

        [[nodiscard]] virtual bool IsValidMove(
            const std::array<std::array<std::unique_ptr<Piece>, BOARD_COLS>, BOARD_ROWS>& board,
            Position from, Position to) const = 0;

        [[nodiscard]] std::string GetChineseChar() const;
        [[nodiscard]] PieceType GetType() const { return type_; }
        [[nodiscard]] PieceColor GetColor() const { return color_; }
        [[nodiscard]] bool IsAlive() const { return alive_; }
        void SetDead() { alive_ = false; }

    protected:
        [[nodiscard]] bool IsSameColor(
            const std::array<std::array<std::unique_ptr<Piece>, BOARD_COLS>, BOARD_ROWS>& board,
            Position to) const;

        PieceType type_;
        PieceColor color_;
        bool alive_ = true;
    };

    // ---------- 具体棋子类声明 ----------

    class Chariot : public Piece {
    public:
        explicit Chariot(PieceColor c);
        bool IsValidMove(
            const std::array<std::array<std::unique_ptr<Piece>, BOARD_COLS>, BOARD_ROWS>& board,
            Position from, Position to) const override;
    };

    class Horse : public Piece {
    public:
        explicit Horse(PieceColor c);
        bool IsValidMove(
            const std::array<std::array<std::unique_ptr<Piece>, BOARD_COLS>, BOARD_ROWS>& board,
            Position from, Position to) const override;
    };

    class Elephant : public Piece {
    public:
        explicit Elephant(PieceColor c);
        bool IsValidMove(
            const std::array<std::array<std::unique_ptr<Piece>, BOARD_COLS>, BOARD_ROWS>& board,
            Position from, Position to) const override;
    };

    class Advisor : public Piece {
    public:
        explicit Advisor(PieceColor c);
        bool IsValidMove(
            const std::array<std::array<std::unique_ptr<Piece>, BOARD_COLS>, BOARD_ROWS>& board,
            Position from, Position to) const override;
    };

    class General : public Piece {
    public:
        explicit General(PieceColor c);
        bool IsValidMove(
            const std::array<std::array<std::unique_ptr<Piece>, BOARD_COLS>, BOARD_ROWS>& board,
            Position from, Position to) const override;
    private:
        bool CheckPalace(Position p) const;
    };

    class Cannon : public Piece {
    public:
        explicit Cannon(PieceColor c);
        bool IsValidMove(
            const std::array<std::array<std::unique_ptr<Piece>, BOARD_COLS>, BOARD_ROWS>& board,
            Position from, Position to) const override;
    };

    class Soldier : public Piece {
    public:
        explicit Soldier(PieceColor c);
        bool IsValidMove(
            const std::array<std::array<std::unique_ptr<Piece>, BOARD_COLS>, BOARD_ROWS>& board,
            Position from, Position to) const override;
    };

    // 工厂函数
    std::unique_ptr<Piece> CreatePiece(PieceType type, PieceColor color);

} // namespace ChineseChess