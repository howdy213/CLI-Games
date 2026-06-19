#include "Piece.h"
#include <algorithm>
#include <cstdlib>

namespace ChineseChess {

    // ==================== Piece 基类实现 ====================

    Piece::Piece(PieceType type, PieceColor color)
        : type_(type), color_(color) {
    }

    std::string Piece::GetChineseChar() const {
        static const std::string chars[] = {
            "　",  // EMPTY
            "帅", "仕", "相", "马", "车", "炮", "兵",   // 红方 (1~7)
            "将", "士", "象", "马", "车", "炮", "卒"    // 黑方 (8~14)
        };
        return chars[static_cast<int>(type_)];
    }

    bool Piece::IsSameColor(
        const std::array<std::array<std::unique_ptr<Piece>, BOARD_COLS>, BOARD_ROWS>& board,
        Position to) const {
        // 修正：使用 GetColor() 而非直接访问 color_
        return board[to.y][to.x] && board[to.y][to.x]->GetColor() == color_;
    }


    // ==================== 车 (Chariot) ====================

    Chariot::Chariot(PieceColor c)
        : Piece(c == PieceColor::RED ? PieceType::R_CHARIOT : PieceType::B_CHARIOT, c) {
    }

    bool Chariot::IsValidMove(
        const std::array<std::array<std::unique_ptr<Piece>, BOARD_COLS>, BOARD_ROWS>& board,
        Position from, Position to) const {
        if (IsSameColor(board, to)) return false;
        if (from.x != to.x && from.y != to.y) return false;

        if (from.x == to.x) { // 垂直移动
            int step = (from.y < to.y) ? 1 : -1;
            for (int y = from.y + step; y != to.y; y += step) {
                if (board[y][from.x]) return false;
            }
        }
        else {              // 水平移动
            int step = (from.x < to.x) ? 1 : -1;
            for (int x = from.x + step; x != to.x; x += step) {
                if (board[from.y][x]) return false;
            }
        }
        return true;
    }


    // ==================== 马 (Horse) ====================

    Horse::Horse(PieceColor c)
        : Piece(c == PieceColor::RED ? PieceType::R_HORSE : PieceType::B_HORSE, c) {
    }

    bool Horse::IsValidMove(
        const std::array<std::array<std::unique_ptr<Piece>, BOARD_COLS>, BOARD_ROWS>& board,
        Position from, Position to) const {
        if (IsSameColor(board, to)) return false;

        int dx = std::abs(from.x - to.x);
        int dy = std::abs(from.y - to.y);
        if (!((dx == 1 && dy == 2) || (dx == 2 && dy == 1))) return false;

        // 马腿位置
        Position leg = from;
        if (dx == 2) {
            leg.x += (to.x > from.x) ? 1 : -1;
        }
        else {
            leg.y += (to.y > from.y) ? 1 : -1;
        }
        return board[leg.y][leg.x] == nullptr;
    }


    // ==================== 象/相 (Elephant) ====================

    Elephant::Elephant(PieceColor c)
        : Piece(c == PieceColor::RED ? PieceType::R_ELEPHANT : PieceType::B_ELEPHANT, c) {
    }

    bool Elephant::IsValidMove(
        const std::array<std::array<std::unique_ptr<Piece>, BOARD_COLS>, BOARD_ROWS>& board,
        Position from, Position to) const {
        if (IsSameColor(board, to)) return false;

        if (std::abs(from.x - to.x) != 2 || std::abs(from.y - to.y) != 2) return false;

        // 象眼
        Position eye((from.x + to.x) / 2, (from.y + to.y) / 2);
        if (board[eye.y][eye.x]) return false;

        // 不能过河
        if (color_ == PieceColor::RED && to.y < 5) return false;
        if (color_ == PieceColor::BLACK && to.y > 4) return false;

        return true;
    }


    // ==================== 士/仕 (Advisor) ====================

    Advisor::Advisor(PieceColor c)
        : Piece(c == PieceColor::RED ? PieceType::R_ADVISOR : PieceType::B_ADVISOR, c) {
    }

    bool Advisor::IsValidMove(
        const std::array<std::array<std::unique_ptr<Piece>, BOARD_COLS>, BOARD_ROWS>& board,
        Position from, Position to) const {
        if (IsSameColor(board, to)) return false;

        if (std::abs(from.x - to.x) != 1 || std::abs(from.y - to.y) != 1) return false;

        if (color_ == PieceColor::RED) {
            return to.y >= RED_GENERAL_ROW_MIN && to.y <= RED_GENERAL_ROW_MAX &&
                to.x >= RED_GENERAL_COL_MIN && to.x <= RED_GENERAL_COL_MAX;
        }
        else {
            return to.y >= BLACK_GENERAL_ROW_MIN && to.y <= BLACK_GENERAL_ROW_MAX &&
                to.x >= BLACK_GENERAL_COL_MIN && to.x <= BLACK_GENERAL_COL_MAX;
        }
    }


    // ==================== 将/帅 (General) ====================

    General::General(PieceColor c)
        : Piece(c == PieceColor::RED ? PieceType::R_GENERAL : PieceType::B_GENERAL, c) {
    }

    bool General::IsValidMove(
        const std::array<std::array<std::unique_ptr<Piece>, BOARD_COLS>, BOARD_ROWS>& board,
        Position from, Position to) const {
        if (IsSameColor(board, to)) return false;

        // 飞将（将帅对面）
        const auto& target = board[to.y][to.x];
        if (target &&
            (target->GetType() == PieceType::B_GENERAL || target->GetType() == PieceType::R_GENERAL) &&
            to.x == from.x) {
            int minY = std::min(to.y, from.y);
            int maxY = std::max(to.y, from.y);
            for (int y = minY + 1; y < maxY; ++y) {
                if (board[y][to.x]) return false;
            }
            return true;
        }

        // 正常移动：一步直线
        if (std::abs(from.x - to.x) + std::abs(from.y - to.y) != 1) return false;

        return CheckPalace(to);
    }

    bool General::CheckPalace(Position p) const {
        if (color_ == PieceColor::RED) {
            return p.y >= RED_GENERAL_ROW_MIN && p.y <= RED_GENERAL_ROW_MAX &&
                p.x >= RED_GENERAL_COL_MIN && p.x <= RED_GENERAL_COL_MAX;
        }
        else {
            return p.y >= BLACK_GENERAL_ROW_MIN && p.y <= BLACK_GENERAL_ROW_MAX &&
                p.x >= BLACK_GENERAL_COL_MIN && p.x <= BLACK_GENERAL_COL_MAX;
        }
    }


    // ==================== 炮 (Cannon) ====================

    Cannon::Cannon(PieceColor c)
        : Piece(c == PieceColor::RED ? PieceType::R_CANNON : PieceType::B_CANNON, c) {
    }

    bool Cannon::IsValidMove(
        const std::array<std::array<std::unique_ptr<Piece>, BOARD_COLS>, BOARD_ROWS>& board,
        Position from, Position to) const {
        if (IsSameColor(board, to)) return false;
        if (from.x != to.x && from.y != to.y) return false;

        int obstacleCount = 0;

        if (from.x == to.x) {
            int step = (from.y < to.y) ? 1 : -1;
            for (int y = from.y + step; y != to.y; y += step) {
                if (board[y][from.x]) ++obstacleCount;
            }
        }
        else {
            int step = (from.x < to.x) ? 1 : -1;
            for (int x = from.x + step; x != to.x; x += step) {
                if (board[from.y][x]) ++obstacleCount;
            }
        }

        bool isCapturing = (board[to.y][to.x] != nullptr);
        return (isCapturing && obstacleCount == 1) || (!isCapturing && obstacleCount == 0);
    }


    // ==================== 兵/卒 (Soldier) ====================

    Soldier::Soldier(PieceColor c)
        : Piece(c == PieceColor::RED ? PieceType::R_SOLDIER : PieceType::B_SOLDIER, c) {
    }

    bool Soldier::IsValidMove(
        const std::array<std::array<std::unique_ptr<Piece>, BOARD_COLS>, BOARD_ROWS>& board,
        Position from, Position to) const {
        if (IsSameColor(board, to)) return false;

        int dx = std::abs(from.x - to.x);
        int dy = to.y - from.y;
        if (dx > 1 || std::abs(dy) > 1 || (dx == 1 && dy != 0)) return false;

        // 只能前进
        if ((color_ == PieceColor::RED && dy > 0) || (color_ == PieceColor::BLACK && dy < 0))
            return false;

        // 未过河不能横移
        if (dx == 1) {
            if (color_ == PieceColor::RED && from.y > 4) return false;
            if (color_ == PieceColor::BLACK && from.y < 5) return false;
        }

        return true;
    }


    // ==================== 棋子工厂 ====================

    std::unique_ptr<Piece> CreatePiece(PieceType type, PieceColor color) {
        switch (type) {
        case PieceType::R_GENERAL: case PieceType::B_GENERAL:
            return std::make_unique<General>(color);
        case PieceType::R_ADVISOR: case PieceType::B_ADVISOR:
            return std::make_unique<Advisor>(color);
        case PieceType::R_ELEPHANT: case PieceType::B_ELEPHANT:
            return std::make_unique<Elephant>(color);
        case PieceType::R_HORSE: case PieceType::B_HORSE:
            return std::make_unique<Horse>(color);
        case PieceType::R_CHARIOT: case PieceType::B_CHARIOT:
            return std::make_unique<Chariot>(color);
        case PieceType::R_CANNON: case PieceType::B_CANNON:
            return std::make_unique<Cannon>(color);
        case PieceType::R_SOLDIER: case PieceType::B_SOLDIER:
            return std::make_unique<Soldier>(color);
        default:
            return nullptr;
        }
    }

} // namespace ChineseChess