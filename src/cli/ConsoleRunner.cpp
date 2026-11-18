#include "cli/ConsoleRunner.hpp"

#include "engine/GameSession.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace ttt::cli {

namespace {

constexpr const char* kReset   = "\033[0m";
constexpr const char* kBold    = "\033[1m";
constexpr const char* kDim     = "\033[2m";
constexpr const char* kCyan    = "\033[96m";
constexpr const char* kMagenta = "\033[95m";
constexpr const char* kGreen   = "\033[92m";
constexpr const char* kYellow  = "\033[93m";
constexpr const char* kRed     = "\033[91m";

void enableAnsiOnWindows() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode)) {
            SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }
#endif
}

std::optional<std::string> readLine(const std::string& prompt) {
    std::cout << prompt << std::flush;
    std::string line;
    if (!std::getline(std::cin, line)) {
        return std::nullopt;
    }
    return line;
}

std::optional<int> readIntInRange(const std::string& prompt, int minVal, int maxVal) {
    while (true) {
        auto line = readLine(prompt);
        if (!line.has_value()) return std::nullopt;
        try {
            size_t pos = 0;
            const int value = std::stoi(*line, &pos);
            while (pos < line->size() && std::isspace(static_cast<unsigned char>((*line)[pos]))) {
                ++pos;
            }
            if (pos == line->size() && value >= minVal && value <= maxVal) {
                return value;
            }
        } catch (...) {
        }
        std::cout << kRed << "  Please enter a valid number between " << minVal
                  << " and " << maxVal << "." << kReset << '\n';
    }
}

void renderBoard(const Board& board) {
    const auto winLine = board.winningLine();
    auto isWinCell = [&](int cell) {
        if (!winLine.has_value()) return false;
        return std::find(winLine->begin(), winLine->end(), cell) != winLine->end();
    };

    std::cout << "\n";
    for (int r = 0; r < 3; ++r) {
        std::cout << "     ";
        for (int c = 0; c < 3; ++c) {
            const int cell = r * 3 + c;
            const Mark m = board.at(cell);
            if (m == Mark::X) {
                std::cout << kBold << (isWinCell(cell) ? kYellow : kCyan) << " X " << kReset;
            } else if (m == Mark::O) {
                std::cout << kBold << (isWinCell(cell) ? kYellow : kMagenta) << " O " << kReset;
            } else {
                std::cout << kDim << " " << (cell + 1) << " " << kReset;
            }
            if (c < 2) std::cout << kGreen << "│" << kReset;
        }
        std::cout << "\n";
        if (r < 2) {
            std::cout << "     " << kGreen << "───┼───┼───" << kReset << "\n";
        }
    }
    std::cout << "\n";
}

void printScoreboard(const GameSession& session) {
    const auto& sc = session.scoreboard();
    std::cout << kDim << "  Scoreboard: " << kReset
              << kCyan << session.player1DisplayName() << ": " << sc.player1Wins << kReset
              << "  |  " << kYellow << "Draws: " << sc.draws << kReset
              << "  |  " << kMagenta << session.player2DisplayName() << ": " << sc.player2Wins << kReset
              << "\n";
}

} // namespace

int runConsoleApp() {
    enableAnsiOnWindows();

    std::cout << kBold << kCyan
              << "╔══════════════════════════════════════════════╗\n"
              << "║       TIC-TAC-TOE  //  2026 CLI EDITION      ║\n"
              << "║   Bitboard Minimax • 4-Tier AI • Cross-OS    ║\n"
              << "╚══════════════════════════════════════════════╝\n"
              << kReset;

    auto nameOpt = readLine("\nEnter your name (press Enter for Player 1): ");
    if (!nameOpt.has_value()) return 0;
    const std::string player1Name = nameOpt->empty() ? "Player 1" : *nameOpt;

    std::cout << kGreen << "Welcome, " << player1Name << "!" << kReset << "\n";

    GameSession session;

    while (true) {
        std::cout << "\n" << kBold << "========== MAIN MENU ==========" << kReset << "\n"
                  << "  1. Player vs AI (Easy / Medium / Hard / Explosive)\n"
                  << "  2. Player vs Player (Local 2-Player)\n"
                  << "  0. Exit\n";

        auto menuChoice = readIntInRange("Select mode [0-2]: ", 0, 2);
        if (!menuChoice.has_value() || *menuChoice == 0) {
            std::cout << kCyan << "\nThanks for playing Tic-Tac-Toe 2026, " << player1Name
                      << "! Goodbye.\n" << kReset;
            return 0;
        }

        MatchConfig cfg;
        cfg.player1Name = player1Name;
        cfg.mode = (*menuChoice == 1) ? GameMode::PlayerVsAI : GameMode::PlayerVsPlayer;

        if (cfg.mode == GameMode::PlayerVsPlayer) {
            auto p2Opt = readLine("Enter Player 2 name (default: Player 2): ");
            if (!p2Opt.has_value()) return 0;
            cfg.player2Name = p2Opt->empty() ? "Player 2" : *p2Opt;
        } else {
            std::cout << "\n" << kBold << "Select AI Difficulty:" << kReset << "\n"
                      << "  1. Easy      - Casual rookie\n"
                      << "  2. Medium    - Tactical blocks & depth-2 lookahead\n"
                      << "  3. Hard      - Deep Alpha-Beta Minimax\n"
                      << "  4. " << kRed << kBold << "Explosive" << kReset
                      << " - Unbeatable Trap-Maximizer (0% loss rate)\n";
            auto diffOpt = readIntInRange("Choose difficulty [1-4]: ", 1, 4);
            if (!diffOpt.has_value()) return 0;
            cfg.difficulty = static_cast<Difficulty>(*diffOpt - 1);
        }

        std::cout << "\nChoose " << cfg.player1Name << "'s symbol:\n"
                  << "  1. X (moves first)\n"
                  << "  2. O (moves second)\n";
        auto symOpt = readIntInRange("Select symbol [1-2]: ", 1, 2);
        if (!symOpt.has_value()) return 0;
        cfg.player1Mark = (*symOpt == 1) ? Mark::X : Mark::O;

        session.configure(cfg);

        bool playingSeries = true;
        while (playingSeries) {
            while (!session.isGameOver()) {
                renderBoard(session.board());

                if (session.isAITurn()) {
                    const auto dec = session.performAIMove();
                    if (dec.has_value()) {
                        std::cout << (cfg.difficulty == Difficulty::Explosive ? kRed : kMagenta)
                                  << kBold << "  " << session.player2DisplayName()
                                  << " plays cell " << (dec->cell + 1) << kReset
                                  << kDim << "  [" << dec->tag << " • "
                                  << dec->nodesEvaluated << " nodes • "
                                  << std::fixed << std::setprecision(2) << dec->thinkTimeMs
                                  << " ms"
                                  << (cfg.difficulty == Difficulty::Explosive
                                          ? (" • " + std::to_string(dec->trapCount) + " traps")
                                          : "")
                                  << "]" << kReset << "\n";
                    }
                    continue;
                }

                const std::string activeName = session.nameForMark(session.currentTurn());
                const char sym = markToChar(session.currentTurn());
                while (true) {
                    auto raw = readLine("  " + activeName + " (" + sym +
                                        ") — enter cell [1-9] or 'u' to undo: ");
                    if (!raw.has_value()) return 0;
                    if (*raw == "u" || *raw == "U") {
                        if (session.undo()) {
                            std::cout << kYellow << "  Move undone." << kReset << "\n";
                            break;
                        }
                        std::cout << kRed << "  Nothing to undo." << kReset << "\n";
                        continue;
                    }
                    try {
                        size_t pos = 0;
                        const int cell1Based = std::stoi(*raw, &pos);
                        if (pos == raw->size() && cell1Based >= 1 && cell1Based <= 9) {
                            if (session.applyMove(cell1Based - 1)) {
                                break;
                            }
                            std::cout << kRed << "  Cell " << cell1Based
                                      << " is already occupied. Choose another." << kReset << "\n";
                            continue;
                        }
                    } catch (...) {
                    }
                    std::cout << kRed << "  Invalid input. Enter a number from 1 to 9 (or 'u')."
                              << kReset << "\n";
                }
            }

            renderBoard(session.board());
            if (session.outcome() == Outcome::Draw) {
                std::cout << kBold << kYellow << "  >>> It's a DRAW! <<<\n" << kReset;
            } else {
                std::cout << kBold << kGreen << "  >>> " << session.winnerName()
                          << " WINS! <<<\n" << kReset;
            }
            printScoreboard(session);

            std::cout << "\n  1. Rematch   2. Swap X/O & Rematch   0. Main Menu\n";
            auto nextOpt = readIntInRange("Choice [0-2]: ", 0, 2);
            if (!nextOpt.has_value() || *nextOpt == 0) {
                playingSeries = false;
            } else if (*nextOpt == 1) {
                session.newRound();
            } else {
                session.swapSidesAndRestart();
            }
        }
    }
}

} // namespace ttt::cli
