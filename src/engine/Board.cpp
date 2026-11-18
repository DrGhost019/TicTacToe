#include "engine/Board.hpp"

namespace ttt {

void Board::reset() noexcept {
    maskX_ = 0u;
    maskO_ = 0u;
    historySize_ = 0;
}

bool Board::makeMove(int cell, Mark mark) noexcept {
    if (mark == Mark::Empty || !isEmpty(cell)) {
        return false;
    }
    const uint16_t bit = static_cast<uint16_t>(1u << cell);
    if (mark == Mark::X) {
        maskX_ |= bit;
    } else {
        maskO_ |= bit;
    }
    history_[static_cast<size_t>(historySize_)] = MoveRecord{cell, mark};
    ++historySize_;
    return true;
}

bool Board::undoMove() noexcept {
    if (historySize_ <= 0) {
        return false;
    }
    --historySize_;
    const MoveRecord rec = history_[static_cast<size_t>(historySize_)];
    const uint16_t bit = static_cast<uint16_t>(1u << rec.cell);
    if (rec.mark == Mark::X) {
        maskX_ &= static_cast<uint16_t>(~bit);
    } else if (rec.mark == Mark::O) {
        maskO_ &= static_cast<uint16_t>(~bit);
    }
    return true;
}

bool Board::hasWon(Mark mark) const noexcept {
    if (mark == Mark::Empty) return false;
    const uint16_t m = (mark == Mark::X) ? maskX_ : maskO_;
    for (uint16_t winMask : kWinMasks) {
        if ((m & winMask) == winMask) {
            return true;
        }
    }
    return false;
}

Outcome Board::outcome() const noexcept {
    if (hasWon(Mark::X)) return Outcome::XWins;
    if (hasWon(Mark::O)) return Outcome::OWins;
    if (isFull()) return Outcome::Draw;
    return Outcome::Ongoing;
}

std::optional<std::array<int, 3>> Board::winningLine() const noexcept {
    for (size_t i = 0; i < kWinMasks.size(); ++i) {
        const uint16_t w = kWinMasks[i];
        if ((maskX_ & w) == w || (maskO_ & w) == w) {
            return kWinLines[i];
        }
    }
    return std::nullopt;
}

std::vector<int> Board::availableMoves() const {
    std::vector<int> moves;
    moves.reserve(static_cast<size_t>(remainingMoves()));
    const uint16_t empty = emptyMask();
    for (int cell = 0; cell < kCellCount; ++cell) {
        if (empty & static_cast<uint16_t>(1u << cell)) {
            moves.push_back(cell);
        }
    }
    return moves;
}

int Board::countImmediateWins(Mark mark) const noexcept {
    if (mark == Mark::Empty) return 0;
    const uint16_t own = maskFor(mark);
    const uint16_t empty = emptyMask();
    int count = 0;
    for (int cell = 0; cell < kCellCount; ++cell) {
        const uint16_t bit = static_cast<uint16_t>(1u << cell);
        if ((empty & bit) == 0u) continue;
        const uint16_t nextOwn = static_cast<uint16_t>(own | bit);
        for (uint16_t winMask : kWinMasks) {
            if ((nextOwn & winMask) == winMask) {
                ++count;
                break;
            }
        }
    }
    return count;
}

std::optional<MoveRecord> Board::lastMove() const noexcept {
    if (historySize_ <= 0) return std::nullopt;
    return history_[static_cast<size_t>(historySize_ - 1)];
}

} // namespace ttt
