#include "engine/AI.hpp"
#include "engine/Board.hpp"
#include "engine/GameSession.hpp"

#include <cassert>
#include <chrono>
#include <iostream>
#include <string>

using namespace ttt;

namespace {

int testsPassed = 0;

void check(bool cond, const std::string& msg) {
    if (!cond) {
        std::cerr << "[FAIL] " << msg << '\n';
        std::exit(1);
    }
    ++testsPassed;
}

void testBoardBasics() {
    Board b;
    check(b.moveCount() == 0, "Initial board has 0 moves");
    check(b.remainingMoves() == 9, "Initial board has 9 remaining moves");
    check(b.outcome() == Outcome::Ongoing, "Initial board is ongoing");
    check(!b.makeMove(-1, Mark::X), "Reject negative cell");
    check(!b.makeMove(9, Mark::X), "Reject cell >= 9");
    check(!b.makeMove(0, Mark::Empty), "Reject Empty mark");

    check(b.makeMove(4, Mark::X), "Place X at center");
    check(!b.isEmpty(4), "Center is no longer empty");
    check(b.at(4) == Mark::X, "Center holds X");
    check(!b.makeMove(4, Mark::O), "Reject move on occupied cell");

    check(b.undoMove(), "Undo center move");
    check(b.isEmpty(4), "Center is empty after undo");
    check(b.moveCount() == 0, "Move count back to 0");
    check(!b.undoMove(), "Undo on empty board returns false");

    // Verify all 8 winning lines
    for (size_t i = 0; i < Board::kWinLines.size(); ++i) {
        b.reset();
        const auto& line = Board::kWinLines[i];
        check(b.makeMove(line[0], Mark::X), "Win line cell 0");
        check(b.makeMove(line[1], Mark::X), "Win line cell 1");
        check(b.outcome() == Outcome::Ongoing, "Not won with 2 in a row");
        check(b.makeMove(line[2], Mark::X), "Win line cell 2");
        check(b.outcome() == Outcome::XWins, "X wins on line " + std::to_string(i));
        const auto wl = b.winningLine();
        check(wl.has_value() && *wl == line, "winningLine() matches exact line");
    }

    // Verify draw board:
    // X O X
    // X O O
    // O X X
    b.reset();
    const std::array<std::pair<int, Mark>, 9> drawSeq = {{
        {0, Mark::X}, {1, Mark::O}, {2, Mark::X},
        {3, Mark::X}, {4, Mark::O}, {5, Mark::O},
        {6, Mark::O}, {7, Mark::X}, {8, Mark::X}
    }};
    for (auto [cell, mark] : drawSeq) {
        check(b.makeMove(cell, mark), "Apply draw sequence cell");
    }
    check(b.isFull(), "Draw board is full");
    check(b.outcome() == Outcome::Draw, "Draw outcome detected");
    check(!b.winningLine().has_value(), "No winning line on draw");
}

struct ExhaustiveStats {
    int games = 0;
    int aiWins = 0;
    int draws = 0;
    int aiLosses = 0;
};

void exploreAllHumanResponses(Board& board, AIEngine& ai, Mark aiMark, Mark turn,
                              ExhaustiveStats& stats) {
    const Outcome out = board.outcome();
    if (out != Outcome::Ongoing) {
        ++stats.games;
        if (out == Outcome::Draw) {
            ++stats.draws;
        } else if ((out == Outcome::XWins && aiMark == Mark::X) ||
                   (out == Outcome::OWins && aiMark == Mark::O)) {
            ++stats.aiWins;
        } else {
            ++stats.aiLosses;
        }
        return;
    }

    if (turn == aiMark) {
        const AIDecision dec = ai.chooseMove(board, aiMark, Difficulty::Explosive);
        check(dec.cell >= 0 && board.isEmpty(dec.cell), "Explosive AI chose legal cell");
        (void)board.makeMove(dec.cell, aiMark);
        exploreAllHumanResponses(board, ai, aiMark, opponentOf(turn), stats);
        (void)board.undoMove();
    } else {
        const auto moves = board.availableMoves();
        for (int cell : moves) {
            (void)board.makeMove(cell, turn);
            exploreAllHumanResponses(board, ai, aiMark, opponentOf(turn), stats);
            (void)board.undoMove();
        }
    }
}

void testExplosiveUnbeatableExhaustive() {
    AIEngine ai(2026);
    Board board;

    const auto t0 = std::chrono::steady_clock::now();

    // Case 1: Explosive plays X (first)
    ExhaustiveStats asX;
    exploreAllHumanResponses(board, ai, Mark::X, Mark::X, asX);
    check(asX.aiLosses == 0, "Explosive as X has 0 losses across every possible human line");
    check(asX.aiWins > 0, "Explosive as X wins against human mistakes");

    // Case 2: Explosive plays O (second)
    board.reset();
    ExhaustiveStats asO;
    exploreAllHumanResponses(board, ai, Mark::O, Mark::X, asO);
    check(asO.aiLosses == 0, "Explosive as O has 0 losses across every possible human line");
    check(asO.aiWins > 0, "Explosive as O punishes human mistakes");

    const auto t1 = std::chrono::steady_clock::now();
    const double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::cout << "  [Explosive Exhaustive Proof]\n"
              << "    As X: " << asX.games << " terminal lines (" << asX.aiWins
              << " wins, " << asX.draws << " draws, " << asX.aiLosses << " losses)\n"
              << "    As O: " << asO.games << " terminal lines (" << asO.aiWins
              << " wins, " << asO.draws << " draws, " << asO.aiLosses << " losses)\n"
              << "    Verified in " << ms << " ms\n";
}

void testExplosiveTrapMaximizerOpening() {
    AIEngine ai(42);
    Board empty;
    const AIDecision opening = ai.chooseMove(empty, Mark::X, Difficulty::Explosive);
    // Corners (0, 2, 6, 8) create 7/8 lethal human-blunder traps on turn 1.
    const bool isCorner = (opening.cell == 0 || opening.cell == 2 ||
                           opening.cell == 6 || opening.cell == 8);
    check(isCorner, "Explosive AI discovers corner opening maximizes lethal traps (7/8)");
    check(opening.trapCount == 7, "Corner opening reports exactly 7 traps");
}

void testTacticalIntelligenceAndTiers() {
    AIEngine ai(1337);

    // Immediate winning move test for Medium, Hard, Explosive
    Board winBoard;
    (void)winBoard.makeMove(0, Mark::X);
    (void)winBoard.makeMove(1, Mark::X);
    (void)winBoard.makeMove(3, Mark::O);
    (void)winBoard.makeMove(4, Mark::O);

    for (Difficulty diff : {Difficulty::Medium, Difficulty::Hard, Difficulty::Explosive}) {
        const AIDecision d = ai.chooseMove(winBoard, Mark::X, diff);
        check(d.cell == 2, std::string(difficultyName(diff)) + " takes immediate win at cell 2");
    }

    // Immediate block test for Hard and Explosive
    Board blockBoard;
    (void)blockBoard.makeMove(0, Mark::X);
    (void)blockBoard.makeMove(1, Mark::X);
    (void)blockBoard.makeMove(4, Mark::O);
    for (Difficulty diff : {Difficulty::Hard, Difficulty::Explosive}) {
        const AIDecision d = ai.chooseMove(blockBoard, Mark::O, diff);
        check(d.cell == 2, std::string(difficultyName(diff)) + " blocks immediate threat at cell 2");
    }

    // Tier ladder simulation: pit each tier against a random player (200 games each)
    auto winRateVsRandom = [](Difficulty diff) {
        AIEngine simAI(99);
        AIEngine randomPlayer(77);
        int wins = 0;
        int losses = 0;
        constexpr int kGames = 200;
        for (int g = 0; g < kGames; ++g) {
            Board b;
            Mark turn = Mark::X;
            const Mark aiMark = (g % 2 == 0) ? Mark::X : Mark::O;
            while (b.outcome() == Outcome::Ongoing) {
                int cell = -1;
                if (turn == aiMark) {
                    cell = simAI.chooseMove(b, aiMark, diff).cell;
                } else {
                    const auto moves = b.availableMoves();
                    cell = moves[static_cast<size_t>((g * 31 + b.moveCount() * 17) % static_cast<int>(moves.size()))];
                }
                (void)b.makeMove(cell, turn);
                turn = opponentOf(turn);
            }
            const Outcome o = b.outcome();
            if ((o == Outcome::XWins && aiMark == Mark::X) ||
                (o == Outcome::OWins && aiMark == Mark::O)) {
                ++wins;
            } else if (o != Outcome::Draw) {
                ++losses;
            }
        }
        return std::pair<int, int>{wins, losses};
    };

    const auto [easyW, easyL] = winRateVsRandom(Difficulty::Easy);
    const auto [medW, medL] = winRateVsRandom(Difficulty::Medium);
    const auto [hardW, hardL] = winRateVsRandom(Difficulty::Hard);
    const auto [expW, expL] = winRateVsRandom(Difficulty::Explosive);

    std::cout << "  [200-Game Ladder vs Naive Opponent]\n"
              << "    Easy:      " << easyW << "W / " << easyL << "L\n"
              << "    Medium:    " << medW  << "W / " << medL  << "L\n"
              << "    Hard:      " << hardW << "W / " << hardL << "L\n"
              << "    Explosive: " << expW  << "W / " << expL  << "L\n";

    check(expL == 0, "Explosive suffers 0 losses in simulation");
    check(medW > easyW, "Medium outperforms Easy");
    check(expW >= medW, "Explosive outperforms Medium");
    check(hardL <= medL, "Hard loses less or equal to Medium");
}

void testGameSession() {
    GameSession session(2026);
    MatchConfig cfg;
    cfg.mode = GameMode::PlayerVsPlayer;
    cfg.player1Name = "   "; // Should fallback cleanly
    cfg.player2Name = "Alex";
    cfg.player1Mark = Mark::X;
    session.configure(cfg);

    check(session.player1DisplayName() == "Player 1", "Blank P1 name falls back to Player 1");
    check(session.player2DisplayName() == "Alex", "P2 name preserved");

    check(session.applyMove(0), "X moves 0");
    check(session.applyMove(3), "O moves 3");
    check(session.applyMove(1), "X moves 1");
    check(session.applyMove(4), "O moves 4");
    check(session.applyMove(2), "X moves 2 -> X wins");

    check(session.isGameOver(), "Session detects game over");
    check(session.scoreboard().player1Wins == 1, "P1 score incremented");
    check(session.scoreboard().currentStreak == 1, "Streak incremented");

    // Undo should revert finished state and score cleanly
    check(session.undo(), "Undo after win succeeds");
    check(!session.isGameOver(), "Game ongoing after undo");
    check(session.scoreboard().player1Wins == 0, "Score reverted cleanly on undo");

    // PvAI mode where human picks O -> AI plays X first
    cfg.mode = GameMode::PlayerVsAI;
    cfg.difficulty = Difficulty::Explosive;
    cfg.player1Mark = Mark::O;
    session.configure(cfg);
    check(session.isAITurn(), "AI opens when human selects O");
    const auto aiMove = session.performAIMove();
    check(aiMove.has_value() && aiMove->cell >= 0, "AI executes opening move");
    check(!session.canUndo(), "Cannot undo AI's opening move before human moves");
}

} // namespace

int main() {
    std::cout << "Running Tic-Tac-Toe 2026 Engine Verification Suite...\n";
    testBoardBasics();
    testExplosiveTrapMaximizerOpening();
    testExplosiveUnbeatableExhaustive();
    testTacticalIntelligenceAndTiers();
    testGameSession();
    std::cout << "ALL " << testsPassed << " CHECKS PASSED!\n";
    return 0;
}
