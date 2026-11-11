#include "games/BalanceTowerGame.hpp"

#include <QPainter>
#include <QLinearGradient>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFont>
#include <QtMath>
#include <algorithm>
#include <cmath>

namespace games {

BalanceTowerGame::BalanceTowerGame(QWidget *parent) : GameEngine(parent)
{
    setMinimumSize(360, 280);
}

QString BalanceTowerGame::gameName() const
{
    return QStringLiteral("Balance Tower — Wieża równowagi");
}

bool BalanceTowerGame::hasFreshSignal() const
{
    return m_hasSensorSample && !m_awaitingNeutral &&
           std::isfinite(m_inputAge) && m_inputAge <= kSignalTimeoutSeconds;
}

void BalanceTowerGame::initialize()
{
    QSettings settings(QStringLiteral("Biofeedback"), QStringLiteral("Biofeedback"));
    m_personalBestScore = settings.value(QStringLiteral("games/balance_tower/records/best_score"), 0).toInt();
    m_bestRound = settings.value(QStringLiteral("games/balance_tower/records/best_round"), 1).toInt();
    m_achievements = settings.value(QStringLiteral("games/balance_tower/achievements")).toStringList();
    m_blocks.clear();
    m_blocks.append({0.0, 0.0, 1.0, 1.0, 1.0});
    m_lives = 3;
    m_round = 1;
    m_combo = 0;
    m_bestCombo = 0;
    m_falls = 0;
    m_targetPosition = 0.0;
    m_targetDepth = 0.0;
    m_inputAge = kSignalTimeoutSeconds + 1.0;
    m_gestureDuration = 0.0;
    m_visualTime = 0.0;
    m_stability = 1.0;
    m_tilt = 0.0;
    m_currentForce = 0.0;
    m_blockWidth = 0.78;
    m_hasSensorSample = false;
    m_awaitingNeutral = true;
    m_gestureActive = false;
    m_depthAxis = false;
    m_roundCompleted = false;
    m_gameOver = false;
    m_campaignClean = true;
    m_status = QStringLiteral("Ustaw nacisk w strefie neutralnej 10–30%");
    setScore(0);
}

bool BalanceTowerGame::handleInput(double value)
{
    if (!std::isfinite(value) || m_gameOver || m_roundCompleted)
        return false;
    const double force = qBound(0.0, value, 100.0);
    m_currentForce = force;
    m_hasSensorSample = true;
    m_inputAge = 0.0;

    if (m_awaitingNeutral) {
        if (force >= kNeutralMin && force <= kNeutralMax) {
            m_awaitingNeutral = false;
            m_status = QStringLiteral("Gotowy — ustaw element i wykonaj krótki gest, aby go upuścić");
        } else {
            m_status = QStringLiteral("Oczekiwanie na strefę neutralną 10–30%");
        }
        return true;
    }

    // A high-force hold is a gesture, not a second control channel.
    if (force >= 82.0) {
        if (!m_gestureActive) {
            m_gestureActive = true;
            m_gestureDuration = 0.0;
        }
        return true;
    }

    if (m_gestureActive && force <= kNeutralMax) {
        const double duration = m_gestureDuration;
        m_gestureActive = false;
        m_gestureDuration = 0.0;
        if (duration >= 0.12 && duration <= 0.85)
            placeBlock();
        else if (duration > 0.85)
            toggleAxis();
        return true;
    }

    if (force >= kNeutralMin && force <= kNeutralMax)
        return true;

    const double normalized = force < kNeutralMin
        ? 0.5 * force / kNeutralMin
        : 0.5 + 0.5 * (force - kNeutralMax) / (100.0 - kNeutralMax);
    const double position = qBound(-1.0, (normalized - 0.5) * 2.0, 1.0);
    if (m_depthAxis)
        m_targetDepth = position * 0.42;
    else
        m_targetPosition = position * 0.42;
    return true;
}

void BalanceTowerGame::toggleAxis()
{
    m_depthAxis = !m_depthAxis;
    m_status = m_depthAxis
        ? QStringLiteral("Aktywna oś głębokości — ustaw element i wykonaj krótki gest")
        : QStringLiteral("Aktywna oś pozioma — ustaw element i wykonaj krótki gest");
}

double BalanceTowerGame::centerOfMassX() const
{
    double weighted = 0.0, mass = 0.0;
    for (const Block &block : m_blocks) {
        weighted += block.x * block.mass;
        mass += block.mass;
    }
    return mass > 0.0 ? weighted / mass : 0.0;
}

double BalanceTowerGame::centerOfMassZ() const
{
    double weighted = 0.0, mass = 0.0;
    for (const Block &block : m_blocks) {
        weighted += block.z * block.mass;
        mass += block.mass;
    }
    return mass > 0.0 ? weighted / mass : 0.0;
}

void BalanceTowerGame::placeBlock()
{
    if (!hasFreshSignal() || m_blocks.isEmpty())
        return;
    const Block &top = m_blocks.last();
    const double x = m_targetPosition;
    const double z = m_targetDepth;
    const double overlapX = qMax(0.0, (top.width + m_blockWidth) * 0.5 - std::abs(x - top.x));
    const double overlapZ = qMax(0.0, (top.depth + m_blockWidth) * 0.5 - std::abs(z - top.z));
    const double overlap = qMin(overlapX, overlapZ);
    if (overlap < 0.12) {
        ++m_falls;
        m_campaignClean = false;
        m_combo = 0;
        --m_lives;
        addScore(-20);
        m_status = QStringLiteral("Element spadł — utrata próby");
        if (m_lives <= 0) {
            m_gameOver = true;
            finishRound(false);
        }
        return;
    }

    const double precision = qBound(0.0, 1.0 - (std::abs(x - top.x) + std::abs(z - top.z)) / 0.84, 1.0);
    const double width = qMax(0.38, qMin(m_blockWidth, qMin(overlapX, overlapZ)));
    const double depth = width;
    const double mass = 1.0 + (m_blocks.size() % 3) * 0.12;
    m_blocks.append({x, z, width, depth, mass});
    ++m_combo;
    m_bestCombo = qMax(m_bestCombo, m_combo);
    addScore(50 + static_cast<int>(precision * 100.0) + qMin(m_combo, 10) * 5);
    const double comX = std::abs(centerOfMassX());
    const double comZ = std::abs(centerOfMassZ());
    m_stability = qBound(0.0, 1.0 - qMax(comX, comZ) / 0.62, 1.0);
    m_tilt = qBound(-0.35, (centerOfMassX() + centerOfMassZ()) * 0.42, 0.35);
    if (m_blocks.size() == 2)
        unlockAchievement(QStringLiteral("first_block"));
    if (m_blocks.size() >= 6)
        unlockAchievement(QStringLiteral("tower_six"));
    if (m_combo >= 5)
        unlockAchievement(QStringLiteral("steady_hand"));
    m_status = QStringLiteral("Element ustawiony — precyzja %1%").arg(qRound(precision * 100.0));
    if (m_blocks.size() - 1 >= kBlocksPerRound)
        finishRound(true);
}

void BalanceTowerGame::finishRound(bool stable)
{
    if (m_roundCompleted)
        return;
    m_roundCompleted = stable && m_round >= 3;
    if (stable) {
        addScore(250 * m_round);
        unlockAchievement(QStringLiteral("round_complete"));
        if (m_round >= 3)
            unlockAchievement(QStringLiteral("tower_master"));
        if (m_campaignClean && m_round >= 3)
            unlockAchievement(QStringLiteral("perfect_foundation"));
        if (m_round >= 3) {
            m_status = QStringLiteral("Wieża ukończona — zwycięstwo!");
        } else {
            ++m_round;
            m_bestRound = qMax(m_bestRound, m_round);
            m_blockWidth = qMax(0.48, 0.78 - 0.08 * (m_round - 1));
            m_blocks.clear();
            m_blocks.append({0.0, 0.0, 1.0, 1.0, 1.0});
            m_targetPosition = 0.0;
            m_targetDepth = 0.0;
            m_stability = 1.0;
            m_status = QStringLiteral("Nowa plansza — poziom %1").arg(m_round);
        }
    }
    saveRecords();
}

void BalanceTowerGame::unlockAchievement(const QString &id)
{
    if (m_achievements.contains(id))
        return;
    m_achievements.append(id);
    QSettings settings(QStringLiteral("Biofeedback"), QStringLiteral("Biofeedback"));
    settings.setValue(QStringLiteral("games/balance_tower/achievements"), m_achievements);
}

void BalanceTowerGame::saveRecords()
{
    if (getScore() > m_personalBestScore)
        m_personalBestScore = getScore();
    QSettings settings(QStringLiteral("Biofeedback"), QStringLiteral("Biofeedback"));
    settings.setValue(QStringLiteral("games/balance_tower/records/best_score"), m_personalBestScore);
    settings.setValue(QStringLiteral("games/balance_tower/records/best_round"),
                      qMax(m_bestRound, m_round));
    settings.sync();
}

void BalanceTowerGame::update(double deltaTime)
{
    if (!std::isfinite(deltaTime) || deltaTime <= 0.0)
        return;
    m_visualTime += qMin(deltaTime, 0.1);
    if (m_inputAge <= kSignalTimeoutSeconds)
        m_inputAge += deltaTime;
    if (m_gestureActive)
        m_gestureDuration += deltaTime;
    if (m_hasSensorSample && m_inputAge > kSignalTimeoutSeconds) {
        m_awaitingNeutral = true;
        m_gestureActive = false;
        m_gestureDuration = 0.0;
        m_status = QStringLiteral("Brak świeżego sygnału — wieża i gesty zatrzymane");
    }
    if (!hasFreshSignal() || m_gameOver || m_roundCompleted)
        return;

    m_stability = qBound(0.0, m_stability - qAbs(m_tilt) * deltaTime * 0.018, 1.0);
    m_tilt *= std::exp(-deltaTime * 1.8);
}

void BalanceTowerGame::render(QPainter &painter)
{
    const QRectF bounds = rect();
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0.0, QColor(12, 24, 48));
    bg.setColorAt(1.0, QColor(35, 67, 78));
    painter.fillRect(bounds, bg);
    painter.setRenderHint(QPainter::Antialiasing);
    const qreal scale = qMin(width() * 0.31, height() * 0.055);
    const qreal baseX = width() * 0.5;
    const qreal baseY = height() * 0.79;
    const qreal blockH = qMax<qreal>(13.0, height() * 0.038);
    const qreal perspective = qBound<qreal>(0.65, 1.0 - std::abs(m_tilt) * 0.35, 1.0);

    painter.setPen(QPen(QColor(125, 208, 220), 2.0));
    painter.setBrush(QColor(34, 87, 105));
    painter.drawRoundedRect(QRectF(baseX - scale, baseY, scale * 2, 22), 8, 8);
    for (int i = 0; i < m_blocks.size(); ++i) {
        const Block &block = m_blocks[i];
        const qreal y = baseY - (i + 1) * blockH;
        const qreal x = baseX + block.x * scale - block.width * scale * 0.5;
        const qreal w = block.width * scale;
        const qreal depthShift = block.z * scale * 0.22;
        const QRectF face(x + depthShift, y, w, blockH - 2);
        QColor fill = QColor::fromHsv((i * 29 + 165) % 360, 170, 225);
        painter.setPen(QPen(fill.lighter(145), 1.2));
        painter.setBrush(fill);
        painter.drawRoundedRect(face, 4, 4);
        painter.setPen(QPen(QColor(255,255,255,90), 1.0));
        painter.drawLine(face.topLeft() + QPointF(4,2), face.topRight() + QPointF(-4,2));
    }

    const qreal activeWidth = m_blocks.isEmpty() ? m_blockWidth : qMin(m_blockWidth, m_blocks.last().width);
    const qreal activeX = baseX + m_targetPosition * scale - activeWidth * scale * 0.5;
    const qreal activeY = baseY - (m_blocks.size() + 1) * blockH - 18;
    painter.setPen(QPen(QColor(255, 225, 135), 2.0, Qt::DashLine));
    painter.setBrush(QColor(255, 207, 96, 65));
    painter.drawRoundedRect(QRectF(activeX, activeY, activeWidth * scale,
                                   blockH - 2), 4, 4);

    painter.setPen(Qt::white);
    painter.setFont(QFont(QStringLiteral("Sans Serif"), qMax(9, height()/42), QFont::DemiBold));
    painter.drawText(QRectF(14, 8, width()-28, 24), Qt::AlignLeft,
        QStringLiteral("BALANCE TOWER  •  POZIOM %1/3").arg(m_round));
    painter.drawText(QRectF(14, 32, width()-28, 22), Qt::AlignLeft,
        QStringLiteral("Wynik %1   Elementy %2/%3   Życia %4   Seria %5")
            .arg(getScore()).arg(m_blocks.size()-1).arg(kBlocksPerRound).arg(m_lives).arg(m_combo));

    QRectF meter(18, height()-50, width()-36, 7);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(50,70,82));
    painter.drawRoundedRect(meter, 3, 3);
    painter.setBrush(QColor(89,220,190));
    painter.drawRoundedRect(QRectF(meter.left(), meter.top(), meter.width()*m_currentForce/100.0, meter.height()),3,3);
    painter.setPen(QColor(255,255,255));
    painter.drawText(QRectF(18,height()-39,width()-36,22),Qt::AlignLeft,
        QStringLiteral("Stabilność %1%   Oś: %2   %3")
            .arg(qRound(m_stability*100.0))
            .arg(m_depthAxis ? QStringLiteral("głębokość") : QStringLiteral("pozioma"))
            .arg(hasFreshSignal() ? QStringLiteral("SYGNAŁ AKTYWNY") : QStringLiteral("PAUZA — oczekiwanie na sygnał")));
    painter.drawText(QRectF(18,height()-20,width()-36,18),Qt::AlignCenter,
        QStringLiteral("10–30% neutralnie • krótki gest ≥82%: upuść • długi gest: zmień oś"));
    if (m_gameOver || m_roundCompleted) {
        painter.fillRect(bounds,QColor(3,10,20,205));
        painter.setFont(QFont(QStringLiteral("Sans Serif"),qMax(16,height()/18),QFont::Bold));
        painter.drawText(bounds,Qt::AlignCenter,m_roundCompleted ? QStringLiteral("WIEŻA UKOŃCZONA") : QStringLiteral("KONIEC GRY"));
    }
}

} // namespace games
