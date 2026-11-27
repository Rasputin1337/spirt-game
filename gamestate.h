#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <QColor>
#include <QVector>
#include <QtTypes>

class GameLogic;

class GameState
{
public:
    enum class Status { Ready, Active, Completed, Abandoned };

    static bool isValidDifficulty(int colorCount);

    int difficulty() const { return m_difficulty; }
    int cleared() const { return m_position; }
    int tileCount() const { return int(m_sequence.size()); }
    int mistakes() const { return m_mistakes; }
    Status status() const { return m_status; }
    qint64 finishTime() const { return m_finishTime; }
    qint64 bestTime() const { return m_bestTime; }

private:
    friend class GameLogic;

    int m_difficulty = 2;
    QVector<QColor> m_sequence;
    int m_position = 0;
    int m_mistakes = 0;
    Status m_status = Status::Ready;
    qint64 m_finishTime = 0;
    qint64 m_bestTime = 0;
};

#endif // GAMESTATE_H
