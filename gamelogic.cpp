#include "gamelogic.h"
#include <QtMinMax>
#include <algorithm>
#include <random>
#include <utility>

GameLogic::GameLogic(QObject *parent) : QObject(parent)
{}

QVector<QColor> GameLogic::startLevel(int colorCount, qint64 bestTime)
{
    if (!GameState::isValidDifficulty(colorCount))
        return {};

    QVector<QColor> levelColors = {Qt::red, Qt::blue, Qt::green, Qt::yellow};
    levelColors.resize(colorCount);
    std::mt19937 generator(std::random_device{}());
    std::shuffle(levelColors.begin(), levelColors.end(), generator);

    GameState next;
    next.m_difficulty = colorCount;
    next.m_bestTime = bestTime;
    next.m_status = GameState::Status::Active;
    for (const QColor &color : levelColors) {
        for (int i = 0; i < 4; ++i)
            next.m_sequence.append(color);
    }

    auto board = next.m_sequence;
    std::mt19937 boardGenerator(std::random_device{}());
    std::shuffle(board.begin(), board.end(), boardGenerator);
    m_state = std::move(next);
    return board;
}

void GameLogic::abandonRound()
{
    if (m_state.m_status == GameState::Status::Active)
        m_state.m_status = GameState::Status::Abandoned;
}

GameLogic::ClickResult GameLogic::checkClick(QColor color, qint64 elapsedMs)
{
    if (m_state.m_status != GameState::Status::Active)
        return ClickResult::Ignored;

    if (color != getCurrentTargetColor()) {
        ++m_state.m_mistakes;
        return ClickResult::Mistake;
    }

    if (++m_state.m_position < m_state.m_sequence.size())
        return ClickResult::Correct;

    m_state.m_status = GameState::Status::Completed;
    m_state.m_finishTime = qMax<qint64>(1, elapsedMs);
    if (m_state.m_mistakes == 0
        && (m_state.m_bestTime <= 0 || m_state.m_finishTime < m_state.m_bestTime)) {
        m_state.m_bestTime = m_state.m_finishTime;
        return ClickResult::NewRecord;
    }
    return ClickResult::Completed;
}

QColor GameLogic::getCurrentTargetColor() const
{
    return m_state.m_status == GameState::Status::Active
        ? m_state.m_sequence[m_state.m_position] : QColor();
}
