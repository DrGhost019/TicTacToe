#pragma once

#include "engine/AI.hpp"
#include "engine/Board.hpp"

#include <optional>
#include <string>

namespace ttt {

enum class GameMode : uint8_t {
    PlayerVsAI = 0,
    PlayerVsPlayer = 1
};

struct MatchConfig {
    GameMode mode = GameMode::PlayerVsAI;
    Difficulty difficulty = Difficulty::Medium;
    std::string player1Name = "Player 1";
    std::string player2Name = "Player 2";
    Mark player1Mark = Mark::X;
};

struct Scoreboard {
    int player1Wins = 0;
    int player2Wins = 0;
    int draws = 0;
    int roundsPlayed = 0;
    int currentStreak = 0;
    int streakOwner = 0; // 0 = none, 1 = player 1, 2 = player 2/AI
};

/**
 * @brief UI-agnostic match controller handling rules, turns, scoring, undo, and AI moves.
 */
class GameSession {
public:
    explicit GameSession(uint32_t aiSeed = 0);

    void configure(const MatchConfig& config);
    void newRound();
    void swapSidesAndRestart();
    void resetScores();

    [[nodiscard]] bool applyMove(int cell);
    [[nodiscard]] std::optional<AIDecision> performAIMove();
    [[nodiscard]] bool undo();
    [[nodiscard]] bool canUndo() const noexcept;

    [[nodiscard]] const Board& board() const noexcept { return board_; }
    [[nodiscard]] const MatchConfig& config() const noexcept { return config_; }
    [[nodiscard]] const Scoreboard& scoreboard() const noexcept { return scores_; }
    [[nodiscard]] Mark currentTurn() const noexcept { return currentTurn_; }
    [[nodiscard]] Outcome outcome() const noexcept { return board_.outcome(); }
    [[nodiscard]] bool isGameOver() const noexcept { return board_.outcome() != Outcome::Ongoing; }

    [[nodiscard]] Mark player1Mark() const noexcept { return config_.player1Mark; }
    [[nodiscard]] Mark player2Mark() const noexcept { return opponentOf(config_.player1Mark); }
    [[nodiscard]] Mark aiMark() const noexcept { return player2Mark(); }

    [[nodiscard]] bool isAITurn() const noexcept {
        return config_.mode == GameMode::PlayerVsAI &&
               !isGameOver() &&
               currentTurn_ == aiMark();
    }

    [[nodiscard]] std::string player1DisplayName() const;
    [[nodiscard]] std::string player2DisplayName() const;
    [[nodiscard]] std::string nameForMark(Mark mark) const;
    [[nodiscard]] std::string winnerName() const;

    [[nodiscard]] const std::optional<AIDecision>& lastAIDecision() const noexcept {
        return lastAIDecision_;
    }

    [[nodiscard]] std::array<int, Board::kCellCount> evaluateCurrentPosition() {
        return ai_.evaluatePosition(board_, currentTurn_);
    }

private:
    MatchConfig config_{};
    Board board_{};
    AIEngine ai_;
    Scoreboard scores_{};
    Mark currentTurn_ = Mark::X;
    bool roundScored_ = false;
    std::optional<AIDecision> lastAIDecision_;

    void recordOutcomeIfFinished();
};

} // namespace ttt
