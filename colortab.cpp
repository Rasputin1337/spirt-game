#include "colortab.h"
#include <QCursor>
#include <QSizePolicy>
#include <QStyleOptionButton>
#include <QStylePainter>

ColorTab::ColorTab(QColor color, QWidget *parent) : QPushButton(parent), m_color(color)
{
    setMinimumSize(66, 54);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setCursor(Qt::PointingHandCursor);
    setText(symbolFor(color));
    setAccessibleName(nameFor(color));
    setToolTip(nameFor(color));
    updateAppearance();
}

void ColorTab::setTarget(bool target)
{
    if (m_target == target) return;
    m_target = target;
    setAccessibleDescription(target ? QStringLiteral("Текущая цель") : QString());
}

void ColorTab::paintEvent(QPaintEvent *)
{
    QStyleOptionButton option;
    initStyleOption(&option);
    option.state.setFlag(QStyle::State_HasFocus, false);

    QStylePainter painter(this);
    painter.drawControl(QStyle::CE_PushButton, option);
}

QColor ColorTab::displayColor(QColor color)
{
    return color.lighter(170);
}

void ColorTab::updateAppearance()
{
    const QColor bright = displayColor(m_color);
    const QColor fill = QColor::fromRgb((bright.red() + 27) / 2, (bright.green() + 39) / 2,
                                        (bright.blue() + 64) / 2);
    setStyleSheet(QStringLiteral(
        "QPushButton { background: %1; color: #ffffff; border: none;"
        " border-radius: 14px; font-size: 27px; font-weight: 700; }"
        "QPushButton:hover { background: %2; }"
        "QPushButton:pressed { background: %3; }")
        .arg(fill.name(), fill.lighter(108).name(), fill.darker(108).name()));
}

QColor ColorTab::getColor() const
{
    return m_color;
}

QString ColorTab::symbolFor(QColor color)
{
    if (color == Qt::red) return QStringLiteral("●");
    if (color == Qt::blue) return QStringLiteral("◆");
    if (color == Qt::green) return QStringLiteral("▲");
    if (color == Qt::yellow) return QStringLiteral("■");
    if (color == Qt::magenta) return QStringLiteral("✦");
    if (color == Qt::cyan) return QStringLiteral("⬟");
    return {};
}

QString ColorTab::nameFor(QColor color)
{
    if (color == Qt::red) return QStringLiteral("Красный");
    if (color == Qt::blue) return QStringLiteral("Синий");
    if (color == Qt::green) return QStringLiteral("Зелёный");
    if (color == Qt::yellow) return QStringLiteral("Жёлтый");
    if (color == Qt::magenta) return QStringLiteral("Пурпурный");
    if (color == Qt::cyan) return QStringLiteral("Голубой");
    return {};
}
