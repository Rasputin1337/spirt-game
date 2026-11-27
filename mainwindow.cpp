#include "mainwindow.h"
#include "colortab.h"

#include <QAction>
#include <QColor>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QProgressBar>
#include <QPushButton>
#include <QSettings>
#include <QStackedWidget>
#include <QString>
#include <QTimer>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>
#include <QtMinMax>
#include <algorithm>
#include <random>

namespace {
QString timeText(qint64 milliseconds)
{
    const auto seconds = milliseconds / 1000;
    return QStringLiteral("%1:%2.%3").arg(seconds / 60)
        .arg(seconds % 60, 2, 10, QLatin1Char('0')).arg(milliseconds / 100 % 10);
}

QString recordKey(int difficulty)
{
    return QStringLiteral("records/%1").arg(difficulty);
}

QString recordText(qint64 milliseconds)
{
    return milliseconds > 0 ? timeText(milliseconds) : QStringLiteral("—");
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), gameLogic(new GameLogic(this)), timer(new QTimer(this))
{
    setWindowTitle(QStringLiteral("Spirit Game"));
    resize(640, 590);
    setStyleSheet(QStringLiteral(
        "QMainWindow, QStackedWidget#pages { background: #111827; color: #edf2ff; }"
        "QMenuBar, QMenu { background: #1b2740; color: #edf2ff; }"
        "QMenuBar::item:selected, QMenu::item:selected { background: #34496f; }"
        "QStackedWidget#pages QLabel { color: #edf2ff; }"
        "QLabel#homeTitle { font-size: 44px; font-weight: 800; }"
        "QLabel#title, QLabel#targetName { font-size: 24px; font-weight: 800; }"
        "QLabel#feedback, QLabel#targetCaption, QLabel#levelRecord { color: #c1cee6; }"
        "QWidget#targetPanel, QWidget#levelCard { background: #1b2740; border-radius: 14px; }"
        "QPushButton#restart, QPushButton#homeButton, QPushButton#levelButton {"
        " background: #263654; color: #ffffff;"
        " border: 2px solid transparent; border-radius: 9px; padding: 8px 14px; }"
        "QPushButton#levelButton { font-size: 17px; font-weight: 700; padding: 12px; }"
        "QPushButton#restart:hover, QPushButton#homeButton:hover,"
        " QPushButton#levelButton:hover { background: #34496f; }"
        "QPushButton#restart:focus, QPushButton#homeButton:focus,"
        " QPushButton#levelButton:focus { border-color: #b6cbff; }"
        "QProgressBar { background: #263654; border: 0; border-radius: 4px; }"
        "QProgressBar::chunk { background: #91a7ff; border-radius: 4px; }"));

    pages = new QStackedWidget(this);
    pages->setObjectName(QStringLiteral("pages"));
    setCentralWidget(pages);
    createHomePage();
    game = new QWidget(pages);
    pages->addWidget(game);
    auto *page = new QVBoxLayout(game);
    page->setContentsMargins(24, 20, 24, 22);
    page->setSpacing(12);

    auto *header = new QHBoxLayout;
    auto *title = new QLabel(QStringLiteral("Spirit Game"), game);
    title->setObjectName(QStringLiteral("title"));
    auto *homeButton = new QPushButton(QStringLiteral("В меню"), game);
    homeButton->setObjectName(QStringLiteral("homeButton"));
    connect(homeButton, &QPushButton::clicked, this, &MainWindow::showHome);
    restart = new QPushButton(QStringLiteral("Заново"), game);
    restart->setObjectName(QStringLiteral("restart"));
    connect(restart, &QPushButton::clicked, this, [this] { startNewGame(difficulty); });
    header->addWidget(title, 1);
    header->addWidget(homeButton);
    header->addWidget(restart);
    page->addLayout(header);

    auto *targetPanel = new QWidget(game);
    targetPanel->setObjectName(QStringLiteral("targetPanel"));
    auto *targetLayout = new QHBoxLayout(targetPanel);
    targetLayout->setContentsMargins(12, 10, 12, 10);
    targetLayout->setSpacing(14);
    swatch = new QLabel(targetPanel);
    swatch->setFixedSize(58, 52);
    swatch->setAlignment(Qt::AlignCenter);
    targetLayout->addWidget(swatch);
    auto *targetText = new QVBoxLayout;
    targetText->setSpacing(2);
    targetCaption = new QLabel(targetPanel);
    targetCaption->setObjectName(QStringLiteral("targetCaption"));
    target = new QLabel(targetPanel);
    target->setObjectName(QStringLiteral("targetName"));
    targetText->addWidget(targetCaption);
    targetText->addWidget(target);
    targetLayout->addLayout(targetText, 1);
    stats = new QLabel(game);
    stats->setWordWrap(true);
    progress = new QProgressBar(game);
    progress->setAccessibleName(QStringLiteral("Убрано плиток"));
    progress->setTextVisible(false);
    progress->setFixedHeight(8);
    feedback = new QLabel(game);
    feedback->setObjectName(QStringLiteral("feedback"));
    feedback->setWordWrap(true);
    page->addWidget(targetPanel);
    page->addWidget(stats);
    page->addWidget(progress);

    auto *field = new QWidget(game);
    board = new QGridLayout(field);
    board->setContentsMargins(0, 0, 0, 0);
    board->setSpacing(10);
    page->addWidget(field, 1);
    page->addWidget(feedback);

    connect(timer, &QTimer::timeout, this, &MainWindow::updateStats);
    createMenus();
    showHome();
}

void MainWindow::createHomePage()
{
    home = new QWidget(pages);
    pages->addWidget(home);
    auto *layout = new QVBoxLayout(home);
    layout->setContentsMargins(30, 26, 30, 26);
    layout->setSpacing(16);
    layout->addStretch();

    auto *title = new QLabel(QStringLiteral("Spirit Game"), home);
    title->setObjectName(QStringLiteral("homeTitle"));
    layout->addWidget(title);
    auto *instructions = new QLabel(QStringLiteral(
        "Выбери сложность и очисти поле. Убирай плитки по подсказке над полем. "
        "Рекорд засчитывается за раунд без ошибок."), home);
    instructions->setWordWrap(true);
    layout->addWidget(instructions);

    const std::array<QString, 3> names = {
        QStringLiteral("Лёгкая"), QStringLiteral("Средняя"), QStringLiteral("Сложная")
    };
    for (int index = 0; index < int(names.size()); ++index) {
        const int colors = index + 2;
        auto *card = new QWidget(home);
        card->setObjectName(QStringLiteral("levelCard"));
        auto *row = new QHBoxLayout(card);
        row->setContentsMargins(10, 10, 16, 10);
        row->setSpacing(16);
        auto *button = new QPushButton(names[index], card);
        button->setObjectName(QStringLiteral("levelButton"));
        button->setMinimumWidth(130);
        button->setAccessibleDescription(QStringLiteral("%1 цвета, %2 плиток")
                                         .arg(colors).arg(colors * 4));
        connect(button, &QPushButton::clicked, this, [this, colors] { startNewGame(colors); });
        levelButtons[index] = button;
        auto *details = new QVBoxLayout;
        details->setSpacing(3);
        details->addWidget(new QLabel(QStringLiteral("%1 цвета · %2 плиток")
                                      .arg(colors).arg(colors * 4), card));
        auto *record = new QLabel(card);
        record->setObjectName(QStringLiteral("levelRecord"));
        levelRecords[index] = record;
        details->addWidget(record);
        row->addWidget(button);
        row->addLayout(details, 1);
        layout->addWidget(card);
    }
    layout->addStretch();
}

void MainWindow::showHome()
{
    timer->stop();
    QSettings settings(QStringLiteral("SpiritGame"), QStringLiteral("SpiritGame"));
    for (int index = 0; index < int(levelRecords.size()); ++index) {
        const auto record = settings.value(recordKey(index + 2), 0).toLongLong();
        levelRecords[index]->setText(QStringLiteral("Рекорд %1").arg(recordText(record)));
    }
    pages->setCurrentWidget(home);
    levelButtons[difficulty - 2]->setFocus();
}

void MainWindow::createMenus()
{
    auto *menu = menuBar()->addMenu(QStringLiteral("Игра"));
    auto *back = menu->addAction(QStringLiteral("Главное меню"));
    back->setShortcut(QKeySequence(Qt::Key_Escape));
    connect(back, &QAction::triggered, this, &MainWindow::showHome);
    auto *replay = menu->addAction(QStringLiteral("Заново"));
    replay->setShortcut(QKeySequence(QStringLiteral("Ctrl+R")));
    connect(replay, &QAction::triggered, this, [this] { startNewGame(difficulty); });
    menu->addSeparator();
    for (int colors = 2; colors <= 4; ++colors) {
        auto *action = menu->addAction(QStringLiteral("Новая игра (%1 цвета)").arg(colors));
        action->setShortcut(QKeySequence(QStringLiteral("Ctrl+%1").arg(colors)));
        connect(action, &QAction::triggered, this, [this, colors] { startNewGame(colors); });
    }
}

void MainWindow::startNewGame(int colorCount)
{
    while (auto *item = board->takeAt(0)) {
        delete item->widget();
        delete item;
    }

    difficulty = colorCount;
    cleared = 0;
    mistakes = 0;
    finishTime = 0;
    bestTime = QSettings(QStringLiteral("SpiritGame"), QStringLiteral("SpiritGame"))
        .value(recordKey(difficulty), 0).toLongLong();
    gameLogic->startLevel(colorCount);
    auto colors = gameLogic->getAllColorsForLevel();
    std::mt19937 generator(std::random_device{}());
    std::shuffle(colors.begin(), colors.end(), generator);

    constexpr int columns = 4;
    for (int row = 0; row < columns; ++row) {
        board->setRowStretch(row, row < colorCount ? 1 : 0);
        board->setColumnStretch(row, 1);
    }
    for (int i = 0; i < colors.size(); ++i) {
        auto *tile = new ColorTab(colors[i], board->parentWidget());
        connect(tile, &QPushButton::clicked, this, &MainWindow::onTabClicked);
        board->addWidget(tile, i / columns, i % columns);
    }

    progress->setRange(0, colors.size());
    feedback->setText(QStringLiteral("Убирай плитки с белой рамкой."));
    pages->setCurrentWidget(game);
    clock.start();
    timer->start(100);
    updateRound();
}

void MainWindow::updateRound()
{
    const QColor color = gameLogic->getCurrentTargetColor();
    if (color.isValid()) {
        targetCaption->setText(QStringLiteral("Нажимай плитки этого цвета"));
        target->setText(ColorTab::nameFor(color));
        swatch->setText(ColorTab::symbolFor(color));
        swatch->setStyleSheet(QStringLiteral(
            "QLabel { background: %1; color: #101625; border: 3px solid white;"
            " border-radius: 12px; font-size: 27px; font-weight: 700; }")
            .arg(ColorTab::displayColor(color).name()));
    }
    for (int index = 0; index < board->count(); ++index) {
        auto *tile = static_cast<ColorTab *>(board->itemAt(index)->widget());
        tile->setTarget(tile->getColor() == color);
    }
    progress->setValue(cleared);
    updateStats();
}

qint64 MainWindow::elapsedMs() const
{
    return finishTime > 0 ? finishTime : clock.elapsed();
}

void MainWindow::updateStats()
{
    stats->setText(QStringLiteral("Плитки %1/%2   ·   Ошибки %3   ·   %4   ·   Рекорд %5")
                   .arg(cleared).arg(progress->maximum()).arg(mistakes)
                   .arg(timeText(elapsedMs()), recordText(bestTime)));
}

void MainWindow::finishRound()
{
    timer->stop();
    finishTime = qMax<qint64>(1, elapsedMs());
    const bool record = mistakes == 0 && (bestTime <= 0 || finishTime < bestTime);
    if (record) {
        bestTime = finishTime;
        QSettings(QStringLiteral("SpiritGame"), QStringLiteral("SpiritGame"))
            .setValue(recordKey(difficulty), bestTime);
    }
    updateStats();
    targetCaption->setText(QStringLiteral("Раунд завершён"));
    target->setText(QStringLiteral("Поле очищено!"));
    swatch->setText(QStringLiteral("✓"));
    swatch->setStyleSheet(QStringLiteral(
        "QLabel { background: #bbf7d0; color: #14532d; border-radius: 12px;"
        " font-size: 27px; font-weight: 700; }"));
    feedback->setText(record ? QStringLiteral("Новый рекорд: %1").arg(timeText(finishTime))
                             : QStringLiteral("Ещё раунд? Нажми «Заново» или Ctrl+R."));
    restart->setFocus();
}

void MainWindow::onTabClicked()
{
    auto *tile = qobject_cast<ColorTab *>(sender());
    if (!tile || pages->currentWidget() != game || gameLogic->isLevelComplete()) return;

    if (!gameLogic->checkClick(tile->getColor())) {
        ++mistakes;
        feedback->setText(QStringLiteral("Эта плитка другого цвета. Следуй подсказке над полем."));
        updateStats();
        return;
    }

    ++cleared;
    board->removeWidget(tile);
    tile->hide();
    tile->deleteLater();
    feedback->setText(QStringLiteral("Убирай плитки с белой рамкой."));
    updateRound();
    if (gameLogic->isLevelComplete()) finishRound();
}
