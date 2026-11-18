#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace ttt {

enum class Mark : uint8_t {
    Empty = 0,
    X = 1,
    O = 2
};

[[nodiscard]] constexpr Mark opponentOf(Mark mark) noexcept {
    if (mark == Mark::X) return Mark::O;
    if (mark == Mark::O) return Mark::X;
    return Mark::Empty;
}

[[nodiscard]] constexpr char markToChar(Mark mark) noexcept {
    switch (mark) {
        case Mark::X: return 'X';
        case Mark::O: return 'O';
        case Mark::Empty: return ' ';
    }
    return ' ';
}

enum class Outcome : uint8_t {
    Ongoing = 0,
    XWins,
    OWins,
    Draw
};

struct MoveRecord {
    int cell = -1;
    Mark mark = Mark::Empty;
};

/**
 * @brief Bitboard-backed 3x3 Tic-Tac-Toe board.
 *
 * Cells are indexed 0..8 left-to-right, top-to-bottom:
 *   0 | 1 | 2
 *   ---------
 *   3 | 4 | 5
 *   ---------
 *   6 | 7 | 8
 *
 * Each player's pieces occupy a 9-bit bitmask inside a uint16_t, enabling
 * O(1) move validation, O(1) win/draw detection, and an exact 18-bit state key
 * for AI transposition caching.
 */
class Board {
public:
    static constexpr int kCellCount = 9;
    static constexpr uint16_t kFullMask = 0x1FFu;

    static constexpr std::array<uint16_t, 8> kWinMasks = {
        0b000000111u, // Row 0 (0,1,2)
        0b000111000u, // Row 1 (3,4,5)
        0b111000000u, // Row 2 (6,7,8)
        0b001001001u, // Col 0 (0,3,6)
        0b010010010u, // Col 1 (1,4,7)
        0b100100100u, // Col 2 (2,5,8)
        0b100010001u, // Main diag (0,4,8)
        0b001010100u  // Anti diag (2,4,6)
    };

    static constexpr std::array<std::array<int, 3>, 8> kWinLines = {{
        {0, 1, 2},
        {3, 4, 5},
        {6, 7, 8},
        {0, 3, 6},
        {1, 4, 7},
        {2, 5, 8},
        {0, 4, 8},
        {2, 4, 6}
    }};

    Board() noexcept = default;

    void reset() noexcept;

    [[nodiscard]] bool makeMove(int cell, Mark mark) noexcept;
    [[nodiscard]] bool undoMove() noexcept;

    [[nodiscard]] bool isValidCell(int cell) const noexcept {
        return cell >= 0 && cell < kCellCount;
    }

    [[nodiscard]] bool isEmpty(int cell) const noexcept {
        if (!isValidCell(cell)) return false;
        const uint16_t bit = static_cast<uint16_t>(1u << cell);
        return ((maskX_ | maskO_) & bit) == 0u;
    }

    [[nodiscard]] Mark at(int cell) const noexcept {
        if (!isValidCell(cell)) return Mark::Empty;
        const uint16_t bit = static_cast<uint16_t>(1u << cell);
        if (maskX_ & bit) return Mark::X;
        if (maskO_ & bit) return Mark::O;
        return Mark::Empty;
    }

    [[nodiscard]] uint16_t maskFor(Mark mark) const noexcept {
        if (mark == Mark::X) return maskX_;
        if (mark == Mark::O) return maskO_;
        return static_cast<uint16_t>(~(maskX_ | maskO_) & kFullMask);
    }

    [[nodiscard]] uint16_t occupiedMask() const noexcept {
        return static_cast<uint16_t>(maskX_ | maskO_);
    }

    [[nodiscard]] uint16_t emptyMask() const noexcept {
        return static_cast<uint16_t>(~(maskX_ | maskO_) & kFullMask);
    }

    [[nodiscard]] int moveCount() const noexcept { return historySize_; }
    [[nodiscard]] int remainingMoves() const noexcept { return kCellCount - historySize_; }
    [[nodiscard]] bool isFull() const noexcept { return occupiedMask() == kFullMask; }

    [[nodiscard]] bool hasWon(Mark mark) const noexcept;
    [[nodiscard]] Outcome outcome() const noexcept;
    [[nodiscard]] std::optional<std::array<int, 3>> winningLine() const noexcept;
    [[nodiscard]] std::vector<int> availableMoves() const;
    [[nodiscard]] int countImmediateWins(Mark mark) const noexcept;
    [[nodiscard]] std::optional<MoveRecord> lastMove() const noexcept;

    [[nodiscard]] const std::array<MoveRecord, kCellCount>& history() const noexcept {
        return history_;
    }

    /// Compact 18-bit key uniquely representing the entire board state.
    [[nodiscard]] uint32_t stateKey() const noexcept {
        return (static_cast<uint32_t>(maskX_) << 9u) | static_cast<uint32_t>(maskO_);
    }

private:
    uint16_t maskX_ = 0u;
    uint16_t maskO_ = 0u;
    std::array<MoveRecord, kCellCount> history_{};
    int historySize_ = 0;
};

} // namespace ttt
