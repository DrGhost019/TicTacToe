#include "engine/GameSession.hpp"

#include <algorithm>
#include <cctype>

namespace ttt {

namespace {

std::string sanitizeName(const std::string& raw, const std::string& fallback) {
    auto first = std::find_if_not(raw.begin(), raw.end(),
                                  [](unsigned char ch) { return std::isspace(ch) != 0; });
    auto last = std::find_if_not(raw.rbegin(), raw.rend(),
                                 [](unsigned char ch) { return std::isspace(ch) != 0; }).base();
    if (first >= last) {
        return fallback;
    }
    return std::string(first, last);
}

} // namespace

GameSession::GameSession(uint32_t aiSeed)
    : ai_(aiSeed) {}

void GameSession::configure(const MatchConfig& config) {
    config_ = config;
    if (config_.player1Mark != Mark::X && config_.player1Mark != Mark::O) {
        config_.player1Mark = Mark::X;
    }
    config_.player1Name = sanitizeName(config_.player1Name, "Player 1");
    config_.player2Name = sanitizeName(config_.player2Name, "Player 2");
    resetScores();
    newRound();
}

void GameSession::newRound() {
    board_.reset();
    currentTurn_ = Mark::X; // X always opens in standard Tic-Tac-Toe
    roundScored_ = false;
    lastAIDecision_.reset();
}

void GameSession::swapSidesAndRestart() {
    config_.player1Mark = opponentOf(config_.player1Mark);
    newRound();
}

void GameSession::resetScores() {
    scores_ = Scoreboard{};
}

bool GameSession::applyMove(int cell) {
    if (isGameOver()) return false;
    if (!board_.makeMove(cell, currentTurn_)) {
        return false;
    }

    if (board_.outcome() == Outcome::Ongoing) {
        currentTurn_ = opponentOf(currentTurn_);
    } else {
        recordOutcomeIfFinished();
    }
    return true;
}

std::optional<AIDecision> GameSession::performAIMove() {
    if (!isAITurn()) {
        return std::nullopt;
    }
    AIDecision decision = ai_.chooseMove(board_, aiMark(), config_.difficulty);
    if (decision.cell < 0) {
        return std::nullopt;
    }
    if (!applyMove(decision.cell)) {
        return std::nullopt;
    }
    lastAIDecision_ = decision;
    return decision;
}

bool GameSession::canUndo() const noexcept {
    if (board_.moveCount() == 0) return false;
    if (config_.mode == GameMode::PlayerVsAI) {
        // Don't undo the AI's opening move when the human hasn't moved yet.
        if (board_.moveCount() == 1 && player1Mark() == Mark::O) {
            return false;
        }
    }
    return true;
}

bool GameSession::undo() {
    if (!canUndo()) return false;

    // If undoing after a completed round, do not retroactively mutate lifetime scores,
    // just allow replaying from the prior board state cleanly (preventing double-scoring
    // by keeping roundScored_ = true if the round already concluded? Better: allow fresh scoring
    // by reverting the round's score if roundScored_ was set!).
    if (roundScored_) {
        const Outcome prevOut = board_.outcome();
        if (prevOut == Outcome::Draw) {
            scores_.draws = std::max(0, scores_.draws - 1);
        } else {
            const Mark winner = (prevOut == Outcome::XWins) ? Mark::X : Mark::O;
            if (winner == player1Mark()) {
                scores_.player1Wins = std::max(0, scores_.player1Wins - 1);
            } else {
                scores_.player2Wins = std::max(0, scores_.player2Wins - 1);
            }
        }
        scores_.roundsPlayed = std::max(0, scores_.roundsPlayed - 1);
        scores_.currentStreak = 0;
        scores_.streakOwner = 0;
        roundScored_ = false;
    }

    if (config_.mode == GameMode::PlayerVsPlayer) {
        const auto last = board_.lastMove();
        if (!board_.undoMove()) return false;
        currentTurn_ = last ? last->mark : Mark::X;
        return true;
    }

    // Player vs AI: roll back to the human's turn.
    auto last = board_.lastMove();
    if (!last) return false;

    if (last->mark == aiMark()) {
        (void)board_.undoMove();
        if (board_.moveCount() > 0) {
            const auto prev = board_.lastMove();
            if (prev && prev->mark == player1Mark()) {
                (void)board_.undoMove();
            }
        }
    } else {
        (void)board_.undoMove();
    }

    currentTurn_ = player1Mark();
    lastAIDecision_.reset();
    return true;
}

void GameSession::recordOutcomeIfFinished() {
    if (roundScored_) return;
    const Outcome out = board_.outcome();
    if (out == Outcome::Ongoing) return;

    roundScored_ = true;
    ++scores_.roundsPlayed;

    if (out == Outcome::Draw) {
        ++scores_.draws;
        scores_.currentStreak = 0;
        scores_.streakOwner = 0;
        return;
    }

    const Mark winner = (out == Outcome::XWins) ? Mark::X : Mark::O;
    if (winner == player1Mark()) {
        ++scores_.player1Wins;
        if (scores_.streakOwner == 1) {
            ++scores_.currentStreak;
        } else {
            scores_.streakOwner = 1;
            scores_.currentStreak = 1;
        }
    } else {
        ++scores_.player2Wins;
        if (scores_.streakOwner == 2) {
            ++scores_.currentStreak;
        } else {
            scores_.streakOwner = 2;
            scores_.currentStreak = 1;
        }
    }
}

std::string GameSession::player1DisplayName() const {
    return config_.player1Name;
}

std::string GameSession::player2DisplayName() const {
    if (config_.mode == GameMode::PlayerVsAI) {
        return std::string("AI • ") + std::string(difficultyName(config_.difficulty));
    }
    return config_.player2Name;
}

std::string GameSession::nameForMark(Mark mark) const {
    if (mark == player1Mark()) return player1DisplayName();
    if (mark == player2Mark()) return player2DisplayName();
    return "";
}

std::string GameSession::winnerName() const {
    const Outcome out = board_.outcome();
    if (out == Outcome::XWins) return nameForMark(Mark::X);
    if (out == Outcome::OWins) return nameForMark(Mark::O);
    return "";
}

} // namespace ttt
