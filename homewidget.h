#ifndef HOMEWIDGET_H
#define HOMEWIDGET_H

#include <QString>
#include <QWidget>
#include <array>

class QEvent;
class QHideEvent;
class QLabel;
class QPushButton;
class QSequentialAnimationGroup;
class QShowEvent;

class HomeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HomeWidget(QWidget *parent = nullptr);

    void setRecord(int colorCount, const QString &record);
    void setSelectedDifficulty(int colorCount);

signals:
    void difficultySelected(int colorCount);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void positionTitle();
    void finishIntro();

    QWidget *titleSlot;
    QLabel *title;
    QWidget *cards;
    std::array<QPushButton *, 3> levelButtons;
    std::array<QLabel *, 3> levelRecords;
    QSequentialAnimationGroup *intro = nullptr;
    qreal titleProgress = 0;
    int selectedDifficulty = 2;
    bool introFinished = false;
};

#endif // HOMEWIDGET_H
