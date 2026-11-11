#include "games/ForceMazeGame.hpp"

#include <QLinearGradient>
#include <QPainter>
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
    , m_progressRows(0.0)
    , m_playerColumn(4.0)
    , m_targetColumn(4.0)
    , m_inputAge(kSignalTimeoutSeconds + 1.0)
    , m_visualTime(0.0)
    , m_forwardSpeed(1.35)
    , m_hitFlash(0.0)
    , m_checkpointFlash(0.0)
    , m_crystalPulse(0.0)
    , m_hasSensorSample(false)
    , m_awaitingNeutral(true)
    , m_campaignCompleted(false)
    , m_levelClean(true)
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
    m_achievements = settings.value(QStringLiteral("games/force_maze/achievements")).toStringList();

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
    m_forwardSpeed = 1.35;
    m_hitFlash = 0.0;
    m_checkpointFlash = 0.0;
    m_crystalPulse = 0.0;
    m_hasSensorSample = false;
    m_awaitingNeutral = true;
    m_campaignCompleted = false;
    m_levelClean = true;
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
    m_hasSensorSample = true;
    m_inputAge = 0.0;

    if (m_awaitingNeutral) {
        if (force <= kNeutralThreshold) {
            m_awaitingNeutral = false;
            m_status = QStringLiteral("Gotowy — steruj położeniem kulki siłą nacisku");
        } else {
            m_status = QStringLiteral("Zwolnij nacisk, aby ponownie uzbroić sterowanie");
        }
        return true;
    }

    // The neutral band maps to the centre of the corridor. Above it, one
    // calibrated force value controls only the horizontal coordinate.
    const double normalized = force <= kNeutralThreshold
        ? 0.5
        : qBound(0.0, (force - kNeutralThreshold) / (100.0 - kNeutralThreshold), 1.0);
    m_targetColumn = 1.0 + normalized * static_cast<double>(kColumns - 3);
    return true;
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
    settings.setValue(QStringLiteral("games/force_maze/records/best_score"), m_personalBestScore);
    settings.setValue(QStringLiteral("games/force_maze/records/best_crystals"), m_personalBestCrystals);
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
    m_levelClean = false;
    m_hitFlash = 0.65;
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
    if (m_levelClean)
        unlockAchievement(QStringLiteral("clean_level"));
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
    m_forwardSpeed = 1.35 + 0.18 * static_cast<double>(m_level - 1);
    m_playerColumn = 4.0;
    m_targetColumn = 4.0;
    m_status = QStringLiteral("Poziom %1 — labirynt przyspiesza").arg(m_level);
}

void ForceMazeGame::finishCampaign()
{
    m_campaignCompleted = true;
    addScore(1000);
    unlockAchievement(QStringLiteral("maze_master"));
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
    if (m_inputAge <= kSignalTimeoutSeconds)
        m_inputAge += deltaTime;

    if (m_hasSensorSample && m_inputAge > kSignalTimeoutSeconds) {
        m_awaitingNeutral = true;
        m_status = QStringLiteral("Brak świeżego sygnału — ruch zatrzymany");
    }

    // Animation may continue, but all player/world progression is fail-closed.
    if (!hasFreshSignal() || m_campaignCompleted || m_lives <= 0)
        return;

    const double smoothing = 1.0 - std::exp(-deltaTime * 9.0);
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
    QLinearGradient background(0, 0, 0, height());
    background.setColorAt(0.0, QColor(8, 18, 34));
    background.setColorAt(1.0, QColor(17, 42, 55));
    painter.fillRect(bounds, background);

    const qreal margin = qMax<qreal>(14.0, width() * 0.045);
    const qreal hudHeight = qMax<qreal>(58.0, height() * 0.15);
    const QRectF board(margin, hudHeight, width() - 2.0 * margin,
                       height() - hudHeight - qMax<qreal>(48.0, height() * 0.12));
    const qreal cellW = board.width() / kColumns;
    const qreal cellH = board.height() / 8.0;

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(16, 34, 48, 225));
    painter.drawRoundedRect(board, 14, 14);

    // Draw a scrolling, top-down maze. The corridor shifts by one cell at a
    // time; solid side blocks create a readable winding route and depth cues.
    const int baseRow = static_cast<int>(std::floor(m_progressRows));
    const double fractional = m_progressRows - baseRow;
    for (int visible = 0; visible < 9; ++visible) {
        const int row = baseRow + visible - 2;
        if (row < 0 || row >= kRowsPerLevel)
            continue;
        const qreal y = board.top() + (static_cast<qreal>(visible) - 1.0 - fractional) * cellH;
        const int center = corridorCenter(row);
        for (int col = 0; col < kColumns; ++col) {
            const bool safe = std::abs(col - center) <= 1;
            const QRectF cell(board.left() + col * cellW, y, cellW - 2.0, cellH - 2.0);
            painter.setBrush(safe ? QColor(29, 82, 91) : QColor(32, 48, 62));
            painter.setPen(QPen(safe ? QColor(72, 188, 180) : QColor(56, 78, 92), 1.0));
            painter.drawRoundedRect(cell, 4, 4);
            if (!safe) {
                painter.setPen(QColor(80, 104, 117, 90));
                painter.drawLine(cell.topLeft(), cell.bottomRight());
            }
        }

        if ((row + m_level * 2) % 3 == 1 && !m_collectedCrystals.contains(row)) {
            const QPointF crystal(board.left() + (center + 0.5) * cellW,
                                  y + cellH * 0.5);
            const qreal radius = 5.0 + 2.0 * std::sin(m_visualTime * 5.0);
            QPolygonF gem;
            gem << QPointF(crystal.x(), crystal.y() - radius)
                << QPointF(crystal.x() + radius * 0.65, crystal.y())
                << QPointF(crystal.x(), crystal.y() + radius)
                << QPointF(crystal.x() - radius * 0.65, crystal.y());
            painter.setPen(QPen(QColor(156, 248, 255), 1.5));
            painter.setBrush(QColor(74, 219, 235));
            painter.drawPolygon(gem);
        }

        if (((row + m_level) % 4) == 2) {
            const int hazardColumn = center + (((row / 4) % 2) == 0 ? -1 : 1);
            const QPointF hazard(board.left() + (hazardColumn + 0.5) * cellW,
                                 y + cellH * 0.5);
            QPolygonF spike;
            spike << QPointF(hazard.x(), hazard.y() - cellH * 0.28)
                  << QPointF(hazard.x() + cellW * 0.22, hazard.y() + cellH * 0.20)
                  << QPointF(hazard.x() - cellW * 0.22, hazard.y() + cellH * 0.20);
            painter.setPen(QPen(QColor(255, 126, 112), 1.2));
            painter.setBrush(QColor(225, 75, 72));
            painter.drawPolygon(spike);
        }

        if ((row % 6) == 4) {
            painter.setPen(QPen(QColor(255, 192, 92), 3.0));
            const qreal gateY = y + cellH * 0.5;
            painter.drawLine(board.left() + (center - 1.25) * cellW, gateY,
                             board.left() + (center - 0.62) * cellW, gateY);
            painter.drawLine(board.left() + (center + 0.62) * cellW, gateY,
                             board.left() + (center + 1.25) * cellW, gateY);
        }

        if (row > 0 && row % 5 == 0) {
            painter.setPen(QPen(QColor(255, 210, 92), 2.0, Qt::DashLine));
            painter.drawLine(board.left() + (center - 1.35) * cellW, y + cellH * 0.5,
                             board.left() + (center + 1.35) * cellW, y + cellH * 0.5);
        }
    }

    // Player ball and soft trail.
    const qreal playerX = board.left() + (m_playerColumn + 0.5) * cellW;
    const qreal playerY = board.top() + board.height() * 0.69;
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(52, 218, 205, 45));
    painter.drawEllipse(QPointF(playerX, playerY), cellW * 0.42, cellW * 0.42);
    painter.setBrush(QColor(92, 247, 229));
    painter.drawEllipse(QPointF(playerX, playerY), qMax<qreal>(6.0, cellW * 0.22),
                        qMax<qreal>(6.0, cellW * 0.22));
    painter.setBrush(QColor(255, 255, 255, 210));
    painter.drawEllipse(QPointF(playerX - cellW * 0.06, playerY - cellW * 0.07),
                        qMax<qreal>(2.0, cellW * 0.055), qMax<qreal>(2.0, cellW * 0.055));

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

    const QRectF footer(margin, height() - 38, width() - 2 * margin, 28);
    painter.setPen(QColor(180, 211, 220));
    const QString state = hasFreshSignal()
        ? QStringLiteral("Sterowanie aktywne — siła reguluje położenie poziome")
        : QStringLiteral("PAUZA BEZPIECZEŃSTWA — oczekiwanie na świeży sygnał i neutralny nacisk");
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
