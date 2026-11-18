#include "gui/MainWindow.hpp"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

namespace ttt::gui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle("Tic-Tac-Toe // 2026 Edition");
    resize(980, 720);
    setMinimumSize(820, 640);

    buildUi();
    refreshSetupSelections();
    applyTheme();
}

void MainWindow::repolish(QWidget* w) {
    if (!w) return;
    w->style()->unpolish(w);
    w->style()->polish(w);
    w->update();
}

bool MainWindow::isExplosiveActive() const noexcept {
    if (stack_ && stack_->currentIndex() == 1) {
        return session_.config().mode == ttt::GameMode::PlayerVsAI &&
               session_.config().difficulty == ttt::Difficulty::Explosive;
    }
    return selectedMode_ == ttt::GameMode::PlayerVsAI &&
           selectedDifficulty_ == ttt::Difficulty::Explosive;
}

void MainWindow::buildUi() {
    rootWidget_ = new QWidget(this);
    rootWidget_->setObjectName("RootWidget");
    setCentralWidget(rootWidget_);

    auto* rootLayout = new QVBoxLayout(rootWidget_);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    stack_ = new QStackedWidget(rootWidget_);
    setupPage_ = createSetupPage();
    arenaPage_ = createArenaPage();

    stack_->addWidget(setupPage_);
    stack_->addWidget(arenaPage_);
    stack_->setCurrentIndex(0);

    rootLayout->addWidget(stack_);
}

QWidget* MainWindow::createSetupPage() {
    auto* page = new QWidget(this);
    auto* outer = new QVBoxLayout(page);
    outer->setContentsMargins(36, 26, 36, 26);
    outer->setSpacing(18);

    // --- Hero Banner ---
    auto* heroBox = new QVBoxLayout();
    heroBox->setSpacing(6);
    heroBox->setAlignment(Qt::AlignHCenter);

    auto* badge = new QLabel("2024  ->  2026 REMASTERED EDITION", page);
    badge->setObjectName("HeroBadge");
    badge->setAlignment(Qt::AlignCenter);
    badge->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    auto* badgeWrap = new QHBoxLayout();
    badgeWrap->setAlignment(Qt::AlignCenter);
    badgeWrap->addWidget(badge);

    auto* title = new QLabel("TIC-TAC-TOE", page);
    title->setObjectName("HeroTitle");
    title->setAlignment(Qt::AlignCenter);

    auto* subtitle = new QLabel("Bitboard Minimax Engine  •  4-Tier Adaptive AI  •  60 FPS Neon Arena", page);
    subtitle->setObjectName("SecondaryLabel");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("font-size: 13px;");

    heroBox->addLayout(badgeWrap);
    heroBox->addWidget(title);
    heroBox->addWidget(subtitle);
    outer->addLayout(heroBox);

    // --- Center Two-Column Setup Area ---
    auto* columns = new QHBoxLayout();
    columns->setSpacing(20);

    // LEFT CARD: Match Config
    auto* leftCard = new QFrame(page);
    leftCard->setObjectName("Card");
    auto* leftLay = new QVBoxLayout(leftCard);
    leftLay->setContentsMargins(22, 20, 22, 20);
    leftLay->setSpacing(14);

    auto* modeHeading = new QLabel("GAME MODE", leftCard);
    modeHeading->setObjectName("SectionTitle");
    leftLay->addWidget(modeHeading);

    auto* modeRow = new QHBoxLayout();
    modeRow->setSpacing(10);
    modeAiBtn_ = new QPushButton("Player vs AI", leftCard);
    modePvpBtn_ = new QPushButton("Local 2-Player", leftCard);
    modeAiBtn_->setCursor(Qt::PointingHandCursor);
    modePvpBtn_->setCursor(Qt::PointingHandCursor);
    modeAiBtn_->setMinimumHeight(42);
    modePvpBtn_->setMinimumHeight(42);
    modeRow->addWidget(modeAiBtn_);
    modeRow->addWidget(modePvpBtn_);
    leftLay->addLayout(modeRow);

    auto* playersHeading = new QLabel("PLAYER PROFILES", leftCard);
    playersHeading->setObjectName("SectionTitle");
    leftLay->addWidget(playersHeading);

    auto* p1Label = new QLabel("Player 1 Name", leftCard);
    p1Label->setObjectName("SecondaryLabel");
    p1Label->setStyleSheet("font-size: 12px; font-weight: 600;");
    p1NameEdit_ = new QLineEdit(leftCard);
    p1NameEdit_->setPlaceholderText("Enter your name (default: Player 1)");
    p1NameEdit_->setMaxLength(20);

    p2NameLabel_ = new QLabel("Player 2 Name", leftCard);
    p2NameLabel_->setObjectName("SecondaryLabel");
    p2NameLabel_->setStyleSheet("font-size: 12px; font-weight: 600;");
    p2NameEdit_ = new QLineEdit(leftCard);
    p2NameEdit_->setPlaceholderText("Enter opponent name (default: Player 2)");
    p2NameEdit_->setMaxLength(20);

    leftLay->addWidget(p1Label);
    leftLay->addWidget(p1NameEdit_);
    leftLay->addWidget(p2NameLabel_);
    leftLay->addWidget(p2NameEdit_);

    auto* symHeading = new QLabel("PLAYER 1 SYMBOL", leftCard);
    symHeading->setObjectName("SectionTitle");
    leftLay->addWidget(symHeading);

    auto* symRow = new QHBoxLayout();
    symRow->setSpacing(10);
    markXBtn_ = new QPushButton("Play as X  (First)", leftCard);
    markOBtn_ = new QPushButton("Play as O  (Second)", leftCard);
    markXBtn_->setCursor(Qt::PointingHandCursor);
    markOBtn_->setCursor(Qt::PointingHandCursor);
    markXBtn_->setMinimumHeight(40);
    markOBtn_->setMinimumHeight(40);
    symRow->addWidget(markXBtn_);
    symRow->addWidget(markOBtn_);
    leftLay->addLayout(symRow);

    auto* themeHeading = new QLabel("VISUAL THEME", leftCard);
    themeHeading->setObjectName("SectionTitle");
    leftLay->addWidget(themeHeading);

    auto* themeRow = new QHBoxLayout();
    themeRow->setSpacing(8);
    const std::array<QString, 3> themeNames = {"Cyber Neon", "Obsidian Gold", "Retro 2024"};
    for (int i = 0; i < 3; ++i) {
        auto* btn = new QPushButton(themeNames[static_cast<size_t>(i)], leftCard);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setMinimumHeight(36);
        themeBtns_[static_cast<size_t>(i)] = btn;
        themeRow->addWidget(btn);
        connect(btn, &QPushButton::clicked, this, [this, i]() {
            currentThemeId_ = static_cast<ThemeId>(i);
            theme_ = makeTheme(currentThemeId_);
            refreshSetupSelections();
            applyTheme();
        });
    }
    leftLay->addLayout(themeRow);
    leftLay->addStretch();

    // RIGHT CARD: AI Difficulty Selector OR PvP Info Panel
    rightSetupStack_ = new QStackedWidget(page);

    // Stack 0: AI Difficulty Card
    auto* diffCard = new QFrame(page);
    diffCard->setObjectName("Card");
    auto* diffLay = new QVBoxLayout(diffCard);
    diffLay->setContentsMargins(22, 20, 22, 20);
    diffLay->setSpacing(11);

    auto* diffHeading = new QLabel("AI DIFFICULTY TIER", diffCard);
    diffHeading->setObjectName("SectionTitle");
    diffLay->addWidget(diffHeading);

    struct DiffMeta {
        ttt::Difficulty diff;
        QString title;
        QString desc;
    };
    const std::array<DiffMeta, 4> metas = {{
        {ttt::Difficulty::Easy,      "EASY  •  Rookie",
         "Relaxed random play with light win/block instincts"},
        {ttt::Difficulty::Medium,    "MEDIUM  •  Tactician",
         "Spots wins, blocks most threats, depth-2 lookahead"},
        {ttt::Difficulty::Hard,      "HARD  •  Grandmaster",
         "Deep Alpha-Beta Minimax with fork defense"},
        {ttt::Difficulty::Explosive, "EXPLOSIVE  •  UNBEATABLE",
         "0% loss rate Trap-Maximizer + fiery shockwave FX"}
    }};

    for (int i = 0; i < 4; ++i) {
        auto* btn = new QPushButton(metas[static_cast<size_t>(i)].title + "\n" +
                                        metas[static_cast<size_t>(i)].desc,
                                    diffCard);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setMinimumHeight(64);
        btn->setStyleSheet("text-align: left; padding: 10px 14px; line-height: 1.3;");
        diffBtns_[static_cast<size_t>(i)] = btn;
        diffLay->addWidget(btn);

        const auto d = metas[static_cast<size_t>(i)].diff;
        connect(btn, &QPushButton::clicked, this, [this, d]() {
            selectedDifficulty_ = d;
            refreshSetupSelections();
            applyTheme();
        });
    }
    diffLay->addStretch();

    // Stack 1: Local 2-Player Info Card
    auto* pvpInfoCard = new QFrame(page);
    pvpInfoCard->setObjectName("Card");
    auto* pvpLay = new QVBoxLayout(pvpInfoCard);
    pvpLay->setContentsMargins(24, 24, 24, 24);
    pvpLay->setSpacing(14);

    auto* pvpHead = new QLabel("LOCAL 2-PLAYER ARENA", pvpInfoCard);
    pvpHead->setObjectName("SectionTitle");
    pvpLay->addWidget(pvpHead);

    auto* pvpTitle = new QLabel("Head-to-Head Duel", pvpInfoCard);
    pvpTitle->setStyleSheet("font-size: 22px; font-weight: 800;");
    pvpLay->addWidget(pvpTitle);

    auto* pvpDesc = new QLabel(
        "Challenge a friend on the same device with animated neon visuals, "
        "live win streaks, side swapping, and instant move undo.\n\n"
        "•  Click any square or press keys 1–9 to place your mark\n"
        "•  X always takes the opening turn\n"
        "•  Press Z to undo or R for an instant rematch\n"
        "•  Swap X / O anytime between rounds",
        pvpInfoCard);
    pvpDesc->setObjectName("SecondaryLabel");
    pvpDesc->setWordWrap(true);
    pvpDesc->setStyleSheet("font-size: 13px; line-height: 1.5;");
    pvpLay->addWidget(pvpDesc);
    pvpLay->addStretch();

    rightSetupStack_->addWidget(diffCard);
    rightSetupStack_->addWidget(pvpInfoCard);

    columns->addWidget(leftCard, 1);
    columns->addWidget(rightSetupStack_, 1);
    outer->addLayout(columns, 1);

    // --- Launch Button ---
    startBtn_ = new QPushButton("START MATCH  ->", page);
    startBtn_->setObjectName("PrimaryButton");
    startBtn_->setCursor(Qt::PointingHandCursor);
    startBtn_->setMinimumHeight(52);
    outer->addWidget(startBtn_);

    // Connections
    connect(modeAiBtn_, &QPushButton::clicked, this, [this]() {
        selectedMode_ = ttt::GameMode::PlayerVsAI;
        refreshSetupSelections();
        applyTheme();
    });
    connect(modePvpBtn_, &QPushButton::clicked, this, [this]() {
        selectedMode_ = ttt::GameMode::PlayerVsPlayer;
        refreshSetupSelections();
        applyTheme();
    });
    connect(markXBtn_, &QPushButton::clicked, this, [this]() {
        selectedMark_ = ttt::Mark::X;
        refreshSetupSelections();
    });
    connect(markOBtn_, &QPushButton::clicked, this, [this]() {
        selectedMark_ = ttt::Mark::O;
        refreshSetupSelections();
    });
    connect(startBtn_, &QPushButton::clicked, this, &MainWindow::startMatchFromSetup);

    return page;
}

QWidget* MainWindow::createArenaPage() {
    auto* page = new QWidget(this);
    auto* outer = new QVBoxLayout(page);
    outer->setContentsMargins(22, 16, 22, 16);
    outer->setSpacing(14);

    // --- Top Bar ---
    auto* topBar = new QHBoxLayout();
    topBar->setSpacing(10);

    backBtn_ = new QPushButton("<- Setup", page);
    backBtn_->setCursor(Qt::PointingHandCursor);

    modeBadgeLabel_ = new QLabel("VS AI  •  EXPLOSIVE", page);
    modeBadgeLabel_->setObjectName("HeroBadge");

    visionBtn_ = new QPushButton("AI Vision: OFF", page);
    visionBtn_->setObjectName("TogglePill");
    visionBtn_->setCursor(Qt::PointingHandCursor);

    themeCycleBtn_ = new QPushButton("Theme: Cyber Neon", page);
    themeCycleBtn_->setCursor(Qt::PointingHandCursor);

    topBar->addWidget(backBtn_);
    topBar->addWidget(modeBadgeLabel_);
    topBar->addStretch();
    topBar->addWidget(visionBtn_);
    topBar->addWidget(themeCycleBtn_);
    outer->addLayout(topBar);

    // --- Main Arena Split ---
    auto* body = new QHBoxLayout();
    body->setSpacing(18);

    // Centerpiece Board
    boardWidget_ = new BoardWidget(page);
    body->addWidget(boardWidget_, 3);

    // Right Sidebar HUD
    auto* sidebar = new QVBoxLayout();
    sidebar->setSpacing(12);

    // 1. Live Status Card
    statusCard_ = new QFrame(page);
    statusCard_->setObjectName("HudCard");
    statusCard_->setFixedWidth(320);
    auto* statusLay = new QVBoxLayout(statusCard_);
    statusLay->setContentsMargins(16, 14, 16, 14);
    statusLay->setSpacing(4);

    auto* statusCap = new QLabel("MATCH STATUS", statusCard_);
    statusCap->setObjectName("SectionTitle");
    statusTitleLabel_ = new QLabel("Your Turn (X)", statusCard_);
    statusTitleLabel_->setStyleSheet("font-size: 20px; font-weight: 800;");
    statusTitleLabel_->setWordWrap(true);
    statusSubLabel_ = new QLabel("Select a tile or press keys 1-9", statusCard_);
    statusSubLabel_->setObjectName("SecondaryLabel");
    statusSubLabel_->setStyleSheet("font-size: 12px;");
    statusSubLabel_->setWordWrap(true);

    statusLay->addWidget(statusCap);
    statusLay->addWidget(statusTitleLabel_);
    statusLay->addWidget(statusSubLabel_);
    sidebar->addWidget(statusCard_);

    // 2. Scoreboard Card
    auto* scoreCard = new QFrame(page);
    scoreCard->setObjectName("HudCard");
    scoreCard->setFixedWidth(320);
    auto* scoreLay = new QVBoxLayout(scoreCard);
    scoreLay->setContentsMargins(16, 14, 16, 14);
    scoreLay->setSpacing(10);

    auto* scoreHeaderRow = new QHBoxLayout();
    auto* scoreCap = new QLabel("SCOREBOARD", scoreCard);
    scoreCap->setObjectName("SectionTitle");
    roundInfoLabel_ = new QLabel("Round 1", scoreCard);
    roundInfoLabel_->setObjectName("MutedLabel");
    roundInfoLabel_->setStyleSheet("font-size: 11px; font-weight: 700;");
    scoreHeaderRow->addWidget(scoreCap);
    scoreHeaderRow->addStretch();
    scoreHeaderRow->addWidget(roundInfoLabel_);
    scoreLay->addLayout(scoreHeaderRow);

    auto* scoreGrid = new QGridLayout();
    scoreGrid->setSpacing(8);

    auto makeStatBox = [&](QLabel*& nameOut, QLabel*& valOut, const QString& defaultName) {
        auto* box = new QFrame(scoreCard);
        box->setStyleSheet("background-color: rgba(255,255,255,0.03); border-radius: 10px;");
        auto* bLay = new QVBoxLayout(box);
        bLay->setContentsMargins(6, 8, 6, 8);
        bLay->setSpacing(2);
        nameOut = new QLabel(defaultName, box);
        nameOut->setAlignment(Qt::AlignCenter);
        nameOut->setObjectName("SecondaryLabel");
        nameOut->setStyleSheet("font-size: 11px; font-weight: 700;");
        valOut = new QLabel("0", box);
        valOut->setAlignment(Qt::AlignCenter);
        valOut->setStyleSheet("font-size: 24px; font-weight: 800;");
        bLay->addWidget(nameOut);
        bLay->addWidget(valOut);
        return box;
    };

    QLabel* drawNameLbl = nullptr;
    scoreGrid->addWidget(makeStatBox(p1ScoreNameLabel_, p1ScoreValLabel_, "Player 1"), 0, 0);
    scoreGrid->addWidget(makeStatBox(drawNameLbl, drawScoreValLabel_, "DRAWS"), 0, 1);
    scoreGrid->addWidget(makeStatBox(p2ScoreNameLabel_, p2ScoreValLabel_, "AI"), 0, 2);
    scoreLay->addLayout(scoreGrid);
    sidebar->addWidget(scoreCard);

    // 3. AI Telemetry Card
    telemetryCard_ = new QFrame(page);
    telemetryCard_->setObjectName("HudCard");
    telemetryCard_->setFixedWidth(320);
    auto* telLay = new QVBoxLayout(telemetryCard_);
    telLay->setContentsMargins(16, 14, 16, 14);
    telLay->setSpacing(8);

    auto* telCap = new QLabel("AI ENGINE TELEMETRY", telemetryCard_);
    telCap->setObjectName("SectionTitle");
    telemetryTagLabel_ = new QLabel("STANDBY", telemetryCard_);
    telemetryTagLabel_->setStyleSheet("font-size: 12px; font-weight: 800; color: #38BDF8;");
    telLay->addWidget(telCap);
    telLay->addWidget(telemetryTagLabel_);

    evalBar_ = new QProgressBar(telemetryCard_);
    evalBar_->setObjectName("EvalBar");
    evalBar_->setRange(-100, 100);
    evalBar_->setValue(0);
    evalBar_->setTextVisible(false);
    telLay->addWidget(evalBar_);

    auto* telGrid = new QGridLayout();
    telGrid->setSpacing(6);

    auto addMetric = [&](int col, const QString& label, QLabel*& valLbl) {
        auto* cap = new QLabel(label, telemetryCard_);
        cap->setObjectName("MutedLabel");
        cap->setAlignment(Qt::AlignCenter);
        cap->setStyleSheet("font-size: 10px; font-weight: 700;");
        valLbl = new QLabel("-", telemetryCard_);
        valLbl->setAlignment(Qt::AlignCenter);
        valLbl->setStyleSheet("font-size: 13px; font-weight: 700;");
        telGrid->addWidget(cap, 0, col);
        telGrid->addWidget(valLbl, 1, col);
    };

    addMetric(0, "NODES", nodesValLabel_);
    addMetric(1, "THINK TIME", timeValLabel_);
    addMetric(2, "TRAPS SET", trapsValLabel_);
    telLay->addLayout(telGrid);
    sidebar->addWidget(telemetryCard_);

    // 4. Controls Card
    auto* ctrlCard = new QFrame(page);
    ctrlCard->setObjectName("HudCard");
    ctrlCard->setFixedWidth(320);
    auto* ctrlLay = new QVBoxLayout(ctrlCard);
    ctrlLay->setContentsMargins(16, 14, 16, 14);
    ctrlLay->setSpacing(9);

    newRoundBtn_ = new QPushButton("New Round  (R)", ctrlCard);
    newRoundBtn_->setObjectName("PrimaryButton");
    newRoundBtn_->setCursor(Qt::PointingHandCursor);
    ctrlLay->addWidget(newRoundBtn_);

    auto* row2 = new QHBoxLayout();
    row2->setSpacing(8);
    undoBtn_ = new QPushButton("Undo  (Z)", ctrlCard);
    swapBtn_ = new QPushButton("Swap X / O", ctrlCard);
    undoBtn_->setCursor(Qt::PointingHandCursor);
    swapBtn_->setCursor(Qt::PointingHandCursor);
    row2->addWidget(undoBtn_);
    row2->addWidget(swapBtn_);
    ctrlLay->addLayout(row2);

    resetScoreBtn_ = new QPushButton("Reset Scoreboard", ctrlCard);
    resetScoreBtn_->setCursor(Qt::PointingHandCursor);
    ctrlLay->addWidget(resetScoreBtn_);

    sidebar->addWidget(ctrlCard);

    auto* shortcutHint = new QLabel("Keys 1-9 Play  •  Z Undo  •  R Rematch  •  Esc Menu", page);
    shortcutHint->setObjectName("MutedLabel");
    shortcutHint->setAlignment(Qt::AlignCenter);
    shortcutHint->setStyleSheet("font-size: 11px;");
    shortcutHint->setFixedWidth(320);
    sidebar->addWidget(shortcutHint);
    sidebar->addStretch();

    body->addLayout(sidebar);
    outer->addLayout(body, 1);

    // Connections
    connect(backBtn_, &QPushButton::clicked, this, &MainWindow::returnToSetup);
    connect(visionBtn_, &QPushButton::clicked, this, &MainWindow::toggleAIVision);
    connect(themeCycleBtn_, &QPushButton::clicked, this, &MainWindow::cycleTheme);
    connect(boardWidget_, &BoardWidget::cellClicked, this, &MainWindow::handleHumanCellInput);
    connect(newRoundBtn_, &QPushButton::clicked, this, &MainWindow::handleNewRound);
    connect(undoBtn_, &QPushButton::clicked, this, &MainWindow::handleUndo);
    connect(swapBtn_, &QPushButton::clicked, this, &MainWindow::handleSwapSides);
    connect(resetScoreBtn_, &QPushButton::clicked, this, &MainWindow::handleResetScores);

    return page;
}

void MainWindow::applyTheme() {
    const bool explosive = isExplosiveActive();
    setStyleSheet(buildAppStylesheet(theme_, explosive));

    if (boardWidget_) {
        boardWidget_->setTheme(theme_);
        boardWidget_->setExplosiveMode(explosive);
    }

    if (startBtn_) {
        startBtn_->setObjectName(explosive ? "ExplosivePrimaryButton" : "PrimaryButton");
        startBtn_->setText(explosive ? "LAUNCH EXPLOSIVE ARENA  ->" : "START MATCH  ->");
        repolish(startBtn_);
    }

    if (newRoundBtn_) {
        newRoundBtn_->setObjectName(explosive ? "ExplosivePrimaryButton" : "PrimaryButton");
        repolish(newRoundBtn_);
    }

    if (statusCard_) {
        statusCard_->setObjectName(explosive ? "ExplosiveHudCard" : "HudCard");
        repolish(statusCard_);
    }

    if (themeCycleBtn_) {
        themeCycleBtn_->setText("Theme: " + theme_.name);
    }
}

void MainWindow::refreshSetupSelections() {
    const bool isAI = (selectedMode_ == ttt::GameMode::PlayerVsAI);
    modeAiBtn_->setProperty("selected", isAI);
    modePvpBtn_->setProperty("selected", !isAI);
    repolish(modeAiBtn_);
    repolish(modePvpBtn_);

    p2NameLabel_->setVisible(!isAI);
    p2NameEdit_->setVisible(!isAI);
    rightSetupStack_->setCurrentIndex(isAI ? 0 : 1);

    markXBtn_->setProperty("selected", selectedMark_ == ttt::Mark::X);
    markOBtn_->setProperty("selected", selectedMark_ == ttt::Mark::O);
    repolish(markXBtn_);
    repolish(markOBtn_);

    for (int i = 0; i < 3; ++i) {
        themeBtns_[static_cast<size_t>(i)]->setProperty(
            "selected", static_cast<ThemeId>(i) == currentThemeId_);
        repolish(themeBtns_[static_cast<size_t>(i)]);
    }

    for (int i = 0; i < 4; ++i) {
        auto* btn = diffBtns_[static_cast<size_t>(i)];
        const bool sel = (static_cast<ttt::Difficulty>(i) == selectedDifficulty_);
        btn->setProperty("selected", sel);
        if (i == static_cast<int>(ttt::Difficulty::Explosive)) {
            btn->setStyleSheet(sel
                ? "text-align: left; padding: 10px 14px; border: 2px solid #FF5522; "
                  "background-color: rgba(255, 59, 48, 0.16); color: #FFE4D6;"
                : "text-align: left; padding: 10px 14px; border: 1px solid rgba(255, 85, 34, 0.45);");
        } else {
            btn->setStyleSheet("text-align: left; padding: 10px 14px;");
        }
        repolish(btn);
    }
}

void MainWindow::startMatchFromSetup() {
    ++roundToken_;
    aiThinking_ = false;

    ttt::MatchConfig cfg;
    cfg.mode = selectedMode_;
    cfg.difficulty = selectedDifficulty_;
    cfg.player1Name = p1NameEdit_->text().toStdString();
    cfg.player2Name = p2NameEdit_->text().toStdString();
    cfg.player1Mark = selectedMark_;

    session_.configure(cfg);
    boardWidget_->clearAnimations();

    stack_->setCurrentIndex(1);
    applyTheme();
    refreshArenaUI();
    scheduleAIMoveIfNeeded();
}

void MainWindow::returnToSetup() {
    ++roundToken_;
    aiThinking_ = false;
    stack_->setCurrentIndex(0);
    refreshSetupSelections();
    applyTheme();
}

void MainWindow::handleHumanCellInput(int cell) {
    if (aiThinking_ || session_.isGameOver() || session_.isAITurn()) {
        return;
    }

    const ttt::Mark movingMark = session_.currentTurn();
    if (!session_.applyMove(cell)) {
        return;
    }

    boardWidget_->triggerMoveAnimation(cell, movingMark, false);

    if (session_.isGameOver()) {
        const auto out = session_.outcome();
        if (out == ttt::Outcome::Draw) {
            boardWidget_->triggerDrawAnimation();
        } else if (const auto wl = session_.board().winningLine(); wl.has_value()) {
            const ttt::Mark winner = (out == ttt::Outcome::XWins) ? ttt::Mark::X : ttt::Mark::O;
            boardWidget_->triggerWinAnimation(*wl, winner, false);
        }
    }

    refreshArenaUI();
    scheduleAIMoveIfNeeded();
}

void MainWindow::scheduleAIMoveIfNeeded() {
    if (!session_.isAITurn()) return;

    aiThinking_ = true;
    refreshArenaUI();

    const uint64_t token = roundToken_;
    const int delayMs = (session_.config().difficulty == ttt::Difficulty::Explosive) ? 380 : 300;
    QTimer::singleShot(delayMs, this, [this, token]() {
        executeAIMove(token);
    });
}

void MainWindow::executeAIMove(uint64_t expectedToken) {
    if (expectedToken != roundToken_) return;
    aiThinking_ = false;

    if (!session_.isAITurn()) {
        refreshArenaUI();
        return;
    }

    const ttt::Mark aiMark = session_.aiMark();
    const bool explosive = (session_.config().difficulty == ttt::Difficulty::Explosive);
    const auto decision = session_.performAIMove();

    if (decision.has_value() && decision->cell >= 0) {
        boardWidget_->triggerMoveAnimation(decision->cell, aiMark, explosive);

        if (session_.isGameOver()) {
            const auto out = session_.outcome();
            if (out == ttt::Outcome::Draw) {
                boardWidget_->triggerDrawAnimation();
            } else if (const auto wl = session_.board().winningLine(); wl.has_value()) {
                const ttt::Mark winner = (out == ttt::Outcome::XWins) ? ttt::Mark::X : ttt::Mark::O;
                const bool explosiveWin = explosive && (winner == aiMark);
                boardWidget_->triggerWinAnimation(*wl, winner, explosiveWin);
            }
        }
    }

    refreshArenaUI();
}

void MainWindow::handleNewRound() {
    ++roundToken_;
    aiThinking_ = false;
    session_.newRound();
    boardWidget_->clearAnimations();
    refreshArenaUI();
    scheduleAIMoveIfNeeded();
}

void MainWindow::handleUndo() {
    if (!session_.canUndo()) return;
    ++roundToken_;
    aiThinking_ = false;
    if (session_.undo()) {
        boardWidget_->clearAnimations();
        refreshArenaUI();
    }
}

void MainWindow::handleSwapSides() {
    ++roundToken_;
    aiThinking_ = false;
    session_.swapSidesAndRestart();
    boardWidget_->clearAnimations();
    refreshArenaUI();
    scheduleAIMoveIfNeeded();
}

void MainWindow::handleResetScores() {
    session_.resetScores();
    refreshArenaUI();
}

void MainWindow::toggleAIVision() {
    aiVisionEnabled_ = !aiVisionEnabled_;
    refreshArenaUI();
}

void MainWindow::cycleTheme() {
    const int next = (static_cast<int>(currentThemeId_) + 1) % 3;
    currentThemeId_ = static_cast<ThemeId>(next);
    theme_ = makeTheme(currentThemeId_);
    applyTheme();
    refreshArenaUI();
}

void MainWindow::refreshArenaUI() {
    const auto& cfg = session_.config();
    const bool isAI = (cfg.mode == ttt::GameMode::PlayerVsAI);
    const bool explosive = isExplosiveActive();

    // Header badges & toggles
    if (isAI) {
        const QString diffUpper = QString::fromUtf8(
            ttt::difficultyName(cfg.difficulty).data(),
            static_cast<int>(ttt::difficultyName(cfg.difficulty).size())).toUpper();
        modeBadgeLabel_->setText("VS AI  •  " + diffUpper);
    } else {
        modeBadgeLabel_->setText("LOCAL 2-PLAYER DUEL");
    }

    visionBtn_->setVisible(isAI);
    visionBtn_->setText(aiVisionEnabled_ ? "AI Vision: ON" : "AI Vision: OFF");
    visionBtn_->setProperty("active", aiVisionEnabled_);
    repolish(visionBtn_);

    telemetryCard_->setVisible(isAI);

    // Sync board + optional AI Vision evaluation heatmap
    std::array<int, 9> evals;
    evals.fill(ttt::AIDecision::kNoScore);
    const bool humanCanInput = !aiThinking_ && !session_.isGameOver() && !session_.isAITurn();
    if (isAI && aiVisionEnabled_ && humanCanInput) {
        evals = session_.evaluateCurrentPosition();
    }
    boardWidget_->syncBoard(session_.board(), session_.currentTurn(),
                            humanCanInput, isAI && aiVisionEnabled_, evals);

    // Status Card
    const auto out = session_.outcome();
    const auto& sc = session_.scoreboard();
    if (out == ttt::Outcome::Ongoing) {
        if (aiThinking_) {
            statusTitleLabel_->setText(explosive ? "Explosive AI Targeting..." : "AI Calculating...");
            statusSubLabel_->setText(explosive
                ? "Running bitboard Minimax + Trap-Maximizer"
                : "Evaluating tactical continuations...");
        } else {
            const QString turnName = QString::fromStdString(session_.nameForMark(session_.currentTurn()));
            const QString sym = (session_.currentTurn() == ttt::Mark::X) ? "X" : "O";
            statusTitleLabel_->setText(turnName + "'s Turn (" + sym + ")");
            statusSubLabel_->setText("Click any open square or press keys 1-9");
        }
    } else if (out == ttt::Outcome::Draw) {
        statusTitleLabel_->setText("Stalemate Draw!");
        statusSubLabel_->setText(explosive
            ? "Impressive defense — you survived Explosive AI!"
            : "Perfectly balanced match. Ready for a rematch?");
    } else {
        const QString winner = QString::fromStdString(session_.winnerName());
        statusTitleLabel_->setText(winner + " Wins!");
        if (sc.currentStreak >= 2) {
            statusSubLabel_->setText(QString("Unstoppable! %1-game win streak").arg(sc.currentStreak));
        } else {
            statusSubLabel_->setText("Press R or click New Round to play again");
        }
    }

    // Scoreboard Card
    const QString p1Sym = (session_.player1Mark() == ttt::Mark::X) ? " [X]" : " [O]";
    const QString p2Sym = (session_.player2Mark() == ttt::Mark::X) ? " [X]" : " [O]";
    p1ScoreNameLabel_->setText(QString::fromStdString(session_.player1DisplayName()) + p1Sym);
    p2ScoreNameLabel_->setText(QString::fromStdString(session_.player2DisplayName()) + p2Sym);
    p1ScoreValLabel_->setText(QString::number(sc.player1Wins));
    drawScoreValLabel_->setText(QString::number(sc.draws));
    p2ScoreValLabel_->setText(QString::number(sc.player2Wins));
    roundInfoLabel_->setText(QString("Round %1").arg(sc.roundsPlayed + (session_.isGameOver() ? 0 : 1)));

    // AI Telemetry Card
    if (isAI) {
        if (const auto& dec = session_.lastAIDecision(); dec.has_value()) {
            telemetryTagLabel_->setText(QString::fromStdString(dec->tag).toUpper());
            telemetryTagLabel_->setStyleSheet(
                explosive
                    ? "font-size: 11px; font-weight: 800; color: #FF9500;"
                    : QString("font-size: 11px; font-weight: 800; color: %1;").arg(theme_.accent.name()));
            evalBar_->setValue(std::clamp(dec->score, -100, 100));
            nodesValLabel_->setText(QString::number(dec->nodesEvaluated));
            timeValLabel_->setText(QString::number(dec->thinkTimeMs, 'f', 2) + " ms");
            trapsValLabel_->setText(
                cfg.difficulty == ttt::Difficulty::Explosive
                    ? QString::number(dec->trapCount)
                    : "-");
        } else {
            telemetryTagLabel_->setText("READY");
            evalBar_->setValue(0);
            nodesValLabel_->setText("-");
            timeValLabel_->setText("-");
            trapsValLabel_->setText(cfg.difficulty == ttt::Difficulty::Explosive ? "0" : "-");
        }
    }

    undoBtn_->setEnabled(session_.canUndo() && !aiThinking_);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (stack_->currentIndex() == 1) {
        const int key = event->key();
        if (key >= Qt::Key_1 && key <= Qt::Key_9) {
            handleHumanCellInput(key - Qt::Key_1);
            return;
        }
        if (key == Qt::Key_R) {
            handleNewRound();
            return;
        }
        if (key == Qt::Key_Z) {
            handleUndo();
            return;
        }
        if (key == Qt::Key_Escape) {
            returnToSetup();
            return;
        }
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::setupDemoState(ttt::Difficulty diff, bool showVision) {
    selectedMode_ = ttt::GameMode::PlayerVsAI;
    selectedDifficulty_ = diff;
    selectedMark_ = ttt::Mark::X;
    p1NameEdit_->setText("Moein");
    aiVisionEnabled_ = showVision;

    ttt::MatchConfig cfg;
    cfg.mode = ttt::GameMode::PlayerVsAI;
    cfg.difficulty = diff;
    cfg.player1Name = "Moein";
    cfg.player1Mark = ttt::Mark::X;
    session_.configure(cfg);

    // Play a realistic tactical sequence so the screenshot shows active moves & telemetry
    (void)session_.applyMove(0); // X top-left
    (void)session_.performAIMove(); // AI responds optimally (center)
    (void)session_.applyMove(8); // X bottom-right
    (void)session_.performAIMove(); // AI blocks / attacks

    stack_->setCurrentIndex(1);
    applyTheme();
    refreshArenaUI();
}

} // namespace ttt::gui
