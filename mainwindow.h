#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include "gamelogic.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onTabClicked();
    void startGameWith2Colors() { startNewGame(2); }
    void startGameWith3Colors() { startNewGame(3); }
    void startGameWith4Colors() { startNewGame(4); }

private:
    void createMenus();
    void startNewGame(int colorCount);
    void shuffleRemainingTabs(QWidget *excludeWidget);

    GameLogic *gameLogic;
    QWidget *centralWidget;
    QGridLayout *layout;
    int m_columns = 4; // Количество колонок в сетке
};

#endif // MAINWINDOW_H