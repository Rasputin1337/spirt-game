#include "gamelogic.h"
#include <QRandomGenerator>
#include <algorithm>

GameLogic::GameLogic(QObject *parent) : QObject(parent), m_currentIndex(0)
{
    m_palette = {Qt::red, Qt::blue, Qt::green, Qt::yellow, Qt::magenta, Qt::cyan};
}

void GameLogic::startLevel(int colorCount)
{
    m_sequence.clear();
    m_currentIndex = 0;

    QVector<QColor> levelColors;
    for(int i = 0; i < colorCount && i < m_palette.size(); ++i) {
        levelColors.append(m_palette[i]);
    }

    for (const QColor& color : levelColors) {
        for(int j = 0; j < 4; ++j) {
            m_sequence.append(color);
        }
    }
}

bool GameLogic::checkClick(QColor color)
{
    if (m_currentIndex >= m_sequence.size()) return false;
    if (m_sequence[m_currentIndex] == color) {
        m_currentIndex++;
        return true;
    }
    return false;
}

QColor GameLogic::getCurrentTargetColor() const
{
    if (m_currentIndex < m_sequence.size()) {
        return m_sequence[m_currentIndex];
    }
    return QColor();
}

bool GameLogic::isLevelComplete() const
{
    return m_currentIndex >= m_sequence.size();
}

QVector<QColor> GameLogic::getAllColorsForLevel() const
{
    return m_sequence;
}