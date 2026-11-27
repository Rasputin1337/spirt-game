#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <QObject>
#include "gamestate.h"

class GameLogic : public QObject
{
    Q_OBJECT
public:
    enum class ClickResult { Ignored, Mistake, Correct, Completed, NewRecord };

    explicit GameLogic(QObject *parent = nullptr);
    QVector<QColor> startLevel(int colorCount, qint64 bestTime);
    void abandonRound();
    ClickResult checkClick(QColor color, qint64 elapsedMs);
    QColor getCurrentTargetColor() const;
    const GameState &state() const { return m_state; }

private:
    GameState m_state;
};

#endif // GAMELOGIC_H
