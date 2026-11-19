#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <QObject>
#include <QColor>
#include <QVector>

class GameLogic : public QObject
{
    Q_OBJECT
public:
    explicit GameLogic(QObject *parent = nullptr);

    void startLevel(int colorCount);
    bool checkClick(QColor color);
    QColor getCurrentTargetColor() const;
    bool isLevelComplete() const;
    QVector<QColor> getAllColorsForLevel() const;

private:
    QVector<QColor> m_sequence;
    int m_currentIndex;
    QVector<QColor> m_palette;
};

#endif // GAMELOGIC_H