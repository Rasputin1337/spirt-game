#include "mainwindow.h"
#include "colortab.h"
#include <QMessageBox>
#include <QPushButton>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <random>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    centralWidget = new QWidget(this);
    layout = new QGridLayout(centralWidget);
    setCentralWidget(centralWidget);
    setWindowTitle("Spirit Game");
    resize(600, 400);

    gameLogic = new GameLogic(this);

    createMenus();
    startNewGame(2);
}

MainWindow::~MainWindow() {}

void MainWindow::createMenus()
{
    QMenu *gameMenu = menuBar()->addMenu("Игра");

    QAction *action2Colors = new QAction("Новая игра (2 цвета)", this);
    connect(action2Colors, &QAction::triggered, this, &MainWindow::startGameWith2Colors);
    gameMenu->addAction(action2Colors);

    QAction *action3Colors = new QAction("Новая игра (3 цвета)", this);
    connect(action3Colors, &QAction::triggered, this, &MainWindow::startGameWith3Colors);
    gameMenu->addAction(action3Colors);

    QAction *action4Colors = new QAction("Новая игра (4 цвета)", this);
    connect(action4Colors, &QAction::triggered, this, &MainWindow::startGameWith4Colors);
    gameMenu->addAction(action4Colors);
}

void MainWindow::startNewGame(int colorCount)
{
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    gameLogic->startLevel(colorCount);
    QVector<QColor> allColors = gameLogic->getAllColorsForLevel();

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(allColors.begin(), allColors.end(), g);

    for (int i = 0; i < allColors.size(); ++i) {
        ColorTab *tab = new ColorTab(allColors[i], this);
        connect(tab, &QPushButton::clicked, this, &MainWindow::onTabClicked);

        int row = i / m_columns;
        int col = i % m_columns;
        layout->addWidget(tab, row, col);
    }
}

void MainWindow::onTabClicked()
{
    ColorTab *clickedTab = qobject_cast<ColorTab*>(sender());
    if (!clickedTab) return;

    bool isCorrect = gameLogic->checkClick(clickedTab->getColor());

    if (isCorrect) {
        QColor clickedColor = clickedTab->getColor();
        clickedTab->deleteLater();

        if (clickedColor == Qt::green) {
            shuffleRemainingTabs(clickedTab);
        }

        if (gameLogic->isLevelComplete()) {
            QMessageBox::information(this, "Победа!", "Уровень пройден!");
        }
    } else {
        QColor target = gameLogic->getCurrentTargetColor();
        QMessageBox::warning(this, "Ошибка",
                             QString("Неверный порядок! Сейчас нужно закрывать цвет: %1").arg(target.name()));
    }
}


void MainWindow::shuffleRemainingTabs(QWidget *excludeWidget)
{
    QVector<ColorTab*> remainingTabs;

    for (int i = 0; i < layout->count(); ++i) {
        ColorTab *tab = qobject_cast<ColorTab*>(layout->itemAt(i)->widget());
        if (tab && tab != excludeWidget) {
            remainingTabs.append(tab);
        }
    }
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(remainingTabs.begin(), remainingTabs.end(), g);

    for (int i = 0; i < remainingTabs.size(); ++i) {
        layout->addWidget(remainingTabs[i], i / m_columns, i % m_columns);
    }
}