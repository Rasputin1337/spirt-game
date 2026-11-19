#ifndef COLORTAB_H
#define COLORTAB_H

#include <QPushButton>
#include <QColor>

class ColorTab : public QPushButton
{
    Q_OBJECT
public:
    ColorTab(QColor color, QWidget *parent = nullptr);

    QColor getColor() const;

private:
    QColor m_color;
};

#endif // COLORTAB_H