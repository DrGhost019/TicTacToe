#include "cli/ConsoleRunner.hpp"

#ifndef TTT_CLI_ONLY
#include "gui/MainWindow.hpp"
#include <QApplication>
#include <QDir>
#include <QTimer>
#endif

#include <iostream>
#include <string>
#include <string_view>

namespace {

void printHelp(const char* argv0) {
    std::cout << "Tic-Tac-Toe 2026 Edition\n"
              << "Usage: " << argv0 << " [options]\n\n"
              << "Options:\n"
              << "  --cli                  Run in cross-platform terminal mode\n"
              << "  --help, -h             Show this help message\n";
}

} // namespace

int main(int argc, char* argv[]) {
    bool forceCli = false;
    std::string screenshotDir;

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg(argv[i]);
        if (arg == "--cli") {
            forceCli = true;
        } else if (arg == "--help" || arg == "-h") {
            printHelp(argv[0]);
            return 0;
        } else if (arg == "--capture-screenshots" && i + 1 < argc) {
            screenshotDir = argv[++i];
        }
    }

#ifdef TTT_CLI_ONLY
    (void)forceCli;
    (void)screenshotDir;
    return ttt::cli::runConsoleApp();
#else
    if (forceCli) {
        return ttt::cli::runConsoleApp();
    }

    QApplication app(argc, argv);
    app.setApplicationName("Tic-Tac-Toe 2026");
    app.setApplicationVersion("2.0.0");

    ttt::gui::MainWindow window;
    window.show();

    if (!screenshotDir.empty()) {
        QDir().mkpath(QString::fromStdString(screenshotDir));
        const QString dir = QString::fromStdString(screenshotDir);

        QTimer::singleShot(120, &window, [&window, &app, dir]() {
            window.grab().save(dir + "/setup_screen.png");

            // 1. Cyber Neon + Explosive Overdrive + AI Vision
            window.setupDemoState(ttt::gui::ThemeId::CyberNeon, ttt::Difficulty::Explosive, true, false);
            QTimer::singleShot(160, &window, [&window, &app, dir]() {
                window.grab().save(dir + "/arena_explosive.png");

                // 2. Cyber Neon Theme (Hard AI + Winning Laser Beam)
                window.setupDemoState(ttt::gui::ThemeId::CyberNeon, ttt::Difficulty::Hard, false, true);
                QTimer::singleShot(420, &window, [&window, &app, dir]() {
                    window.grab().save(dir + "/theme_cyber_neon.png");

                    // 3. Obsidian Gold Theme (Hard AI + AI Vision)
                    window.setupDemoState(ttt::gui::ThemeId::ObsidianGold, ttt::Difficulty::Hard, true, false);
                    QTimer::singleShot(180, &window, [&window, &app, dir]() {
                        window.grab().save(dir + "/theme_obsidian_gold.png");

                        // 4. Retro 2024 Terminal Tribute Theme (Medium AI + Winning Laser Beam)
                        window.setupDemoState(ttt::gui::ThemeId::Retro2024, ttt::Difficulty::Medium, false, true);
                        QTimer::singleShot(420, &window, [&window, &app, dir]() {
                            window.grab().save(dir + "/theme_retro_2024.png");
                            app.quit();
                        });
                    });
                });
            });
        });
    }

    return app.exec();
#endif
}
