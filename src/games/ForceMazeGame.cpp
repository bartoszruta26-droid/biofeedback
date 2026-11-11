#include "games/ForceMazeGame.hpp"

#include <QPainter>
#include <QMouseEvent>
#include <QPainterPath>
#include <QPolygonF>
#include <QSettings>
#include <QFont>
#include <algorithm>
#include <cmath>

namespace games {

ForceMazeGame::ForceMazeGame(QWidget *parent)
    : GameEngine(parent)
    , m_level(1)
    , m_lives(3)
    , m_crystals(0)
    , m_totalCrystals(0)
    , m_checkpoints(0)
    , m_lastProcessedRow(-1)
    , m_personalBestScore(0)
    , m_personalBestCrystals(0)
    , m_personalBestLevel(1)
    , m_cleanLevels(0)
    , m_campaignDeaths(0)
    , m_progressRows(0.0)
    , m_playerColumn(4.0)
    , m_targetColumn(4.0)
    , m_inputAge(kSignalTimeoutSeconds + 1.0)
    , m_visualTime(0.0)
    , m_forwardSpeed(1.35)
    , m_hitFlash(0.0)
    , m_checkpointFlash(0.0)
    , m_crystalPulse(0.0)
    , m_playerTrailPhase(0.0)
    , m_currentForce(0.0)
    , m_cameraShake(0.0)
    , m_hasSensorSample(false)
    , m_awaitingNeutral(true)
    , m_campaignCompleted(false)
    , m_levelClean(true)
    , m_campaignClean(true)
    , m_leftHanded(false)
    , m_status(QStringLiteral("Zwolnij tensometr do strefy neutralnej"))
{
    setMinimumSize(320, 240);
}

QString ForceMazeGame::gameName() const
{
    return QStringLiteral("Force Maze — Labirynt siły");
}

bool ForceMazeGame::hasFreshSignal() const
{
    return m_hasSensorSample && !m_awaitingNeutral &&
           std::isfinite(m_inputAge) && m_inputAge <= kSignalTimeoutSeconds;
}

int ForceMazeGame::corridorCenter(int row) const
{
    // A deterministic, reproducible maze path. Each section shifts by at most
    // one column, so the route is navigable using one continuous force axis.
    int center = 4;
    for (int i = 0; i <= row; ++i) {
        const int selector = (i * 7 + m_level * 11 + i * i * 3) % 5;
        if (selector == 0) center -= 1;
        else if (selector == 3) center += 1;
        center = qBound(1, center, kColumns - 2);
    }
    return center;
}

bool ForceMazeGame::corridorContains(double column, int row) const
{
    return std::abs(column - static_cast<double>(corridorCenter(row))) <= kCorridorHalfWidth;
}

void ForceMazeGame::initialize()
{
    QSettings settings(QStringLiteral("Biofeedback"), QStringLiteral("Biofeedback"));
    m_personalBestScore = settings.value(QStringLiteral("games/force_maze/records/best_score"), 0).toInt();
    m_personalBestCrystals = settings.value(QStringLiteral("games/force_maze/records/best_crystals"), 0).toInt();
    m_personalBestLevel = settings.value(QStringLiteral("games/force_maze/records/best_level"), 1).toInt();
    m_achievements = settings.value(QStringLiteral("games/force_maze/achievements")).toStringList();
    m_leftHanded = settings.value(QStringLiteral("games/force_maze/left_handed"), false).toBool();

    m_level = 1;
    m_lives = 3;
    m_crystals = 0;
    m_totalCrystals = 0;
    for (int level = 1; level <= kCampaignLevels; ++level) {
        for (int row = 0; row < kRowsPerLevel; ++row) {
            if ((row + level * 2) % 3 == 1)
                ++m_totalCrystals;
        }
    }
    m_checkpoints = 0;
    m_lastProcessedRow = -1;
    m_progressRows = 0.0;
    m_playerColumn = 4.0;
    m_targetColumn = 4.0;
    m_inputAge = kSignalTimeoutSeconds + 1.0;
    m_visualTime = 0.0;
    m_forwardSpeed = 0.95;
    m_hitFlash = 0.0;
    m_checkpointFlash = 0.0;
    m_crystalPulse = 0.0;
    m_hasSensorSample = false;
    m_awaitingNeutral = true;
    m_campaignCompleted = false;
    m_levelClean = true;
    m_campaignClean = true;
    m_cleanLevels = 0;
    m_campaignDeaths = 0;
    m_playerTrailPhase = 0.0;
    m_currentForce = 0.0;
    m_cameraShake = 0.0;
    m_status = QStringLiteral("Zwolnij tensometr do strefy neutralnej");
    m_collectedCrystals.clear();
    m_reachedCheckpoints.clear();
    setScore(0);
}

bool ForceMazeGame::handleInput(double value)
{
    if (!std::isfinite(value))
        return false;

    const double force = qBound(0.0, value, 100.0);
    m_currentForce = force;
    m_hasSensorSample = true;
    m_inputAge = 0.0;

    const bool inNeutralBand = force >= kNeutralMin && force <= kNeutralMax;
    if (m_awaitingNeutral) {
        if (inNeutralBand) {
            m_awaitingNeutral = false;
            m_targetColumn = 4.0;
            m_status = QStringLiteral("Gotowy — strefa neutralna 10–30%, sterowanie aktywne");
        } else {
            m_status = QStringLiteral("Ustaw nacisk w szerokiej strefie neutralnej 10–30%");
        }
        return true;
    }

    // Wide dead zone: values from 10% to 30% hold the current target and
    // cannot cause drift. Outside it, force is mapped continuously to the
    // two sides. The handedness setting reverses the horizontal direction.
    if (inNeutralBand)
        return true;

    double normalized = force < kNeutralMin
        ? 0.5 * force / kNeutralMin
        : 0.5 + 0.5 * (force - kNeutralMax) / (100.0 - kNeutralMax);
    normalized = qBound(0.0, normalized, 1.0);
    if (m_leftHanded)
        normalized = 1.0 - normalized;
    m_targetColumn = 1.0 + normalized * static_cast<double>(kColumns - 3);
    return true;
}

void ForceMazeGame::setLeftHanded(bool leftHanded)
{
    if (m_leftHanded == leftHanded)
        return;
    m_leftHanded = leftHanded;
    m_targetColumn = 4.0;
    m_awaitingNeutral = true;
    m_status = QStringLiteral("Zmieniono rękę — ustaw nacisk w strefie neutralnej 10–30%");
    QSettings settings(QStringLiteral("Biofeedback"), QStringLiteral("Biofeedback"));
    settings.setValue(QStringLiteral("games/force_maze/left_handed"), m_leftHanded);
    settings.sync();
}

void ForceMazeGame::mousePressEvent(QMouseEvent *event)
{
    if (!event)
        return;
    const qreal margin = qMax<qreal>(14.0, width() * 0.045);
    const QRectF handButton(width() - margin - 132.0, 5.0, 132.0, 24.0);
    if (handButton.contains(event->pos())) {
        setLeftHanded(!m_leftHanded);
        QWidget::update();
        event->accept();
        return;
    }
    event->ignore();
}

void ForceMazeGame::unlockAchievement(const QString &id)
{
    if (m_achievements.contains(id))
        return;
    m_achievements.append(id);
    QSettings settings(QStringLiteral("Biofeedback"), QStringLiteral("Biofeedback"));
    settings.setValue(QStringLiteral("games/force_maze/achievements"), m_achievements);
    m_status = QStringLiteral("Odblokowano osiągnięcie: %1").arg(id);
}

void ForceMazeGame::saveRecords()
{
    QSettings settings(QStringLiteral("Biofeedback"), QStringLiteral("Biofeedback"));
    if (getScore() > m_personalBestScore)
        m_personalBestScore = getScore();
    if (m_crystals > m_personalBestCrystals)
        m_personalBestCrystals = m_crystals;
    if (m_level > m_personalBestLevel)
        m_personalBestLevel = m_level;
    settings.setValue(QStringLiteral("games/force_maze/records/best_score"), m_personalBestScore);
    settings.setValue(QStringLiteral("games/force_maze/records/best_crystals"), m_personalBestCrystals);
    settings.setValue(QStringLiteral("games/force_maze/records/best_level"), m_personalBestLevel);
    settings.sync();
}

void ForceMazeGame::collectCrystal(int row)
{
    if (m_collectedCrystals.contains(row))
        return;
    m_collectedCrystals.append(row);
    ++m_crystals;
    addScore(100 + m_level * 15);
    m_crystalPulse = 0.45;
    if (m_crystals == 1)
        unlockAchievement(QStringLiteral("first_crystal"));
    if (m_crystals >= 10)
        unlockAchievement(QStringLiteral("crystal_collector"));
    m_status = QStringLiteral("Kryształ zebrany — +%1 pkt").arg(100 + m_level * 15);
}

void ForceMazeGame::reachCheckpoint(int row)
{
    if (m_reachedCheckpoints.contains(row))
        return;
    m_reachedCheckpoints.append(row);
    ++m_checkpoints;
    addScore(150);
    m_checkpointFlash = 0.8;
    m_status = QStringLiteral("Punkt kontrolny osiągnięty");
    unlockAchievement(QStringLiteral("checkpoint"));
}

void ForceMazeGame::hitWall(int row)
{
    Q_UNUSED(row);
    --m_lives;
    ++m_campaignDeaths;
    m_levelClean = false;
    m_campaignClean = false;
    m_hitFlash = 0.65;
    m_cameraShake = 0.24;
    addScore(-25);
    const int checkpoint = (m_lastProcessedRow / 5) * 5;
    m_progressRows = qMax(0, checkpoint);
    m_playerColumn = static_cast<double>(corridorCenter(qMax(0, checkpoint)));
    m_targetColumn = m_playerColumn;
    m_lastProcessedRow = checkpoint - 1;
    m_status = QStringLiteral("Uderzenie w ścianę — powrót do punktu kontrolnego");
    if (m_lives <= 0) {
        saveRecords();
        stop();
    }
}

void ForceMazeGame::processRowCrossing(int row)
{
    const int center = corridorCenter(row);
    const bool gateRow = (row % 6) == 4;
    if (gateRow && std::abs(m_playerColumn - static_cast<double>(center)) > 0.62) {
        hitWall(row);
        return;
    }
    const bool hazardRow = ((row + m_level) % 4) == 2;
    const int hazardColumn = center + (((row / 4) % 2) == 0 ? -1 : 1);
    if (hazardRow && std::abs(m_playerColumn - static_cast<double>(hazardColumn)) < 0.48) {
        hitWall(row);
        return;
    }
    if (!corridorContains(m_playerColumn, row)) {
        hitWall(row);
        return;
    }

    // Crystal locations are deterministic and lie inside the safe corridor.
    if ((row + m_level * 2) % 3 == 1)
        collectCrystal(row);
    if (row > 0 && row % 5 == 0)
        reachCheckpoint(row);
}

void ForceMazeGame::completeLevel()
{
    if (m_levelClean) {
        ++m_cleanLevels;
        unlockAchievement(QStringLiteral("clean_level"));
        if (m_cleanLevels >= 3)
            unlockAchievement(QStringLiteral("three_clean_levels"));
    }
    if (m_level >= 3)
        unlockAchievement(QStringLiteral("level_three"));
    if (m_level >= 5)
        unlockAchievement(QStringLiteral("level_five"));
    addScore(250 * m_level);
    if (m_level >= kCampaignLevels) {
        finishCampaign();
        return;
    }
    ++m_level;
    m_progressRows = 0.0;
    m_lastProcessedRow = -1;
    m_collectedCrystals.clear();
    m_reachedCheckpoints.clear();
    m_levelClean = true;
    m_forwardSpeed = 0.95 + 0.08 * static_cast<double>(m_level - 1);
    m_playerColumn = 4.0;
    m_targetColumn = 4.0;
    m_status = QStringLiteral("Poziom %1 — labirynt przyspiesza").arg(m_level);
}

void ForceMazeGame::finishCampaign()
{
    m_campaignCompleted = true;
    addScore(1000);
    unlockAchievement(QStringLiteral("maze_master"));
    if (m_campaignClean)
        unlockAchievement(QStringLiteral("untouched_campaign"));
    if (m_crystals >= 25)
        unlockAchievement(QStringLiteral("crystal_master"));
    saveRecords();
    m_status = QStringLiteral("Kampania ukończona");
    stop();
}

void ForceMazeGame::update(double deltaTime)
{
    if (!std::isfinite(deltaTime) || deltaTime <= 0.0)
        return;

    m_visualTime += deltaTime;
    m_hitFlash = qMax(0.0, m_hitFlash - deltaTime);
    m_checkpointFlash = qMax(0.0, m_checkpointFlash - deltaTime);
    m_crystalPulse = qMax(0.0, m_crystalPulse - deltaTime);
    m_cameraShake = qMax(0.0, m_cameraShake - deltaTime);
    m_playerTrailPhase += deltaTime * 8.0;
    if (m_inputAge <= kSignalTimeoutSeconds)
        m_inputAge += deltaTime;

    if (m_hasSensorSample && m_inputAge > kSignalTimeoutSeconds) {
        m_awaitingNeutral = true;
        m_status = QStringLiteral("Brak świeżego sygnału — ruch zatrzymany");
    }

    // Animation may continue, but all player/world progression is fail-closed.
    if (!hasFreshSignal() || m_campaignCompleted || m_lives <= 0)
        return;

    const double smoothing = 1.0 - std::exp(-deltaTime * 12.0);
    m_playerColumn += (m_targetColumn - m_playerColumn) * smoothing;
    m_progressRows += m_forwardSpeed * deltaTime;

    const int currentRow = static_cast<int>(std::floor(m_progressRows));
    while (m_lastProcessedRow < currentRow && m_lastProcessedRow < kRowsPerLevel - 1) {
        ++m_lastProcessedRow;
        processRowCrossing(m_lastProcessedRow);
        if (m_lives <= 0 || !m_isRunning)
            return;
        // A wall collision rewinds progress to the last checkpoint. Do not
        // continue processing rows from the pre-collision frame position.
        if (static_cast<int>(std::floor(m_progressRows)) != currentRow)
            return;
    }

    if (m_progressRows >= static_cast<double>(kRowsPerLevel))
        completeLevel();
}

void ForceMazeGame::render(QPainter &painter)
{
    const QRectF bounds = rect();
    painter.fillRect(bounds, QColor(11, 28, 43));
    painter.save();
    if (m_cameraShake > 0.0) {
        const qreal shake = m_cameraShake * 9.0;
        painter.translate(std::sin(m_visualTime * 73.0) * shake,
                          std::cos(m_visualTime * 61.0) * shake * 0.55);
    }

    const qreal margin = qMax<qreal>(14.0, width() * 0.045);
    const qreal hudHeight = qMax<qreal>(58.0, height() * 0.15);
    const QRectF board(margin, hudHeight, width() - 2.0 * margin,
                       height() - hudHeight - qMax<qreal>(48.0, height() * 0.12));
    const qreal cellW = board.width() / kColumns;
    const qreal cellH = board.height() / 8.0;

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(16, 34, 48, 225));
    painter.drawRoundedRect(board, 14, 14);

    // Perspective tunnel: the far end narrows toward a vanishing point.
    // Row depth controls tile width, horizontal spacing, vertical spacing,
    // lighting and object scale to create an actual pseudo-3D corridor.
    const int baseRow = static_cast<int>(std::floor(m_progressRows));
    const double fractional = m_progressRows - baseRow;
    const qreal horizonX = board.center().x();
    const qreal horizonY = board.top() + board.height() * 0.08;
    for (int visible = 0; visible < 10; ++visible) {
        const int row = baseRow + visible - 2;
        if (row < 0 || row >= kRowsPerLevel)
            continue;
        const qreal depth = qBound<qreal>(0.12, (static_cast<qreal>(visible) + 0.2 - fractional) / 8.0, 1.0);
        const qreal perspective = 0.34 + 0.66 * depth;
        const qreal y = horizonY + (board.bottom() - horizonY) * depth;
        const qreal rowHeight = cellH * perspective;
        const qreal tileW = cellW * perspective;
        const int center = corridorCenter(row);
        const qreal rowCenterX = horizonX + (center - 4.0) * cellW * perspective;
        for (int col = 0; col < kColumns; ++col) {
            const bool safe = std::abs(col - center) <= 1;
            const qreal x = rowCenterX + (col - center) * tileW - tileW * 0.5;
            const QRectF cell(x, y - rowHeight * 0.5, tileW - 2.0, rowHeight - 2.0);
            const int shade = static_cast<int>(48 + 70 * depth);
            painter.setBrush(safe ? QColor(20, shade + 28, shade + 34) : QColor(shade - 8, shade, shade + 12));
            painter.setPen(QPen(safe ? QColor(70, 190, 188, 90 + static_cast<int>(120 * depth))
                                     : QColor(75, 100, 120, 80 + static_cast<int>(100 * depth)),
                                0.7 + 1.0 * depth));
            painter.drawRoundedRect(cell, 3.0 + 4.0 * depth, 3.0 + 4.0 * depth);
            if (!safe && depth > 0.45) {
                painter.setPen(QColor(105, 132, 150, static_cast<int>(40 + 60 * depth)));
                painter.drawLine(cell.topLeft(), cell.bottomRight());
            }
        }

        if ((row + m_level * 2) % 3 == 1 && !m_collectedCrystals.contains(row)) {
            const QPointF crystal(rowCenterX + 0.5 * tileW, y);
            const qreal radius = (3.0 + 5.0 * depth) * (1.0 + 0.12 * std::sin(m_visualTime * 5.0 + row));
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(62, 225, 244, static_cast<int>(40 + 65 * depth)));
            painter.drawEllipse(crystal, radius * 1.8, radius * 1.8);
            QPolygonF gem;
            gem << QPointF(crystal.x(), crystal.y() - radius)
                << QPointF(crystal.x() + radius * 0.65, crystal.y())
                << QPointF(crystal.x(), crystal.y() + radius)
                << QPointF(crystal.x() - radius * 0.65, crystal.y());
            painter.setPen(QPen(QColor(180, 250, 255), 1.0 + depth));
            painter.setBrush(QColor(74, 219, 235));
            painter.drawPolygon(gem);
        }

        if (((row + m_level) % 4) == 2) {
            const int hazardColumn = center + (((row / 4) % 2) == 0 ? -1 : 1);
            const QPointF hazard(rowCenterX + (hazardColumn - center + 0.5) * tileW, y);
            QPolygonF spike;
            spike << QPointF(hazard.x(), hazard.y() - rowHeight * 0.28)
                  << QPointF(hazard.x() + tileW * 0.22, hazard.y() + rowHeight * 0.20)
                  << QPointF(hazard.x() - tileW * 0.22, hazard.y() + rowHeight * 0.20);
            painter.setPen(QPen(QColor(255, 150, 120), 0.8 + depth));
            painter.setBrush(QColor(215, 65, 68));
            painter.drawPolygon(spike);
        }

        if ((row % 6) == 4) {
            painter.setPen(QPen(QColor(255, 196, 100, static_cast<int>(100 + 150 * depth)), 1.5 + 2.0 * depth));
            const qreal gateY = y;
            painter.drawLine(rowCenterX - 1.25 * tileW, gateY,
                             rowCenterX - 0.62 * tileW, gateY);
            painter.drawLine(rowCenterX + 0.62 * tileW, gateY,
                             rowCenterX + 1.25 * tileW, gateY);
        }

        if (row > 0 && row % 5 == 0) {
            painter.setPen(QPen(QColor(255, 210, 92, static_cast<int>(90 + 150 * depth)),
                                1.0 + 1.3 * depth, Qt::DashLine));
            painter.drawLine(rowCenterX - 1.35 * tileW, y,
                             rowCenterX + 1.35 * tileW, y);
        }
    }

    // Player is anchored near the foreground; its perspective-scaled glow,
    // rolling highlight and short trailing particles react to movement.
    const qreal playerDepth = 0.92;
    const qreal playerScale = 0.34 + 0.66 * playerDepth;
    const qreal playerX = horizonX + (m_playerColumn - 4.0) * cellW * playerScale;
    const qreal playerY = horizonY + (board.bottom() - horizonY) * 0.72;
    const qreal ballRadius = qMax<qreal>(7.0, cellW * 0.19 * playerScale);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(52, 218, 205, 35));
    painter.drawEllipse(QPointF(playerX, playerY), ballRadius * 2.2, ballRadius * 2.2);
    for (int particle = 0; particle < 4; ++particle) {
        const qreal phase = std::fmod(m_playerTrailPhase + particle * 1.7, 6.0);
        const qreal alpha = qMax<qreal>(0.0, 150.0 * (1.0 - phase / 6.0));
        const qreal offset = (particle + 1) * 7.0;
        painter.setBrush(QColor(65, 225, 220, static_cast<int>(alpha)));
        painter.drawEllipse(QPointF(playerX, playerY + offset), ballRadius * (0.16 + 0.05 * particle),
                            ballRadius * (0.16 + 0.05 * particle));
    }
    painter.setBrush(QColor(92, 247, 229));
    painter.drawEllipse(QPointF(playerX, playerY), ballRadius, ballRadius);
    painter.setPen(QPen(QColor(200, 255, 250, 220), 1.2));
    painter.setBrush(QColor(255, 255, 255, 210));
    painter.drawEllipse(QPointF(playerX - ballRadius * 0.28, playerY - ballRadius * 0.32),
                        ballRadius * 0.25, ballRadius * 0.25);
    painter.setPen(QPen(QColor(50, 160, 170, 180), 1.0));
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(QRectF(playerX - ballRadius * 0.72, playerY - ballRadius * 0.72,
                           ballRadius * 1.44, ballRadius * 1.44),
                    static_cast<int>(m_visualTime * 800) % 5760, 2300);
    painter.restore();

    painter.setPen(Qt::white);
    painter.setFont(QFont(QStringLiteral("Sans Serif"), qMax(8, height() / 42), QFont::DemiBold));
    painter.drawText(QRectF(margin, 8, width() - 2 * margin, 22),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     QStringLiteral("FORCE MAZE   •   POZIOM %1/%2").arg(m_level).arg(kCampaignLevels));
    painter.drawText(QRectF(margin, 30, width() - 2 * margin, 22),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     QStringLiteral("Wynik %1    Kryształy %2/%3    Życia %4    Punkty kontrolne %5")
                         .arg(getScore()).arg(m_crystals).arg(m_totalCrystals)
                         .arg(m_lives).arg(m_checkpoints));
    const QRectF forceTrack(margin, 54, width() - 2 * margin, 5);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(45, 67, 78));
    painter.drawRoundedRect(forceTrack, 2.5, 2.5);
    const QRectF forceFill(forceTrack.left(), forceTrack.top(),
                           forceTrack.width() * m_currentForce / 100.0, forceTrack.height());
    painter.setBrush(QColor(77, 224, 211));
    painter.drawRoundedRect(forceFill, 2.5, 2.5);
    const qreal neutralLeft = forceTrack.left() + forceTrack.width() * kNeutralMin / 100.0;
    const qreal neutralRight = forceTrack.left() + forceTrack.width() * kNeutralMax / 100.0;
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 220, 130, 95));
    painter.drawRect(QRectF(neutralLeft, forceTrack.top() - 2,
                            neutralRight - neutralLeft, forceTrack.height() + 4));
    painter.setPen(QPen(QColor(255, 220, 130), 1.2));
    painter.drawLine(QPointF(neutralLeft, forceTrack.top() - 3),
                     QPointF(neutralLeft, forceTrack.bottom() + 3));
    painter.drawLine(QPointF(neutralRight, forceTrack.top() - 3),
                     QPointF(neutralRight, forceTrack.bottom() + 3));
    const qreal forceX = forceTrack.left() + forceTrack.width() * m_currentForce / 100.0;
    painter.setPen(QPen(QColor(255, 255, 255), 1.5));
    painter.drawLine(QPointF(forceX, forceTrack.top() - 4),
                     QPointF(forceX, forceTrack.bottom() + 4));

    const QRectF handButton(width() - margin - 132.0, 5.0, 132.0, 24.0);
    painter.setPen(QPen(QColor(112, 206, 213), 1.0));
    painter.setBrush(QColor(20, 57, 70, 230));
    painter.drawRoundedRect(handButton, 7.0, 7.0);
    painter.setPen(Qt::white);
    painter.setFont(QFont(QStringLiteral("Sans Serif"), qMax(8, height() / 48), QFont::DemiBold));
    painter.drawText(handButton, Qt::AlignCenter,
                     m_leftHanded ? QStringLiteral("Ręka: lewa") : QStringLiteral("Ręka: prawa"));

    const QRectF footer(margin, height() - 38, width() - 2 * margin, 28);
    painter.setPen(QColor(180, 211, 220));
    const QString state = hasFreshSignal()
        ? QStringLiteral("Strefa neutralna 10–30% — poza nią siła reguluje położenie; kliknij u góry, aby zmienić rękę")
        : QStringLiteral("PAUZA — oczekiwanie na świeży sygnał i nacisk w strefie neutralnej 10–30%");
    painter.drawText(footer, Qt::AlignCenter, m_status.isEmpty() ? state : m_status);

    if (m_hitFlash > 0.0) {
        painter.fillRect(bounds, QColor(255, 70, 65, static_cast<int>(90.0 * m_hitFlash / 0.65)));
    }
    if (m_checkpointFlash > 0.0) {
        painter.setPen(QPen(QColor(255, 215, 110, 220), 3.0));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(board.adjusted(3, 3, -3, -3), 14, 14);
    }
    if (m_campaignCompleted || m_lives <= 0) {
        painter.fillRect(bounds, QColor(4, 10, 18, 205));
        painter.setPen(Qt::white);
        painter.setFont(QFont(QStringLiteral("Sans Serif"), qMax(14, height() / 18), QFont::Bold));
        painter.drawText(board, Qt::AlignCenter,
                         m_campaignCompleted ? QStringLiteral("LABIRYNT POKONANY")
                                             : QStringLiteral("KONIEC WYPRAWY"));
        painter.setFont(QFont(QStringLiteral("Sans Serif"), qMax(9, height() / 34)));
        painter.drawText(QRectF(board.left(), board.center().y() + 34, board.width(), 32),
                         Qt::AlignCenter,
                         QStringLiteral("Wynik %1   Rekord %2   Kryształy %3")
                             .arg(getScore()).arg(m_personalBestScore).arg(m_crystals));
    }
}

} // namespace games
