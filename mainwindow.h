#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QElapsedTimer>
#include <QMainWindow>
#include <QtTypes>
#include <array>
#include "gamelogic.h"

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
    void createHomePage();
    void showHome();
    void startNewGame(int colorCount);
    void onTabClicked();
    void updateRound();
    void updateStats();
    void finishRound();
    qint64 elapsedMs() const;

    GameLogic *gameLogic;
    QStackedWidget *pages;
    QWidget *home;
    QWidget *game;
    std::array<QPushButton *, 3> levelButtons;
    std::array<QLabel *, 3> levelRecords;
    QGridLayout *board;
    QLabel *target;
    QLabel *targetCaption;
    QLabel *swatch;
    QLabel *feedback;
    QLabel *stats;
    QProgressBar *progress;
    QPushButton *restart;
    QTimer *timer;
    QElapsedTimer clock;
    int difficulty = 2;
    int cleared = 0;
    int mistakes = 0;
    qint64 bestTime = 0;
    qint64 finishTime = 0;
};

#endif // MAINWINDOW_H
