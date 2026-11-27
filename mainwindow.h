#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QElapsedTimer>
#include <QMainWindow>
#include <QtTypes>
#include "gamelogic.h"

class ErrorCountLabel;
class HomeWidget;
class QGridLayout;
class QLabel;
class QProgressBar;
class QTimer;
class QPushButton;
class QStackedWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void createMenus();
    void showHome();
    void startNewGame(int colorCount);
    void onTabClicked();
    void updateRound();
    void updateStats();
    void finishRound(bool newRecord);
    qint64 elapsedMs() const;

    GameLogic *gameLogic;
    QStackedWidget *pages;
    HomeWidget *home;
    QWidget *game;
    QGridLayout *board;
    QLabel *target;
    QLabel *targetCaption;
    QLabel *swatch;
    QLabel *tileCount;
    ErrorCountLabel *errorCount;
    QLabel *timeAndRecord;
    QProgressBar *progress;
    QPushButton *restart;
    QTimer *timer;
    QElapsedTimer clock;
};

#endif // MAINWINDOW_H
