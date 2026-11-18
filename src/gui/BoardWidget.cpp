#include "gui/BoardWidget.hpp"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>

#include <algorithm>
#include <cmath>

namespace ttt::gui {

namespace {
constexpr float kPi = 3.14159265358979323846f;
}

BoardWidget::BoardWidget(QWidget* parent)
    : QWidget(parent),
      rng_(std::random_device{}()) {
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    cells_.fill(ttt::Mark::Empty);
    cellAnimProgress_.fill(1.0f);
    evalScores_.fill(ttt::AIDecision::kNoScore);

    animTimer_.setInterval(16); // ~60 FPS
    connect(&animTimer_, &QTimer::timeout, this, &BoardWidget::onTick);
    frameClock_.start();
    animTimer_.start();
}

void BoardWidget::setTheme(const ThemePalette& theme) {
    theme_ = theme;
    update();
}

void BoardWidget::setExplosiveMode(bool enabled) {
    explosiveMode_ = enabled;
    update();
}

void BoardWidget::syncBoard(const ttt::Board& board,
                            ttt::Mark currentTurn,
                            bool inputEnabled,
                            bool showHints,
                            const std::array<int, 9>& evalScores) {
    for (int i = 0; i < 9; ++i) {
        const ttt::Mark m = board.at(i);
        if (m == ttt::Mark::Empty) {
            cellAnimProgress_[static_cast<size_t>(i)] = 0.0f;
        } else if (cells_[static_cast<size_t>(i)] == ttt::Mark::Empty) {
            // If synced without explicit trigger, show complete immediately.
            cellAnimProgress_[static_cast<size_t>(i)] = 1.0f;
        }
        cells_[static_cast<size_t>(i)] = m;
    }

    currentTurn_ = currentTurn;
    inputEnabled_ = inputEnabled;
    showHints_ = showHints;
    evalScores_ = evalScores;

    if (const auto lm = board.lastMove(); lm.has_value()) {
        lastMoveCell_ = lm->cell;
    } else {
        lastMoveCell_ = -1;
    }

    if (board.outcome() == ttt::Outcome::Ongoing) {
        winLine_.reset();
        winnerMark_ = ttt::Mark::Empty;
        winLineProgress_ = 0.0f;
    }

    setCursor((inputEnabled_ && hoveredCell_ >= 0 && cells_[static_cast<size_t>(hoveredCell_)] == ttt::Mark::Empty)
                  ? Qt::PointingHandCursor
                  : Qt::ArrowCursor);
    update();
}

void BoardWidget::triggerMoveAnimation(int cell, ttt::Mark mark, bool explosiveStrike) {
    if (cell < 0 || cell >= 9) return;
    cells_[static_cast<size_t>(cell)] = mark;
    cellAnimProgress_[static_cast<size_t>(cell)] = 0.01f;
    lastMoveCell_ = cell;

    const QPointF center = cellRect(cell).center();
    const QColor core = (mark == ttt::Mark::X) ? theme_.markX : theme_.markO;
    const QColor glow = (mark == ttt::Mark::X) ? theme_.markXGlow : theme_.markOGlow;

    if (explosiveStrike) {
        spawnBurst(center, theme_.explosivePrimary, theme_.explosiveSecondary,
                   38, 95.0f, 320.0f, 0.72f);
        spawnShockwave(center, theme_.explosiveSecondary,
                       static_cast<float>(cellRect(cell).width()) * 1.15f, 0.42f, 4.5f);
        spawnShockwave(center, theme_.explosivePrimary,
                       static_cast<float>(cellRect(cell).width()) * 0.75f, 0.30f, 3.0f);
        startShake(7.0f, 0.22f);
    } else {
        spawnBurst(center, core, glow, 14, 55.0f, 165.0f, 0.45f);
        spawnShockwave(center, core,
                       static_cast<float>(cellRect(cell).width()) * 0.58f, 0.28f, 2.2f);
    }
    update();
}

void BoardWidget::triggerWinAnimation(const std::array<int, 3>& winLine,
                                      ttt::Mark winner,
                                      bool explosiveWin) {
    winLine_ = winLine;
    winnerMark_ = winner;
    winLineProgress_ = 0.01f;

    const QColor p1 = explosiveWin ? theme_.explosivePrimary : theme_.winLine;
    const QColor p2 = explosiveWin
                          ? theme_.explosiveSecondary
                          : ((winner == ttt::Mark::X) ? theme_.markX : theme_.markO);

    for (int cell : winLine) {
        const QPointF c = cellRect(cell).center();
        spawnBurst(c, p1, p2, explosiveWin ? 36 : 24, 90.0f, explosiveWin ? 360.0f : 260.0f, 0.95f);
        spawnShockwave(c, p1, static_cast<float>(cellRect(cell).width()) * (explosiveWin ? 1.5f : 1.0f),
                       explosiveWin ? 0.65f : 0.48f, explosiveWin ? 5.0f : 3.2f);
    }

    if (explosiveWin) {
        startShake(11.0f, 0.36f);
    } else {
        startShake(4.0f, 0.16f);
    }
    update();
}

void BoardWidget::triggerDrawAnimation() {
    drawFlash_ = 1.0f;
    const QPointF center = boardRect().center();
    spawnShockwave(center, theme_.accent, static_cast<float>(boardRect().width()) * 0.65f, 0.52f, 3.0f);
    update();
}

void BoardWidget::clearAnimations() {
    winLine_.reset();
    winnerMark_ = ttt::Mark::Empty;
    winLineProgress_ = 0.0f;
    drawFlash_ = 0.0f;
    particles_.clear();
    shockwaves_.clear();
    shakeTime_ = 0.0f;
    shakeDuration_ = 0.0f;
    cellAnimProgress_.fill(0.0f);
    update();
}

float BoardWidget::easeOutCubic(float t) noexcept {
    t = std::clamp(t, 0.0f, 1.0f);
    const float inv = 1.0f - t;
    return 1.0f - inv * inv * inv;
}

QRectF BoardWidget::boardRect() const {
    const float pad = 18.0f;
    const float availW = std::max(100.0f, static_cast<float>(width()) - 2.0f * pad);
    const float availH = std::max(100.0f, static_cast<float>(height()) - 2.0f * pad);
    const float side = std::min(availW, availH);
    const float x = (static_cast<float>(width()) - side) * 0.5f;
    const float y = (static_cast<float>(height()) - side) * 0.5f;
    return QRectF(x, y, side, side);
}

QRectF BoardWidget::cellRect(int cell) const {
    const QRectF br = boardRect();
    const float innerPad = br.width() * 0.045f;
    const float gap = br.width() * 0.032f;
    const float tileSide = (br.width() - 2.0f * innerPad - 2.0f * gap) / 3.0f;
    const int row = cell / 3;
    const int col = cell % 3;
    const float x = br.left() + innerPad + col * (tileSide + gap);
    const float y = br.top() + innerPad + row * (tileSide + gap);
    return QRectF(x, y, tileSide, tileSide);
}

int BoardWidget::hitTestCell(const QPointF& pos) const {
    for (int i = 0; i < 9; ++i) {
        if (cellRect(i).contains(pos)) {
            return i;
        }
    }
    return -1;
}

void BoardWidget::spawnBurst(const QPointF& center, const QColor& primary,
                             const QColor& secondary, int count,
                             float minSpeed, float maxSpeed, float maxLife) {
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * kPi);
    std::uniform_real_distribution<float> speedDist(minSpeed, maxSpeed);
    std::uniform_real_distribution<float> sizeDist(2.5f, 6.2f);
    std::uniform_real_distribution<float> lifeDist(maxLife * 0.55f, maxLife);
    std::uniform_real_distribution<float> colorPick(0.0f, 1.0f);

    for (int i = 0; i < count; ++i) {
        const float angle = angleDist(rng_);
        const float speed = speedDist(rng_);
        Particle pt;
        pt.pos = center;
        pt.vel = QPointF(std::cos(angle) * speed, std::sin(angle) * speed);
        pt.size = sizeDist(rng_);
        pt.maxLife = lifeDist(rng_);
        pt.life = pt.maxLife;
        pt.color = (colorPick(rng_) < 0.55f) ? primary : secondary;
        pt.drag = 0.92f;
        particles_.push_back(pt);
    }
}

void BoardWidget::spawnShockwave(const QPointF& center, const QColor& color,
                                 float maxRadius, float maxLife, float strokeWidth) {
    Shockwave sw;
    sw.center = center;
    sw.radius = 6.0f;
    sw.maxRadius = maxRadius;
    sw.maxLife = maxLife;
    sw.life = maxLife;
    sw.color = color;
    sw.strokeWidth = strokeWidth;
    shockwaves_.push_back(sw);
}

void BoardWidget::startShake(float intensity, float duration) {
    shakeIntensity_ = intensity;
    shakeDuration_ = duration;
    shakeTime_ = duration;
}

void BoardWidget::onTick() {
    const float dt = std::clamp(static_cast<float>(frameClock_.restart()) / 1000.0f, 0.001f, 0.05f);
    ambientPhase_ += dt * 2.2f;
    if (ambientPhase_ > 1000.0f) ambientPhase_ -= 1000.0f;

    bool dirty = explosiveMode_ || hoveredCell_ >= 0 || winLine_.has_value();

    for (int i = 0; i < 9; ++i) {
        auto& p = cellAnimProgress_[static_cast<size_t>(i)];
        if (cells_[static_cast<size_t>(i)] != ttt::Mark::Empty && p < 1.0f) {
            p = std::min(1.0f, p + dt / 0.26f);
            dirty = true;
        }
    }

    if (winLine_.has_value() && winLineProgress_ < 1.0f) {
        winLineProgress_ = std::min(1.0f, winLineProgress_ + dt / 0.34f);
        dirty = true;
    }

    if (drawFlash_ > 0.0f) {
        drawFlash_ = std::max(0.0f, drawFlash_ - dt * 1.8f);
        dirty = true;
    }

    if (shakeTime_ > 0.0f) {
        shakeTime_ = std::max(0.0f, shakeTime_ - dt);
        dirty = true;
    }

    if (!particles_.empty()) {
        dirty = true;
        for (auto& pt : particles_) {
            pt.pos += pt.vel * dt;
            pt.vel *= std::pow(pt.drag, dt * 60.0f);
            pt.life -= dt;
        }
        particles_.erase(
            std::remove_if(particles_.begin(), particles_.end(),
                           [](const Particle& pt) { return pt.life <= 0.0f; }),
            particles_.end());
    }

    if (!shockwaves_.empty()) {
        dirty = true;
        for (auto& sw : shockwaves_) {
            sw.life -= dt;
            const float progress = 1.0f - std::clamp(sw.life / sw.maxLife, 0.0f, 1.0f);
            sw.radius = sw.maxRadius * easeOutCubic(progress);
        }
        shockwaves_.erase(
            std::remove_if(shockwaves_.begin(), shockwaves_.end(),
                           [](const Shockwave& sw) { return sw.life <= 0.0f; }),
            shockwaves_.end());
    }

    if (dirty) {
        update();
    }
}

void BoardWidget::mouseMoveEvent(QMouseEvent* event) {
    const int hit = hitTestCell(event->position());
    if (hit != hoveredCell_) {
        hoveredCell_ = hit;
        const bool clickable = inputEnabled_ && hit >= 0 &&
                               cells_[static_cast<size_t>(hit)] == ttt::Mark::Empty;
        setCursor(clickable ? Qt::PointingHandCursor : Qt::ArrowCursor);
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void BoardWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && inputEnabled_) {
        const int hit = hitTestCell(event->position());
        if (hit >= 0 && cells_[static_cast<size_t>(hit)] == ttt::Mark::Empty) {
            emit cellClicked(hit);
        }
    }
    QWidget::mousePressEvent(event);
}

void BoardWidget::leaveEvent(QEvent* event) {
    if (hoveredCell_ != -1) {
        hoveredCell_ = -1;
        setCursor(Qt::ArrowCursor);
        update();
    }
    QWidget::leaveEvent(event);
}

void BoardWidget::drawMarkX(QPainter& p, const QRectF& rect, float progress,
                            const QColor& core, const QColor& glow, float alpha) const {
    const float margin = rect.width() * 0.24f;
    const QPointF tl(rect.left() + margin, rect.top() + margin);
    const QPointF br(rect.right() - margin, rect.bottom() - margin);
    const QPointF tr(rect.right() - margin, rect.top() + margin);
    const QPointF bl(rect.left() + margin, rect.bottom() - margin);

    const float t1 = easeOutCubic(std::clamp(progress * 2.0f, 0.0f, 1.0f));
    const float t2 = easeOutCubic(std::clamp((progress - 0.42f) / 0.58f, 0.0f, 1.0f));

    const float baseWidth = std::max(5.5f, static_cast<float>(rect.width()) * 0.095f);

    auto drawStroke = [&](const QPointF& a, const QPointF& b, float t) {
        if (t <= 0.001f) return;
        const QPointF end = a + (b - a) * t;

        // Multi-layer neon bloom
        QColor outer = glow;
        outer.setAlphaF(0.22f * alpha);
        p.setPen(QPen(outer, baseWidth * 2.4f, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(a, end);

        QColor mid = core;
        mid.setAlphaF(0.45f * alpha);
        p.setPen(QPen(mid, baseWidth * 1.45f, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(a, end);

        QColor inner = core;
        inner.setAlphaF(alpha);
        p.setPen(QPen(inner, baseWidth, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(a, end);

        QColor highlight(255, 255, 255);
        highlight.setAlphaF(0.45f * alpha);
        p.setPen(QPen(highlight, baseWidth * 0.30f, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(a, end);
    };

    drawStroke(tl, br, t1);
    drawStroke(tr, bl, t2);
}

void BoardWidget::drawMarkO(QPainter& p, const QRectF& rect, float progress,
                            const QColor& core, const QColor& glow, float alpha) const {
    const float margin = rect.width() * 0.22f;
    const QRectF arcRect = rect.adjusted(margin, margin, -margin, -margin);
    const float eased = easeOutCubic(progress);
    const int spanAngle = static_cast<int>(std::round(-360.0f * 16.0f * eased));
    if (spanAngle == 0) return;

    const float baseWidth = std::max(5.5f, static_cast<float>(rect.width()) * 0.092f);
    p.setBrush(Qt::NoBrush);

    QColor outer = glow;
    outer.setAlphaF(0.22f * alpha);
    p.setPen(QPen(outer, baseWidth * 2.4f, Qt::SolidLine, Qt::RoundCap));
    p.drawArc(arcRect, 90 * 16, spanAngle);

    QColor mid = core;
    mid.setAlphaF(0.45f * alpha);
    p.setPen(QPen(mid, baseWidth * 1.45f, Qt::SolidLine, Qt::RoundCap));
    p.drawArc(arcRect, 90 * 16, spanAngle);

    QColor inner = core;
    inner.setAlphaF(alpha);
    p.setPen(QPen(inner, baseWidth, Qt::SolidLine, Qt::RoundCap));
    p.drawArc(arcRect, 90 * 16, spanAngle);

    QColor highlight(255, 255, 255);
    highlight.setAlphaF(0.42f * alpha);
    p.setPen(QPen(highlight, baseWidth * 0.28f, Qt::SolidLine, Qt::RoundCap));
    p.drawArc(arcRect, 90 * 16, spanAngle);
}

void BoardWidget::drawHintBadge(QPainter& p, const QRectF& tileRect, int score) const {
    if (score == ttt::AIDecision::kNoScore) return;

    QString label;
    QColor pillBg;
    QColor pillFg;
    if (score > 0) {
        label = "WIN";
        pillBg = QColor(16, 185, 129, 46);
        pillFg = QColor("#34D399");
    } else if (score == 0) {
        label = "DRAW";
        pillBg = QColor(56, 189, 248, 40);
        pillFg = QColor("#7DD3FC");
    } else {
        label = "RISK";
        pillBg = QColor(244, 63, 94, 46);
        pillFg = QColor("#FB7185");
    }

    const float w = std::min(52.0f, static_cast<float>(tileRect.width()) * 0.44f);
    const float h = 18.0f;
    const QRectF pill(tileRect.center().x() - w * 0.5f,
                      tileRect.bottom() - h - 7.0f, w, h);

    p.setPen(QPen(pillFg, 1.0));
    p.setBrush(pillBg);
    p.drawRoundedRect(pill, 6.0, 6.0);

    QFont f = font();
    f.setPixelSize(10);
    f.setBold(true);
    p.setFont(f);
    p.setPen(pillFg);
    p.drawText(pill, Qt::AlignCenter, label);
}

void BoardWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    // Apply smooth damped camera shake if active
    if (shakeTime_ > 0.0f && shakeDuration_ > 0.0f) {
        const float factor = shakeTime_ / shakeDuration_;
        const float mag = shakeIntensity_ * factor * factor;
        const float ox = std::sin(ambientPhase_ * 42.0f) * mag;
        const float oy = std::cos(ambientPhase_ * 37.0f) * mag;
        p.translate(ox, oy);
    }

    const QRectF br = boardRect();

    // 1. Ambient outer aura behind the board (fades fully within widget bounds)
    {
        const float pulse = 0.5f + 0.5f * std::sin(ambientPhase_);
        const float auraR = static_cast<float>(std::min(width(), height())) * 0.49f;
        QRadialGradient aura(br.center(), auraR);
        if (explosiveMode_) {
            QColor c1 = theme_.explosivePrimary;
            c1.setAlphaF(0.20f + 0.09f * pulse);
            QColor c2 = theme_.explosiveSecondary;
            c2.setAlphaF(0.08f + 0.04f * pulse);
            aura.setColorAt(0.0, c1);
            aura.setColorAt(0.55, c2);
            aura.setColorAt(0.95, Qt::transparent);
            aura.setColorAt(1.0, Qt::transparent);
        } else {
            QColor c1 = theme_.accent;
            c1.setAlphaF(0.14f);
            aura.setColorAt(0.0, c1);
            aura.setColorAt(0.92, Qt::transparent);
            aura.setColorAt(1.0, Qt::transparent);
        }
        p.setPen(Qt::NoPen);
        p.setBrush(aura);
        p.drawEllipse(br.center(), auraR, auraR);
    }

    // 2. Main board container card
    {
        const QColor frameBorder = explosiveMode_
                                       ? QColor(255, 88, 34, 190)
                                       : theme_.border;
        p.setPen(QPen(frameBorder, explosiveMode_ ? 2.2 : 1.5));
        p.setBrush(theme_.boardBg);
        p.drawRoundedRect(br, 24.0, 24.0);
    }

    // Helper to check if a cell belongs to the winning line
    auto isWinCell = [&](int cell) {
        if (!winLine_.has_value()) return false;
        const auto& wl = *winLine_;
        return cell == wl[0] || cell == wl[1] || cell == wl[2];
    };

    // 3. Draw all 9 tiles
    for (int cell = 0; cell < 9; ++cell) {
        const QRectF tr = cellRect(cell);
        const ttt::Mark mark = cells_[static_cast<size_t>(cell)];
        const bool empty = (mark == ttt::Mark::Empty);
        const bool hovered = inputEnabled_ && empty && (hoveredCell_ == cell);
        const bool winning = isWinCell(cell);
        const bool isLast = (cell == lastMoveCell_ && !empty);

        QColor fill = theme_.tileBg;
        QColor border = theme_.tileBorder;
        float borderW = 1.4f;

        if (winning) {
            const float pulse = 0.5f + 0.5f * std::sin(ambientPhase_ * 2.5f);
            fill = theme_.tileWinBg;
            border = explosiveMode_ ? theme_.explosiveSecondary : theme_.winLine;
            border.setAlphaF(0.65f + 0.35f * pulse);
            borderW = 2.4f;
        } else if (hovered) {
            fill = theme_.tileHover;
            border = (currentTurn_ == ttt::Mark::X) ? theme_.markX : theme_.markO;
            border.setAlphaF(0.75f);
            borderW = 2.0f;
        } else if (isLast) {
            border = explosiveMode_
                         ? theme_.explosiveSecondary
                         : ((mark == ttt::Mark::X) ? theme_.markX : theme_.markO);
            border.setAlphaF(0.55f);
            borderW = 1.8f;
        }

        p.setPen(QPen(border, borderW));
        p.setBrush(fill);
        p.drawRoundedRect(tr, 16.0, 16.0);

        // Subtle keypad number (1-9) in top-left corner of empty tiles (homage to 2024 CLI!)
        if (empty) {
            QFont idxFont = font();
            idxFont.setPixelSize(11);
            idxFont.setBold(true);
            p.setFont(idxFont);
            QColor idxColor = theme_.textMuted;
            idxColor.setAlphaF(hovered ? 0.90f : 0.42f);
            p.setPen(idxColor);
            p.drawText(tr.adjusted(10, 7, -6, -6), Qt::AlignTop | Qt::AlignLeft,
                       QString::number(cell + 1));

            // Hover ghost preview of current player's mark
            if (hovered) {
                const float ghostAlpha = 0.24f + 0.06f * std::sin(ambientPhase_ * 2.2f);
                if (currentTurn_ == ttt::Mark::X) {
                    drawMarkX(p, tr, 1.0f, theme_.markX, theme_.markXGlow, ghostAlpha);
                } else if (currentTurn_ == ttt::Mark::O) {
                    drawMarkO(p, tr, 1.0f, theme_.markO, theme_.markOGlow, ghostAlpha);
                }
            }

            if (showHints_ && inputEnabled_) {
                drawHintBadge(p, tr, evalScores_[static_cast<size_t>(cell)]);
            }
        } else {
            const float prog = cellAnimProgress_[static_cast<size_t>(cell)];
            if (mark == ttt::Mark::X) {
                drawMarkX(p, tr, prog, theme_.markX, theme_.markXGlow, 1.0f);
            } else if (mark == ttt::Mark::O) {
                drawMarkO(p, tr, prog, theme_.markO, theme_.markOGlow, 1.0f);
            }
        }
    }

    // 4. Animated winning laser beam
    if (winLine_.has_value() && winLineProgress_ > 0.01f) {
        const auto& wl = *winLine_;
        const QPointF p0 = cellRect(wl[0]).center();
        const QPointF p2 = cellRect(wl[2]).center();
        const QPointF dir = p2 - p0;
        const float len = std::hypot(static_cast<float>(dir.x()), static_cast<float>(dir.y()));
        const QPointF unit = (len > 0.001f) ? (dir / len) : QPointF(1.0, 0.0);
        const float ext = static_cast<float>(cellRect(wl[0]).width()) * 0.22f;
        const QPointF start = p0 - unit * ext;
        const QPointF fullEnd = p2 + unit * ext;
        const QPointF curEnd = start + (fullEnd - start) * easeOutCubic(winLineProgress_);

        const QColor beamColor = explosiveMode_ ? theme_.explosiveSecondary : theme_.winLine;

        QColor outer = explosiveMode_ ? theme_.explosivePrimary : beamColor;
        outer.setAlphaF(0.35f);
        p.setPen(QPen(outer, 20.0, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(start, curEnd);

        QColor mid = beamColor;
        mid.setAlphaF(0.85f);
        p.setPen(QPen(mid, 10.0, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(start, curEnd);

        p.setPen(QPen(QColor(255, 255, 255, 235), 4.0, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(start, curEnd);
    }

    // 5. Draw shockwave rings
    for (const auto& sw : shockwaves_) {
        const float alpha = std::clamp(sw.life / sw.maxLife, 0.0f, 1.0f);
        QColor c = sw.color;
        c.setAlphaF(alpha * 0.78f);
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(c, sw.strokeWidth * (0.4f + 0.6f * alpha)));
        p.drawEllipse(sw.center, sw.radius, sw.radius);
    }

    // 6. Draw particles
    p.setPen(Qt::NoPen);
    for (const auto& pt : particles_) {
        const float alpha = std::clamp(pt.life / pt.maxLife, 0.0f, 1.0f);
        QColor c = pt.color;
        c.setAlphaF(alpha);
        p.setBrush(c);
        const float r = pt.size * (0.35f + 0.65f * alpha);
        p.drawEllipse(pt.pos, r, r);
    }

    // 7. Subtle draw flash overlay
    if (drawFlash_ > 0.01f) {
        QColor f = theme_.accent;
        f.setAlphaF(drawFlash_ * 0.14f);
        p.setPen(Qt::NoPen);
        p.setBrush(f);
        p.drawRoundedRect(br, 24.0, 24.0);
    }
}

} // namespace ttt::gui
