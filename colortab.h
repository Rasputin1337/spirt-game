#ifndef COLORTAB_H
#define COLORTAB_H

#include <QPushButton>
#include <QColor>
#include <QString>

class QPaintEvent;

class ColorTab : public QPushButton
{
    Q_OBJECT
public:
    ColorTab(QColor color, QWidget *parent = nullptr);

    QColor getColor() const;
    void setTarget(bool target);
    static QColor displayColor(QColor color);
    static QString symbolFor(QColor color);
    static QString nameFor(QColor color);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void updateAppearance();
    QColor m_color;
    bool m_target = false;
};

#endif // COLORTAB_H
