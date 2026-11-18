#include "engine/AI.hpp"

#include <algorithm>
#include <bit>
#include <chrono>
#include <cmath>
#include <limits>
#include <vector>

namespace ttt {

AIEngine::AIEngine(uint32_t seed)
    : tt_(kTableSize) {
    if (seed == 0) {
        std::random_device rd;
        std::seed_seq seq{
            rd(), rd(),
            static_cast<uint32_t>(
                std::chrono::steady_clock::now().time_since_epoch().count())
        };
        rng_.seed(seq);
    } else {
        rng_.seed(seed);
    }
}

void AIEngine::clearCache() noexcept {
    ++ttGen_;
    if (ttGen_ == 0) {
        std::fill(tt_.begin(), tt_.end(), TTEntry{});
        ttGen_ = 1;
    }
    cachedAiMark_ = Mark::Empty;
}

void AIEngine::ensureCacheFor(Mark aiMark) noexcept {
    if (cachedAiMark_ != aiMark) {
        clearCache();
        cachedAiMark_ = aiMark;
    }
}

int AIEngine::findImmediateWin(const Board& board, Mark mark) const noexcept {
    if (mark == Mark::Empty) return -1;
    const uint16_t own = board.maskFor(mark);
    const uint16_t empty = board.emptyMask();
    for (int cell : kMoveOrder) {
        const uint16_t bit = static_cast<uint16_t>(1u << cell);
        if ((empty & bit) == 0u) continue;
        const uint16_t nextOwn = static_cast<uint16_t>(own | bit);
        for (uint16_t winMask : Board::kWinMasks) {
            if ((nextOwn & winMask) == winMask) {
                return cell;
            }
        }
    }
    return -1;
}

int AIEngine::heuristicEval(const Board& board, Mark aiMark) const noexcept {
    const Mark opp = opponentOf(aiMark);
    const uint16_t aiMask = board.maskFor(aiMark);
    const uint16_t oppMask = board.maskFor(opp);
    int score = 0;

    for (uint16_t winMask : Board::kWinMasks) {
        const bool aiHas = (aiMask & winMask) != 0u;
        const bool oppHas = (oppMask & winMask) != 0u;
        if (aiHas && !oppHas) {
            const int count = std::popcount(static_cast<uint16_t>(aiMask & winMask));
            score += (count == 2) ? 14 : 3;
        } else if (oppHas && !aiHas) {
            const int count = std::popcount(static_cast<uint16_t>(oppMask & winMask));
            score -= (count == 2) ? 15 : 3;
        }
    }

    for (int cell = 0; cell < Board::kCellCount; ++cell) {
        const uint16_t bit = static_cast<uint16_t>(1u << cell);
        if (aiMask & bit) score += kPositionalWeight[static_cast<size_t>(cell)];
        else if (oppMask & bit) score -= kPositionalWeight[static_cast<size_t>(cell)];
    }
    return score;
}

int AIEngine::alphaBeta(Board& board, Mark aiMark, Mark currentTurn,
                        int alpha, int beta, int depthRemaining,
                        bool useTT, int& nodesEvaluated) noexcept {
    ++nodesEvaluated;

    const Outcome out = board.outcome();
    if (out != Outcome::Ongoing) {
        if (out == Outcome::Draw) return 0;
        const bool aiWon = (out == Outcome::XWins && aiMark == Mark::X) ||
                           (out == Outcome::OWins && aiMark == Mark::O);
        // Depth-invariant terminal scoring: faster wins yield higher scores.
        const int magnitude = 100 - board.moveCount();
        return aiWon ? magnitude : -magnitude;
    }

    if (depthRemaining <= 0) {
        return heuristicEval(board, aiMark);
    }

    const bool maximizing = (currentTurn == aiMark);
    const int alphaOrig = alpha;
    const int betaOrig = beta;
    const size_t ttIdx = (static_cast<size_t>(board.stateKey()) << 1u) |
                         (maximizing ? 1u : 0u);

    if (useTT) {
        const TTEntry& entry = tt_[ttIdx];
        if (entry.generation == ttGen_ && entry.bound != Bound::Empty) {
            if (entry.bound == Bound::Exact) {
                return entry.score;
            }
            if (entry.bound == Bound::Lower) {
                alpha = std::max(alpha, static_cast<int>(entry.score));
            } else if (entry.bound == Bound::Upper) {
                beta = std::min(beta, static_cast<int>(entry.score));
            }
            if (alpha >= beta) {
                return entry.score;
            }
        }
    }

    const Mark nextTurn = opponentOf(currentTurn);
    const uint16_t empty = board.emptyMask();
    int best = maximizing ? -kInf : kInf;

    for (int cell : kMoveOrder) {
        if ((empty & static_cast<uint16_t>(1u << cell)) == 0u) continue;
        (void)board.makeMove(cell, currentTurn);
        const int val = alphaBeta(board, aiMark, nextTurn, alpha, beta,
                                  depthRemaining - 1, useTT, nodesEvaluated);
        (void)board.undoMove();

        if (maximizing) {
            if (val > best) best = val;
            if (best > alpha) alpha = best;
        } else {
            if (val < best) best = val;
            if (best < beta) beta = best;
        }
        if (alpha >= beta) {
            break;
        }
    }

    if (useTT) {
        TTEntry entry;
        entry.score = static_cast<int16_t>(best);
        entry.generation = ttGen_;
        if (best <= alphaOrig) {
            entry.bound = Bound::Upper;
        } else if (best >= betaOrig) {
            entry.bound = Bound::Lower;
        } else {
            entry.bound = Bound::Exact;
        }
        tt_[ttIdx] = entry;
    }

    return best;
}

int AIEngine::countLethalTraps(Board& board, Mark aiMark, int& trapPressure,
                               int& nodesEvaluated) noexcept {
    trapPressure = 0;
    if (board.outcome() != Outcome::Ongoing) {
        return 0;
    }

    const Mark opp = opponentOf(aiMark);
    const uint16_t empty = board.emptyMask();
    int traps = 0;

    // Inspect every possible human response: how many replies hand AI a forced win?
    for (int reply : kMoveOrder) {
        if ((empty & static_cast<uint16_t>(1u << reply)) == 0u) continue;
        (void)board.makeMove(reply, opp);
        const int replyScore = alphaBeta(board, aiMark, aiMark, -kInf, kInf,
                                         Board::kCellCount, true, nodesEvaluated);
        (void)board.undoMove();

        if (replyScore > 0) {
            ++traps;
            trapPressure += 20 + replyScore; // Prefer faster forced wins on human errors
        }
    }

    // Also reward immediate winning threats (forks = 2+ threats).
    const int immediateThreats = board.countImmediateWins(aiMark);
    trapPressure += immediateThreats * 12;
    if (immediateThreats >= 2) {
        trapPressure += 30;
    }
    return traps;
}

std::array<int, Board::kCellCount> AIEngine::evaluatePosition(const Board& board, Mark perspective) {
    std::array<int, Board::kCellCount> scores;
    scores.fill(AIDecision::kNoScore);
    if (perspective == Mark::Empty || board.outcome() != Outcome::Ongoing) {
        return scores;
    }

    ensureCacheFor(perspective);
    Board sim = board;
    int nodes = 0;
    const Mark nextTurn = opponentOf(perspective);

    for (int cell : kMoveOrder) {
        if (!sim.isEmpty(cell)) continue;
        (void)sim.makeMove(cell, perspective);
        scores[static_cast<size_t>(cell)] =
            alphaBeta(sim, perspective, nextTurn, -kInf, kInf, Board::kCellCount, true, nodes);
        (void)sim.undoMove();
    }
    return scores;
}

std::string AIEngine::buildTag(const Board& beforeMove, int chosenCell, Mark aiMark,
                               int score, int trapCount, Difficulty diff) const {
    if (chosenCell < 0) return "No Move";

    Board after = beforeMove;
    (void)after.makeMove(chosenCell, aiMark);
    const Outcome out = after.outcome();

    if ((out == Outcome::XWins && aiMark == Mark::X) ||
        (out == Outcome::OWins && aiMark == Mark::O)) {
        return (diff == Difficulty::Explosive) ? "DETONATION WIN!" : "Checkmate!";
    }
    if (out == Outcome::Draw) {
        return "Stalemate Sealed";
    }

    // Check if this move blocked an immediate opponent win.
    const Mark opp = opponentOf(aiMark);
    Board oppTest = beforeMove;
    (void)oppTest.makeMove(chosenCell, opp);
    const bool blockedWin = oppTest.hasWon(opp);

    const int threats = after.countImmediateWins(aiMark);
    if (threats >= 2) {
        return (diff == Difficulty::Explosive) ? "EXPLOSIVE FORK!" : "Double Fork!";
    }
    if (score > 0 && (diff == Difficulty::Hard || diff == Difficulty::Explosive)) {
        const int movesLeft = std::max(1, (100 - score) - after.moveCount());
        const int turns = (movesLeft + 1) / 2;
        if (turns <= 3) {
            return "Forced Win in " + std::to_string(turns);
        }
        return "Winning Line";
    }
    if (blockedWin) {
        return (diff == Difficulty::Explosive) ? "Threat Neutralized" : "Tactical Block";
    }
    if (diff == Difficulty::Explosive && trapCount >= 3) {
        return "Trap Maximized (" + std::to_string(trapCount) + ")";
    }
    if (diff == Difficulty::Easy) return "Casual Play";
    if (diff == Difficulty::Medium) return "Tactical Probe";
    if (diff == Difficulty::Hard) return "Positional Lock";
    return "Zero-Mercy Lock";
}

AIDecision AIEngine::chooseEasy(const Board& board, Mark aiMark) {
    AIDecision res;
    const auto moves = board.availableMoves();
    if (moves.empty()) return res;

    res.nodesEvaluated = static_cast<int>(moves.size());
    std::uniform_real_distribution<double> prob(0.0, 1.0);

    // 30% chance to spot an immediate winning move.
    const int winCell = findImmediateWin(board, aiMark);
    if (winCell != -1 && prob(rng_) < 0.30) {
        res.cell = winCell;
        res.score = 100 - (board.moveCount() + 1);
        res.tag = buildTag(board, res.cell, aiMark, res.score, 0, Difficulty::Easy);
        return res;
    }

    // 20% chance to block an immediate threat.
    const int blockCell = findImmediateWin(board, opponentOf(aiMark));
    if (blockCell != -1 && prob(rng_) < 0.20) {
        res.cell = blockCell;
        res.score = 0;
        res.tag = buildTag(board, res.cell, aiMark, res.score, 0, Difficulty::Easy);
        return res;
    }

    // Otherwise choose uniformly from available empty cells (O(1), no rejection loop).
    std::uniform_int_distribution<size_t> pick(0, moves.size() - 1);
    res.cell = moves[pick(rng_)];
    res.score = 0;
    res.tag = buildTag(board, res.cell, aiMark, res.score, 0, Difficulty::Easy);
    return res;
}

AIDecision AIEngine::chooseMedium(const Board& board, Mark aiMark) {
    AIDecision res;
    const auto moves = board.availableMoves();
    if (moves.empty()) return res;

    std::uniform_real_distribution<double> prob(0.0, 1.0);

    // 1. Always take an immediate winning move.
    const int winCell = findImmediateWin(board, aiMark);
    if (winCell != -1) {
        res.cell = winCell;
        res.score = 100 - (board.moveCount() + 1);
        res.nodesEvaluated = static_cast<int>(moves.size());
        res.cellScores[static_cast<size_t>(winCell)] = res.score;
        res.tag = buildTag(board, res.cell, aiMark, res.score, 0, Difficulty::Medium);
        return res;
    }

    // 2. 85% chance to block an immediate opponent win.
    const int blockCell = findImmediateWin(board, opponentOf(aiMark));
    if (blockCell != -1 && prob(rng_) < 0.85) {
        res.cell = blockCell;
        res.score = 0;
        res.nodesEvaluated = static_cast<int>(moves.size());
        res.cellScores[static_cast<size_t>(blockCell)] = 0;
        res.tag = buildTag(board, res.cell, aiMark, res.score, 0, Difficulty::Medium);
        return res;
    }

    // 3. Shallow depth-2 Alpha-Beta with heuristic evaluation + controlled variety.
    Board sim = board;
    const Mark opp = opponentOf(aiMark);
    struct ScoredMove {
        int cell;
        int score;
    };
    std::vector<ScoredMove> scored;
    scored.reserve(moves.size());

    for (int cell : kMoveOrder) {
        if (!sim.isEmpty(cell)) continue;
        (void)sim.makeMove(cell, aiMark);
        const int val = alphaBeta(sim, aiMark, opp, -kInf, kInf, 2, false, res.nodesEvaluated);
        (void)sim.undoMove();
        res.cellScores[static_cast<size_t>(cell)] = val;
        scored.push_back({cell, val});
    }

    std::sort(scored.begin(), scored.end(),
              [](const ScoredMove& a, const ScoredMove& b) { return a.score > b.score; });

    // 72% pick among top tier; 28% pick second-best to stay beatable via forks.
    if (scored.size() > 1 && prob(rng_) < 0.28) {
        const size_t idx = std::min<size_t>(scored.size() - 1, (scored.size() > 2 && prob(rng_) < 0.35) ? 2 : 1);
        res.cell = scored[idx].cell;
        res.score = scored[idx].score;
    } else {
        const int bestScore = scored.front().score;
        std::vector<ScoredMove> bestTier;
        for (const auto& sm : scored) {
            if (sm.score == bestScore) bestTier.push_back(sm);
        }
        std::uniform_int_distribution<size_t> pick(0, bestTier.size() - 1);
        const auto chosen = bestTier[pick(rng_)];
        res.cell = chosen.cell;
        res.score = chosen.score;
    }

    res.tag = buildTag(board, res.cell, aiMark, res.score, 0, Difficulty::Medium);
    return res;
}

AIDecision AIEngine::chooseHard(const Board& board, Mark aiMark) {
    AIDecision res;
    const auto moves = board.availableMoves();
    if (moves.empty()) return res;

    ensureCacheFor(aiMark);
    Board sim = board;
    const Mark opp = opponentOf(aiMark);

    struct ScoredMove {
        int cell;
        int score;
        int posWeight;
    };
    std::vector<ScoredMove> scored;
    scored.reserve(moves.size());

    for (int cell : kMoveOrder) {
        if (!sim.isEmpty(cell)) continue;
        (void)sim.makeMove(cell, aiMark);
        const int val = alphaBeta(sim, aiMark, opp, -kInf, kInf, Board::kCellCount, true, res.nodesEvaluated);
        (void)sim.undoMove();
        res.cellScores[static_cast<size_t>(cell)] = val;
        scored.push_back({cell, val, kPositionalWeight[static_cast<size_t>(cell)]});
    }

    std::sort(scored.begin(), scored.end(), [](const ScoredMove& a, const ScoredMove& b) {
        if (a.score != b.score) return a.score > b.score;
        return a.posWeight > b.posWeight;
    });

    // Always take an immediate win or block an immediate threat.
    const int winCell = findImmediateWin(board, aiMark);
    const int blockCell = findImmediateWin(board, opp);
    std::uniform_real_distribution<double> prob(0.0, 1.0);

    // 14% slight human-like opening/midgame imperfection ONLY when the game is still
    // balanced (not already a forced AI win) and there is no immediate threat, so a
    // sharp player can earn a victory with clever forks without Hard throwing won games.
    if (winCell == -1 && blockCell == -1 &&
        scored.front().score <= 0 &&
        board.remainingMoves() >= 5 &&
        scored.size() >= 2 &&
        prob(rng_) < 0.14) {
        // Pick a subtle non-optimal move (preferring the lowest-scoring non-immediate-loss)
        // to occasionally give a skilled human a fork opening.
        res.cell = scored.back().cell;
        res.score = scored.back().score;
    } else {
        const int bestScore = scored.front().score;
        std::vector<ScoredMove> bestTier;
        for (const auto& sm : scored) {
            if (sm.score == bestScore) bestTier.push_back(sm);
        }
        std::uniform_int_distribution<size_t> pick(0, bestTier.size() - 1);
        const auto chosen = bestTier[pick(rng_)];
        res.cell = chosen.cell;
        res.score = chosen.score;
    }

    res.tag = buildTag(board, res.cell, aiMark, res.score, 0, Difficulty::Hard);
    return res;
}

AIDecision AIEngine::chooseExplosive(const Board& board, Mark aiMark) {
    AIDecision res;
    const auto moves = board.availableMoves();
    if (moves.empty()) return res;

    ensureCacheFor(aiMark);
    Board sim = board;
    const Mark opp = opponentOf(aiMark);

    struct ExplosiveCandidate {
        int cell;
        int score;        // Exact minimax value (primary: guarantees 0% loss rate)
        int traps;        // Number of human replies that hand AI a forced win
        int trapPressure; // Weighted sharpness + fork bonus
        int posWeight;
    };

    std::vector<ExplosiveCandidate> candidates;
    candidates.reserve(moves.size());
    int bestMinimax = -kInf;

    for (int cell : kMoveOrder) {
        if (!sim.isEmpty(cell)) continue;
        (void)sim.makeMove(cell, aiMark);
        const int val = alphaBeta(sim, aiMark, opp, -kInf, kInf,
                                  Board::kCellCount, true, res.nodesEvaluated);
        res.cellScores[static_cast<size_t>(cell)] = val;
        if (val > bestMinimax) {
            bestMinimax = val;
        }
        candidates.push_back({cell, val, 0, 0, kPositionalWeight[static_cast<size_t>(cell)]});
        (void)sim.undoMove();
    }

    // Keep only mathematically optimal moves, then maximize trap lethality!
    std::vector<ExplosiveCandidate> optimal;
    for (auto& cand : candidates) {
        if (cand.score != bestMinimax) continue;
        (void)sim.makeMove(cand.cell, aiMark);
        cand.traps = countLethalTraps(sim, aiMark, cand.trapPressure, res.nodesEvaluated);
        (void)sim.undoMove();
        optimal.push_back(cand);
    }

    std::sort(optimal.begin(), optimal.end(),
              [](const ExplosiveCandidate& a, const ExplosiveCandidate& b) {
                  if (a.traps != b.traps) return a.traps > b.traps;
                  if (a.trapPressure != b.trapPressure) return a.trapPressure > b.trapPressure;
                  return a.posWeight > b.posWeight;
              });

    // Randomize only among strictly tied maximum-trap optimal moves (e.g., symmetric corners).
    const auto& top = optimal.front();
    std::vector<ExplosiveCandidate> bestTier;
    for (const auto& c : optimal) {
        if (c.traps == top.traps && c.trapPressure == top.trapPressure) {
            bestTier.push_back(c);
        }
    }

    std::uniform_int_distribution<size_t> pick(0, bestTier.size() - 1);
    const auto chosen = bestTier[pick(rng_)];

    res.cell = chosen.cell;
    res.score = chosen.score;
    res.trapCount = chosen.traps;
    res.tag = buildTag(board, res.cell, aiMark, res.score, res.trapCount, Difficulty::Explosive);
    return res;
}

AIDecision AIEngine::chooseMove(const Board& board, Mark aiMark, Difficulty difficulty) {
    const auto start = std::chrono::steady_clock::now();
    AIDecision decision;

    if (aiMark == Mark::Empty || board.outcome() != Outcome::Ongoing) {
        return decision;
    }

    switch (difficulty) {
        case Difficulty::Easy:
            decision = chooseEasy(board, aiMark);
            break;
        case Difficulty::Medium:
            decision = chooseMedium(board, aiMark);
            break;
        case Difficulty::Hard:
            decision = chooseHard(board, aiMark);
            break;
        case Difficulty::Explosive:
            decision = chooseExplosive(board, aiMark);
            break;
    }

    const auto end = std::chrono::steady_clock::now();
    decision.thinkTimeMs =
        std::chrono::duration<double, std::milli>(end - start).count();
    return decision;
}

} // namespace ttt
