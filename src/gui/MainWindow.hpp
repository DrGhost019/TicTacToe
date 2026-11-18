#pragma once

#include "engine/GameSession.hpp"
#include "gui/BoardWidget.hpp"
#include "gui/Theme.hpp"

#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QProgressBar>
#include <QPushButton>
#include <QStackedWidget>

#include <array>

namespace ttt::gui {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

    // Helper used for automated visual inspection / screenshot verification.
    void setupDemoState(ttt::Difficulty diff, bool showVision);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    ttt::GameSession session_;
    ThemeId currentThemeId_ = ThemeId::CyberNeon;
    ThemePalette theme_ = makeTheme(ThemeId::CyberNeon);

    ttt::GameMode selectedMode_ = ttt::GameMode::PlayerVsAI;
    ttt::Difficulty selectedDifficulty_ = ttt::Difficulty::Medium;
    ttt::Mark selectedMark_ = ttt::Mark::X;

    bool aiThinking_ = false;
    bool aiVisionEnabled_ = false;
    uint64_t roundToken_ = 0;

    QWidget* rootWidget_ = nullptr;
    QStackedWidget* stack_ = nullptr;

    // --- Setup Page Widgets ---
    QWidget* setupPage_ = nullptr;
    QPushButton* modeAiBtn_ = nullptr;
    QPushButton* modePvpBtn_ = nullptr;
    QLineEdit* p1NameEdit_ = nullptr;
    QLabel* p2NameLabel_ = nullptr;
    QLineEdit* p2NameEdit_ = nullptr;
    QPushButton* markXBtn_ = nullptr;
    QPushButton* markOBtn_ = nullptr;
    std::array<QPushButton*, 3> themeBtns_{};
    QStackedWidget* rightSetupStack_ = nullptr;
    std::array<QPushButton*, 4> diffBtns_{};
    QPushButton* startBtn_ = nullptr;

    // --- Arena Page Widgets ---
    QWidget* arenaPage_ = nullptr;
    QPushButton* backBtn_ = nullptr;
    QLabel* modeBadgeLabel_ = nullptr;
    QPushButton* visionBtn_ = nullptr;
    QPushButton* themeCycleBtn_ = nullptr;

    BoardWidget* boardWidget_ = nullptr;

    QFrame* statusCard_ = nullptr;
    QLabel* statusTitleLabel_ = nullptr;
    QLabel* statusSubLabel_ = nullptr;

    QLabel* p1ScoreNameLabel_ = nullptr;
    QLabel* p1ScoreValLabel_ = nullptr;
    QLabel* drawScoreValLabel_ = nullptr;
    QLabel* p2ScoreNameLabel_ = nullptr;
    QLabel* p2ScoreValLabel_ = nullptr;
    QLabel* roundInfoLabel_ = nullptr;

    QFrame* telemetryCard_ = nullptr;
    QLabel* telemetryTagLabel_ = nullptr;
    QProgressBar* evalBar_ = nullptr;
    QLabel* nodesValLabel_ = nullptr;
    QLabel* timeValLabel_ = nullptr;
    QLabel* trapsValLabel_ = nullptr;

    QPushButton* newRoundBtn_ = nullptr;
    QPushButton* undoBtn_ = nullptr;
    QPushButton* swapBtn_ = nullptr;
    QPushButton* resetScoreBtn_ = nullptr;

    void buildUi();
    QWidget* createSetupPage();
    QWidget* createArenaPage();

    void applyTheme();
    void refreshSetupSelections();
    void startMatchFromSetup();
    void returnToSetup();

    void handleHumanCellInput(int cell);
    void scheduleAIMoveIfNeeded();
    void executeAIMove(uint64_t expectedToken);
    void handleNewRound();
    void handleUndo();
    void handleSwapSides();
    void handleResetScores();
    void toggleAIVision();
    void cycleTheme();

    void refreshArenaUI();
    [[nodiscard]] bool isExplosiveActive() const noexcept;
    static void repolish(QWidget* w);
};

} // namespace ttt::gui
