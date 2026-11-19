#include "colortab.h"

ColorTab::ColorTab(QColor color, QWidget *parent) : QPushButton(parent), m_color(color)
{
    setFixedSize(80, 80);
    QString style = QString("background-color: %1; border: 2px solid black; border-radius: 5px;")
                        .arg(color.name());
    setStyleSheet(style);
}

QColor ColorTab::getColor() const
{
    return m_color;
}