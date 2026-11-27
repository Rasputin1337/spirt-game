#include "mainwindow.h"
#include "colortab.h"
#include "homewidget.h"

#include <QAction>
#include <QColor>
#include <QFontMetrics>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>
#include <QSettings>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QString>
#include <QTimer>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

class ErrorCountLabel final : public QLabel
{
public:
    explicit ErrorCountLabel(QWidget *parent) : QLabel(parent)
    {
        setObjectName(QStringLiteral("errorCount"));
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        pulse.setSingleShot(true);
        pulse.setTimerType(Qt::PreciseTimer);
        connect(&pulse, &QTimer::timeout, this, [this] { update(); });
    }

    void highlight()
    {
        pulse.start(1000);
        update();
    }

    void clearHighlight()
    {
        pulse.stop();
        update();
    }

    QSize sizeHint() const override
    {
        const QFontMetrics metrics(highlightFont());
        return QSize(metrics.horizontalAdvance(text()), metrics.height());
    }

    QSize minimumSizeHint() const override { return sizeHint(); }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        const bool highlighted = pulse.isActive();
        painter.setFont(highlighted ? highlightFont() : font());
        painter.setPen(highlighted ? QColor("#FF7070") : palette().color(QPalette::WindowText));
        painter.drawText(rect(), Qt::AlignLeft | Qt::AlignVCenter, text());
    }

private:
    QFont highlightFont() const
    {
        QFont larger = font();
        if (larger.pixelSize() > 0)
            larger.setPixelSize(larger.pixelSize() + 2);
        else
            larger.setPointSizeF(larger.pointSizeF() + 1.5);
        return larger;
    }

    QTimer pulse;
};

namespace {
QString timeText(qint64 milliseconds)
{
    const auto seconds = milliseconds / 1000;
    return QStringLiteral("%1:%2.%3").arg(seconds / 60)
        .arg(seconds % 60, 2, 10, QLatin1Char('0')).arg(milliseconds / 100 % 10);
}

QSettings gameSettings()
{
    return QSettings(QSettings::defaultFormat(), QSettings::UserScope,
                     QStringLiteral("SpiritGame"), QStringLiteral("SpiritGame"));
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
        "QMainWindow, QStackedWidget#pages { background: #121212; color: #F2F2F2; }"
        "QMenuBar, QMenu { background: #202020; color: #F2F2F2; }"
        "QMenuBar::item:selected, QMenu::item:selected { background: #3A3A3A; }"
        "QStackedWidget#pages QLabel { color: #F2F2F2; }"
        "QLabel#homeTitle { font-size: 44px; font-weight: 800; }"
        "QLabel#title, QLabel#targetName { font-size: 24px; font-weight: 800; }"
        "QLabel#targetCaption, QLabel#levelRecord { color: #C7C7C7; }"
        "QWidget#targetPanel, QWidget#levelCard { background: #202020; border-radius: 14px; }"
        "QPushButton#restart, QPushButton#homeButton, QPushButton#levelButton {"
        " background: #2B2B2B; color: #ffffff;"
        " border: 2px solid transparent; border-radius: 9px; padding: 8px 14px; }"
        "QPushButton#levelButton { font-size: 17px; font-weight: 700; padding: 12px; }"
        "QPushButton#restart:hover, QPushButton#homeButton:hover,"
        " QPushButton#levelButton:hover { background: #3A3A3A; }"
        "QPushButton#restart:focus, QPushButton#homeButton:focus,"
        " QPushButton#levelButton:focus { border-color: #BDBDBD; }"
        "QProgressBar { background: #2B2B2B; border: 0; border-radius: 4px; }"
        "QProgressBar::chunk { background: #BDBDBD; border-radius: 4px; }"));

    pages = new QStackedWidget(this);
    pages->setObjectName(QStringLiteral("pages"));
    setCentralWidget(pages);
    home = new HomeWidget(pages);
    pages->addWidget(home);
    connect(home, &HomeWidget::difficultySelected, this, &MainWindow::startNewGame);
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
    connect(restart, &QPushButton::clicked, this, [this] { startNewGame(gameLogic->state().difficulty()); });
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
    auto *stats = new QHBoxLayout;
    stats->setSpacing(0);
    tileCount = new QLabel(game);
    errorCount = new ErrorCountLabel(game);
    timeAndRecord = new QLabel(game);
    timeAndRecord->setWordWrap(true);
    stats->addWidget(tileCount);
    stats->addWidget(errorCount);
    stats->addWidget(timeAndRecord, 1);
    progress = new QProgressBar(game);
    progress->setAccessibleName(QStringLiteral("Убрано плиток"));
    progress->setTextVisible(false);
    progress->setFixedHeight(8);
    page->addWidget(targetPanel);
    page->addLayout(stats);
    page->addWidget(progress);

    auto *field = new QWidget(game);
    board = new QGridLayout(field);
    board->setContentsMargins(0, 0, 0, 0);
    board->setSpacing(10);
    page->addWidget(field, 1);

    connect(timer, &QTimer::timeout, this, &MainWindow::updateStats);
    createMenus();
    showHome();
}

void MainWindow::showHome()
{
    timer->stop();
    gameLogic->abandonRound();
    errorCount->clearHighlight();
    const auto settings = gameSettings();
    for (int colors = 2; colors <= 4; ++colors) {
        const auto record = settings.value(recordKey(colors), 0).toLongLong();
        home->setRecord(colors, recordText(record));
    }
    pages->setCurrentWidget(home);
    home->setSelectedDifficulty(gameLogic->state().difficulty());
}

void MainWindow::createMenus()
{
    auto *menu = menuBar()->addMenu(QStringLiteral("Игра"));
    auto *back = menu->addAction(QStringLiteral("Главное меню"));
    back->setShortcut(QKeySequence(Qt::Key_Escape));
    connect(back, &QAction::triggered, this, &MainWindow::showHome);
    auto *replay = menu->addAction(QStringLiteral("Заново"));
    replay->setShortcut(QKeySequence(QStringLiteral("Ctrl+R")));
    connect(replay, &QAction::triggered, this, [this] { startNewGame(gameLogic->state().difficulty()); });
    menu->addSeparator();
    for (int colors = 2; colors <= 4; ++colors) {
        auto *action = menu->addAction(QStringLiteral("Новая игра (%1 цвета)").arg(colors));
        action->setShortcut(QKeySequence(QStringLiteral("Ctrl+%1").arg(colors)));
        connect(action, &QAction::triggered, this, [this, colors] { startNewGame(colors); });
    }
}

void MainWindow::startNewGame(int colorCount)
{
    if (!GameState::isValidDifficulty(colorCount))
        return;

    const auto bestTime = gameSettings()
        .value(recordKey(colorCount), 0).toLongLong();
    const auto colors = gameLogic->startLevel(colorCount, bestTime);

    while (auto *item = board->takeAt(0)) {
        delete item->widget();
        delete item;
    }

    errorCount->clearHighlight();

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

    progress->setRange(0, gameLogic->state().tileCount());
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
            "QLabel { background: %1; color: #121212; border: 3px solid white;"
            " border-radius: 12px; font-size: 27px; font-weight: 700; }")
            .arg(ColorTab::displayColor(color).name()));
    }
    for (int index = 0; index < board->count(); ++index) {
        auto *tile = static_cast<ColorTab *>(board->itemAt(index)->widget());
        tile->setTarget(tile->getColor() == color);
    }
    progress->setValue(gameLogic->state().cleared());
    updateStats();
}

qint64 MainWindow::elapsedMs() const
{
    const auto &state = gameLogic->state();
    return state.status() == GameState::Status::Completed ? state.finishTime() : clock.elapsed();
}

void MainWindow::updateStats()
{
    const auto &state = gameLogic->state();
    tileCount->setText(QStringLiteral("Плитки %1/%2   ·   ").arg(state.cleared()).arg(state.tileCount()));
    const QString errors = QStringLiteral("Ошибки %1").arg(state.mistakes());
    if (errorCount->text() != errors)
        errorCount->setText(errors);
    timeAndRecord->setText(QStringLiteral("   ·   %1   ·   Рекорд %2")
                           .arg(timeText(elapsedMs()), recordText(state.bestTime())));
}

void MainWindow::finishRound(bool newRecord)
{
    timer->stop();
    if (newRecord) {
        const auto &state = gameLogic->state();
        gameSettings()
            .setValue(recordKey(state.difficulty()), state.bestTime());
    }
    updateStats();
    targetCaption->setText(QStringLiteral("Раунд завершён"));
    target->setText(QStringLiteral("Поле очищено!"));
    swatch->setText(QStringLiteral("✓"));
    swatch->setStyleSheet(QStringLiteral(
        "QLabel { background: #bbf7d0; color: #14532d; border-radius: 12px;"
        " font-size: 27px; font-weight: 700; }"));
    restart->setFocus();
}

void MainWindow::onTabClicked()
{
    auto *tile = qobject_cast<ColorTab *>(sender());
    if (!tile || pages->currentWidget() != game) return;

    const auto result = gameLogic->checkClick(tile->getColor(), clock.elapsed());
    if (result == GameLogic::ClickResult::Ignored)
        return;
    if (result == GameLogic::ClickResult::Mistake) {
        updateStats();
        errorCount->highlight();
        return;
    }

    board->removeWidget(tile);
    tile->hide();
    tile->deleteLater();
    updateRound();
    if (result == GameLogic::ClickResult::Completed || result == GameLogic::ClickResult::NewRecord)
        finishRound(result == GameLogic::ClickResult::NewRecord);
}
