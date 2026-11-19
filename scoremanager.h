#ifndef SCOREMANAGER_H
#define SCOREMANAGER_H

#include <QObject>

class ScoreManager : public QObject
{
    Q_OBJECT
public:
    explicit ScoreManager(QObject *parent = nullptr);

signals:
};

#endif // SCOREMANAGER_H
