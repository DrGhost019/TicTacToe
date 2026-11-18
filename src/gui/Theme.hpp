#pragma once

#include <QColor>
#include <QString>

namespace ttt::gui {

enum class ThemeId {
    CyberNeon = 0,
    ObsidianGold = 1,
    Retro2024 = 2
};

struct ThemePalette {
    ThemeId id = ThemeId::CyberNeon;
    QString name;
    QString tagline;

    QColor bgTop;
    QColor bgBottom;
    QColor surface;
    QColor surfaceElevated;
    QColor border;
    QColor borderAccent;

    QColor boardBg;
    QColor tileBg;
    QColor tileHover;
    QColor tileBorder;
    QColor tileWinBg;

    QColor markX;
    QColor markXGlow;
    QColor markO;
    QColor markOGlow;

    QColor accent;
    QColor accentHover;
    QColor textPrimary;
    QColor textSecondary;
    QColor textMuted;

    QColor winLine;
    QColor explosivePrimary = QColor("#FF3B30");
    QColor explosiveSecondary = QColor("#FF9500");
};

inline ThemePalette makeTheme(ThemeId id) {
    ThemePalette t;
    t.id = id;
    switch (id) {
        case ThemeId::CyberNeon:
            t.name = "Cyber Neon";
            t.tagline = "2026 Electric Glass";
            t.bgTop = QColor("#080D1A");
            t.bgBottom = QColor("#0F172A");
            t.surface = QColor("#111B2E");
            t.surfaceElevated = QColor("#182540");
            t.border = QColor("#233354");
            t.borderAccent = QColor("#00E5FF");

            t.boardBg = QColor("#0D1526");
            t.tileBg = QColor("#142038");
            t.tileHover = QColor("#1B2C4D");
            t.tileBorder = QColor("#263A60");
            t.tileWinBg = QColor("#19344A");

            t.markX = QColor("#00F0FF");
            t.markXGlow = QColor("#00B8D4");
            t.markO = QColor("#FF2E7E");
            t.markOGlow = QColor("#D81B60");

            t.accent = QColor("#38BDF8");
            t.accentHover = QColor("#7DD3FC");
            t.textPrimary = QColor("#F8FAFC");
            t.textSecondary = QColor("#CBD5E1");
            t.textMuted = QColor("#64748B");
            t.winLine = QColor("#FACC15");
            break;

        case ThemeId::ObsidianGold:
            t.name = "Obsidian Gold";
            t.tagline = "Minimalist Luxury";
            t.bgTop = QColor("#0B0B0E");
            t.bgBottom = QColor("#15151B");
            t.surface = QColor("#17171F");
            t.surfaceElevated = QColor("#20202B");
            t.border = QColor("#2C2C3A");
            t.borderAccent = QColor("#F5C451");

            t.boardBg = QColor("#121218");
            t.tileBg = QColor("#1B1B24");
            t.tileHover = QColor("#252532");
            t.tileBorder = QColor("#323244");
            t.tileWinBg = QColor("#2B2518");

            t.markX = QColor("#F5C451");
            t.markXGlow = QColor("#D99B26");
            t.markO = QColor("#64D2FF");
            t.markOGlow = QColor("#0A84FF");

            t.accent = QColor("#F5C451");
            t.accentHover = QColor("#FCD97B");
            t.textPrimary = QColor("#FAFAFA");
            t.textSecondary = QColor("#D4D4D8");
            t.textMuted = QColor("#71717A");
            t.winLine = QColor("#F5C451");
            break;

        case ThemeId::Retro2024:
            t.name = "Retro 2024";
            t.tagline = "CRT Console Tribute";
            t.bgTop = QColor("#050E08");
            t.bgBottom = QColor("#091910");
            t.surface = QColor("#0B1F14");
            t.surfaceElevated = QColor("#102B1C");
            t.border = QColor("#1B4D32");
            t.borderAccent = QColor("#39FF14");

            t.boardBg = QColor("#07150D");
            t.tileBg = QColor("#0D2417");
            t.tileHover = QColor("#143622");
            t.tileBorder = QColor("#1F5A38");
            t.tileWinBg = QColor("#164026");

            t.markX = QColor("#38BDF8");
            t.markXGlow = QColor("#0284C7");
            t.markO = QColor("#FF4D6D");
            t.markOGlow = QColor("#E11D48");

            t.accent = QColor("#39FF14");
            t.accentHover = QColor("#72FF57");
            t.textPrimary = QColor("#E2FCE8");
            t.textSecondary = QColor("#A7F3C0");
            t.textMuted = QColor("#4D8B64");
            t.winLine = QColor("#39FF14");
            break;
    }
    return t;
}

inline QString buildAppStylesheet(const ThemePalette& t, bool explosive) {
    const QColor primaryAccent = explosive ? t.explosiveSecondary : t.accent;
    const QColor borderFocus = explosive ? t.explosivePrimary : t.borderAccent;

    return QString(R"(
        QMainWindow, QWidget#RootWidget {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 %1, stop:1 %2);
            color: %3;
            font-family: "Inter", "Segoe UI", "Noto Sans", sans-serif;
        }
        QFrame#Card, QFrame#HudCard {
            background-color: %4;
            border: 1px solid %5;
            border-radius: 16px;
        }
        QFrame#ExplosiveHudCard {
            background-color: %4;
            border: 1px solid #FF5522;
            border-radius: 16px;
        }
        QLabel {
            color: %3;
            background: transparent;
        }
        QLabel#MutedLabel {
            color: %6;
        }
        QLabel#SecondaryLabel {
            color: %7;
        }
        QLabel#HeroTitle {
            font-size: 34px;
            font-weight: 800;
            letter-spacing: 3px;
            color: %3;
        }
        QLabel#HeroBadge {
            background-color: %8;
            color: %9;
            border: 1px solid %10;
            border-radius: 12px;
            padding: 4px 12px;
            font-size: 11px;
            font-weight: 700;
            letter-spacing: 1.5px;
        }
        QLabel#SectionTitle {
            font-size: 12px;
            font-weight: 700;
            letter-spacing: 1.4px;
            color: %6;
        }
        QLineEdit {
            background-color: %8;
            color: %3;
            border: 1px solid %5;
            border-radius: 10px;
            padding: 9px 12px;
            font-size: 14px;
            font-weight: 600;
            selection-background-color: %9;
        }
        QLineEdit:focus {
            border: 1px solid %10;
        }
        QPushButton {
            background-color: %8;
            color: %3;
            border: 1px solid %5;
            border-radius: 11px;
            padding: 9px 14px;
            font-size: 13px;
            font-weight: 600;
        }
        QPushButton:hover {
            border: 1px solid %10;
            background-color: %11;
        }
        QPushButton:pressed {
            background-color: %4;
        }
        QPushButton:disabled {
            color: %6;
            border-color: %5;
            background-color: %4;
        }
        QPushButton[selected="true"] {
            background-color: %11;
            border: 2px solid %9;
            color: %3;
        }
        QPushButton#PrimaryButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 %9, stop:1 %10);
            color: #060912;
            border: none;
            border-radius: 12px;
            padding: 12px 20px;
            font-size: 14px;
            font-weight: 800;
            letter-spacing: 1px;
        }
        QPushButton#PrimaryButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 %12, stop:1 %9);
        }
        QPushButton#ExplosivePrimaryButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #FF3B30, stop:1 #FF9500);
            color: #FFFFFF;
            border: none;
            border-radius: 12px;
            padding: 12px 20px;
            font-size: 14px;
            font-weight: 800;
            letter-spacing: 1px;
        }
        QPushButton#ExplosivePrimaryButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #FF574D, stop:1 #FFB03A);
        }
        QPushButton#TogglePill[active="true"] {
            background-color: %11;
            border: 1.5px solid %9;
            color: %9;
        }
        QProgressBar#EvalBar {
            background-color: %8;
            border: 1px solid %5;
            border-radius: 6px;
            height: 10px;
            text-align: center;
        }
        QProgressBar#EvalBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 %13, stop:1 %14);
            border-radius: 5px;
        }
    )")
        .arg(t.bgTop.name())
        .arg(t.bgBottom.name())
        .arg(t.textPrimary.name())
        .arg(t.surface.name())
        .arg(t.border.name())
        .arg(t.textMuted.name())
        .arg(t.textSecondary.name())
        .arg(t.surfaceElevated.name())
        .arg(primaryAccent.name())
        .arg(borderFocus.name())
        .arg(t.tileHover.name())
        .arg(t.accentHover.name())
        .arg(t.markX.name())
        .arg(t.markO.name());
}

} // namespace ttt::gui
