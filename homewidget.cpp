#include "homewidget.h"
#include "gamestate.h"

#include <QEasingCurve>
#include <QEvent>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QSequentialAnimationGroup>
#include <QVariant>
#include <QVariantAnimation>
#include <QVBoxLayout>
#include <QtNumeric>

HomeWidget::HomeWidget(QWidget *parent)
    : QWidget(parent), titleSlot(new QWidget(this)),
      title(new QLabel(QStringLiteral("Spirit Game"), this)), cards(new QWidget(this))
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 26, 30, 26);
    layout->setSpacing(16);
    layout->addStretch();

    title->setObjectName(QStringLiteral("homeTitle"));
    title->setAttribute(Qt::WA_TransparentForMouseEvents);
    title->ensurePolished();
    titleSlot->setMinimumSize(title->sizeHint());
    titleSlot->installEventFilter(this);
    layout->addWidget(titleSlot);

    auto *cardLayout = new QVBoxLayout(cards);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(16);
    layout->addWidget(cards);
    layout->addStretch();

    const std::array<QString, 3> names = {
        QStringLiteral("Лёгкая"), QStringLiteral("Средняя"), QStringLiteral("Сложная")
    };
    for (int index = 0; index < int(names.size()); ++index) {
        const int colors = index + 2;
        auto *card = new QWidget(cards);
        card->setObjectName(QStringLiteral("levelCard"));
        auto *row = new QHBoxLayout(card);
        row->setContentsMargins(10, 10, 16, 10);
        row->setSpacing(16);
        auto *button = new QPushButton(names[index], card);
        button->setObjectName(QStringLiteral("levelButton"));
        button->setMinimumWidth(130);
        button->setAccessibleDescription(QStringLiteral("%1 цвета, %2 плиток")
                                         .arg(colors).arg(colors * 4));
        connect(button, &QPushButton::clicked, this, [this, colors] {
            emit difficultySelected(colors);
        });
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
        cardLayout->addWidget(card);
    }

    auto *opacity = new QGraphicsOpacityEffect(cards);
    opacity->setOpacity(0);
    cards->setGraphicsEffect(opacity);
    cards->setEnabled(false);
    title->raise();
    positionTitle();
}

void HomeWidget::setRecord(int colorCount, const QString &record)
{
    if (!GameState::isValidDifficulty(colorCount))
        return;
    levelRecords[colorCount - 2]->setText(QStringLiteral("Рекорд %1").arg(record));
}

void HomeWidget::setSelectedDifficulty(int colorCount)
{
    if (!GameState::isValidDifficulty(colorCount))
        return;
    selectedDifficulty = colorCount;
    if (introFinished)
        levelButtons[colorCount - 2]->setFocus();
}

bool HomeWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == titleSlot && !introFinished
        && (event->type() == QEvent::Move || event->type() == QEvent::Resize)) {
        positionTitle();
    }
    return QWidget::eventFilter(watched, event);
}

void HomeWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (introFinished || intro)
        return;

    intro = new QSequentialAnimationGroup(this);
    auto *move = new QVariantAnimation(intro);
    move->setStartValue(0.0);
    move->setEndValue(1.0);
    move->setDuration(2500);
    move->setEasingCurve(QEasingCurve::InOutCubic);
    connect(move, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        titleProgress = value.toReal();
        positionTitle();
    });
    intro->addAnimation(move);

    auto *fade = new QPropertyAnimation(cards->graphicsEffect(), "opacity", intro);
    fade->setStartValue(0.0);
    fade->setEndValue(1.0);
    fade->setDuration(1000);
    fade->setEasingCurve(QEasingCurve::InOutCubic);
    intro->addAnimation(fade);
    connect(intro, &QSequentialAnimationGroup::finished, this, &HomeWidget::finishIntro);
    intro->start();
}

void HomeWidget::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    if (intro && isHidden())
        finishIntro();
}

void HomeWidget::positionTitle()
{
    const QRect destination = titleSlot->geometry();
    const int startY = -destination.height();
    const int y = startY + qRound(titleProgress * (destination.y() - startY));
    title->setGeometry(destination.x(), y, destination.width(), destination.height());
}

void HomeWidget::finishIntro()
{
    introFinished = true;
    intro->stop();
    intro->deleteLater();
    intro = nullptr;

    auto *layout = new QVBoxLayout(titleSlot);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(title);
    title->show();
    cards->setGraphicsEffect(nullptr);
    cards->setEnabled(true);
    if (isVisible())
        levelButtons[selectedDifficulty - 2]->setFocus();
}
