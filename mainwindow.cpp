#include "mainwindow.h"
#include "colortab.h"
#include <QMessageBox>
#include <QPushButton>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QDialog>
#include <QLabel>
#include <QPixmap>
#include <QRandomGenerator>
#include <QScreen>
#include <QVBoxLayout>
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

    if (QRandomGenerator::global()->bounded(100) < 5) {
        QDialog photoDialog(this);
        photoDialog.setWindowTitle(QStringLiteral("photo_2026-11-19_17-05-35.jpg"));
        QPixmap photo(QStringLiteral(":/photo_2026-11-19_17-05-35.jpg"));
        const QSize availableSize = screen()->availableGeometry().size() * 0.8;
        if (photo.width() > availableSize.width() || photo.height() > availableSize.height()) {
            photo = photo.scaled(availableSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        auto *photoLabel = new QLabel(&photoDialog);
        photoLabel->setPixmap(photo);
        auto *photoLayout = new QVBoxLayout(&photoDialog);
        photoLayout->addWidget(photoLabel);
        photoDialog.exec();
    }

    bool isCorrect = gameLogic->checkClick(clickedTab->getColor());

    if (isCorrect) {
        clickedTab->deleteLater();

        if (gameLogic->isLevelComplete()) {
            QMessageBox::information(this, "Победа!", "Уровень пройден!");
        }
    } else {
        QColor target = gameLogic->getCurrentTargetColor();
        QMessageBox::warning(this, "Ошибка",
                             QString("Неверный порядок! Сейчас нужно закрывать цвет: %1").arg(target.name()));
    }
}
