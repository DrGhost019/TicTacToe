#pragma once

#include "engine/Board.hpp"

#include <array>
#include <cstdint>
#include <random>
#include <string>
#include <string_view>
#include <vector>

namespace ttt {

enum class Difficulty : uint8_t {
    Easy = 0,
    Medium = 1,
    Hard = 2,
    Explosive = 3
};

[[nodiscard]] constexpr std::string_view difficultyName(Difficulty diff) noexcept {
    switch (diff) {
        case Difficulty::Easy:      return "Easy";
        case Difficulty::Medium:    return "Medium";
        case Difficulty::Hard:      return "Hard";
        case Difficulty::Explosive: return "Explosive";
    }
    return "Unknown";
}

[[nodiscard]] constexpr std::string_view difficultySubtitle(Difficulty diff) noexcept {
    switch (diff) {
        case Difficulty::Easy:      return "Casual Rookie • Forgiving & playful";
        case Difficulty::Medium:    return "Tactician • Blocks threats, shallow lookahead";
        case Difficulty::Hard:      return "Grandmaster • Deep Alpha-Beta Minimax";
        case Difficulty::Explosive: return "Unbeatable • Perfect Trap-Maximizer + FX Overdrive";
    }
    return "";
}

struct AIDecision {
    static constexpr int kNoScore = -9999;

    int cell = -1;
    int score = 0;
    int nodesEvaluated = 0;
    double thinkTimeMs = 0.0;
    int trapCount = 0;
    std::string tag = "Move";
    std::array<int, Board::kCellCount> cellScores{
        kNoScore, kNoScore, kNoScore,
        kNoScore, kNoScore, kNoScore,
        kNoScore, kNoScore, kNoScore
    };
};

/**
 * @brief 4-tier Tic-Tac-Toe AI Engine (Easy, Medium, Hard, Explosive).
 *
 * Uses bitboard state transitions, Alpha-Beta pruning, strategic move ordering,
 * an exact 18-bit transposition table, and a Trap-Maximizer tie-breaker for
 * Explosive mode.
 */
class AIEngine {
public:
    explicit AIEngine(uint32_t seed = 0);

    [[nodiscard]] AIDecision chooseMove(const Board& board, Mark aiMark, Difficulty difficulty);

    /**
     * @brief Evaluates every empty square from `perspective`'s point of view
     * using the exact Minimax solver (used for the GUI "AI Vision" heatmap).
     */
    [[nodiscard]] std::array<int, Board::kCellCount> evaluatePosition(const Board& board, Mark perspective);

    void clearCache() noexcept;

private:
    enum class Bound : uint8_t {
        Empty = 0,
        Exact = 1,
        Lower = 2,
        Upper = 3
    };

    struct TTEntry {
        int16_t score = 0;
        Bound bound = Bound::Empty;
        uint8_t generation = 0;
    };

    static constexpr int kInf = 1000;
    static constexpr std::array<int, 9> kMoveOrder = {4, 0, 2, 6, 8, 1, 3, 5, 7};
    static constexpr std::array<int, 9> kPositionalWeight = {
        3, 1, 3,
        1, 5, 1,
        3, 1, 3
    };

    // 2^18 board states * 2 turn flags = 524,288 slots (~2 MB flat O(1) cache).
    static constexpr size_t kTableSize = (1u << 19u);

    std::mt19937 rng_;
    std::vector<TTEntry> tt_;
    uint8_t ttGen_ = 1;
    Mark cachedAiMark_ = Mark::Empty;

    void ensureCacheFor(Mark aiMark) noexcept;
    [[nodiscard]] int findImmediateWin(const Board& board, Mark mark) const noexcept;
    [[nodiscard]] int alphaBeta(Board& board, Mark aiMark, Mark currentTurn,
                                int alpha, int beta, int depthRemaining,
                                bool useTT, int& nodesEvaluated) noexcept;
    [[nodiscard]] int heuristicEval(const Board& board, Mark aiMark) const noexcept;
    [[nodiscard]] int countLethalTraps(Board& board, Mark aiMark, int& trapPressure,
                                       int& nodesEvaluated) noexcept;

    [[nodiscard]] AIDecision chooseEasy(const Board& board, Mark aiMark);
    [[nodiscard]] AIDecision chooseMedium(const Board& board, Mark aiMark);
    [[nodiscard]] AIDecision chooseHard(const Board& board, Mark aiMark);
    [[nodiscard]] AIDecision chooseExplosive(const Board& board, Mark aiMark);

    [[nodiscard]] std::string buildTag(const Board& beforeMove, int chosenCell,
                                       Mark aiMark, int score, int trapCount,
                                       Difficulty diff) const;
};

} // namespace ttt
