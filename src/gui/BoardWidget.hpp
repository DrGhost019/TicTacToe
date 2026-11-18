#pragma once

#include "engine/AI.hpp"
#include "engine/Board.hpp"
#include "gui/Theme.hpp"

#include <QElapsedTimer>
#include <QPointF>
#include <QRectF>
#include <QTimer>
#include <QWidget>

#include <array>
#include <optional>
#include <random>
#include <vector>

namespace ttt::gui {

class BoardWidget : public QWidget {
    Q_OBJECT

public:
    explicit BoardWidget(QWidget* parent = nullptr);

    void setTheme(const ThemePalette& theme);
    void setExplosiveMode(bool enabled);

    void syncBoard(const ttt::Board& board,
                   ttt::Mark currentTurn,
                   bool inputEnabled,
                   bool showHints,
                   const std::array<int, 9>& evalScores);

    void triggerMoveAnimation(int cell, ttt::Mark mark, bool explosiveStrike);
    void triggerWinAnimation(const std::array<int, 3>& winLine, ttt::Mark winner, bool explosiveWin);
    void triggerDrawAnimation();
    void clearAnimations();

    [[nodiscard]] QSize sizeHint() const override { return QSize(480, 480); }
    [[nodiscard]] QSize minimumSizeHint() const override { return QSize(320, 320); }

signals:
    void cellClicked(int cell);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private slots:
    void onTick();

private:
    struct Particle {
        QPointF pos;
        QPointF vel;
        float size = 4.0f;
        float life = 0.0f;
        float maxLife = 0.6f;
        QColor color;
        float drag = 0.94f;
    };

    struct Shockwave {
        QPointF center;
        float radius = 0.0f;
        float maxRadius = 120.0f;
        float life = 0.0f;
        float maxLife = 0.5f;
        QColor color;
        float strokeWidth = 4.0f;
    };

    ThemePalette theme_ = makeTheme(ThemeId::CyberNeon);
    bool explosiveMode_ = false;
    bool inputEnabled_ = true;
    bool showHints_ = false;

    std::array<ttt::Mark, 9> cells_{};
    std::array<float, 9> cellAnimProgress_{}; // 0.0 -> 1.0 stroke completion
    std::array<int, 9> evalScores_{};
    ttt::Mark currentTurn_ = ttt::Mark::X;
    int hoveredCell_ = -1;
    int lastMoveCell_ = -1;

    std::optional<std::array<int, 3>> winLine_;
    ttt::Mark winnerMark_ = ttt::Mark::Empty;
    float winLineProgress_ = 0.0f;
    float drawFlash_ = 0.0f;
    float ambientPhase_ = 0.0f;

    // Screen shake state
    float shakeTime_ = 0.0f;
    float shakeDuration_ = 0.0f;
    float shakeIntensity_ = 0.0f;

    std::vector<Particle> particles_;
    std::vector<Shockwave> shockwaves_;

    QTimer animTimer_;
    QElapsedTimer frameClock_;
    std::mt19937 rng_;

    [[nodiscard]] QRectF boardRect() const;
    [[nodiscard]] QRectF cellRect(int cell) const;
    [[nodiscard]] int hitTestCell(const QPointF& pos) const;

    void spawnBurst(const QPointF& center, const QColor& primary, const QColor& secondary,
                    int count, float minSpeed, float maxSpeed, float maxLife);
    void spawnShockwave(const QPointF& center, const QColor& color,
                        float maxRadius, float maxLife, float strokeWidth);
    void startShake(float intensity, float duration);
    void drawMarkX(QPainter& p, const QRectF& rect, float progress, const QColor& core,
                   const QColor& glow, float alpha = 1.0f) const;
    void drawMarkO(QPainter& p, const QRectF& rect, float progress, const QColor& core,
                   const QColor& glow, float alpha = 1.0f) const;
    void drawHintBadge(QPainter& p, const QRectF& tileRect, int score) const;
    static float easeOutCubic(float t) noexcept;
};

} // namespace ttt::gui
