# spirt-game
Spirt Game - игра, в которой нужно решить головоломку, нажав на цветные кнопки в правильной последовательности.
Участники проекта:
Стариков А.
Пйодь А.
и
Романюк Д.
Ткаченко М.

Отчёт по 1 неделе:
1. Тема выбрана
2. Определены 3 функциональных действия: выбор сложности, нажатие на кнопку и получение обратной связи
3. Задачи распределены понедельно, первая неделя - Романюк Д., вторая - Ткаченко М., третья - Стариков А., последняя - Пьйодь А.
4. Создан и настроен git
5. Создана схема классов (в первом приближении)
## Схема классов проекта

```mermaid
classDiagram
    class MainWindow {
        -GameLogic gameLogic
        -QGridLayout layout
        +startNewGame(int colorCount)
        +onTabClicked()
        +createMenus()
    }
    
    class ColorTab {
        -QColor m_color
        +ColorTab(QColor color)
        +getColor() QColor
    }
    
    class GameLogic {
        -QVector~QColor~ m_sequence
        -int m_currentIndex
        +startLevel(int colorCount)
        +checkClick(QColor color) bool
        +isLevelComplete() bool
    }
    
    class ScoreManager {
        +saveToFile(QString path)
        +loadFromFile(QString path)
    }

    MainWindow *-- ColorTab : создает
    MainWindow --> GameLogic : использует
    MainWindow --> ScoreManager : использует
```
6. Окно запускается и работает



Отчёт по 2 неделе:

1. Задачи распределены понедельно, первая неделя - Романюк Д., вторая - Ткаченко М., третья - Стариков А., последняя - Пьйодь А.
2. Реализовано главное меню, плавная заставка, таймер прохождения
3. Теперь порядок нужных цветов разный
4. Полностью переработан графический дизайн spirt-game
5. Добавлены визуальные подсказки, добавлены фигуры на каждый из цветов для пользователей с нарушением цветового зрения
6. Созданы собственные виджеты HomeWidget и ErrorCountLabel
7. Применен стандартный контейнер std::array
8. Переработан ColorTab
9. Разработана отдельная модель данных GameState, выполнено разделение интерфейса, игровых данных и игровой логики

## Схема классов проекта

```mermaid
classDiagram
    class MainWindow {
        -GameLogic *gameLogic
        -HomeWidget *home
        -ErrorCountLabel *errorCount
        -QGridLayout *board
        +MainWindow(QWidget *parent = nullptr)
        -showHome() void
        -startNewGame(int colorCount) void
        -onTabClicked() void
        -updateRound() void
        -updateStats() void
        -finishRound(bool newRecord) void
    }

    class HomeWidget {
        -std::array~QPushButton *, 3~ levelButtons
        -std::array~QLabel *, 3~ levelRecords
        -int selectedDifficulty
        +HomeWidget(QWidget *parent = nullptr)
        +setRecord(int colorCount, const QString &record) void
        +setSelectedDifficulty(int colorCount) void
        +difficultySelected(int colorCount) void «signal»
    }

    class ColorTab {
        -QColor m_color
        -bool m_target
        +ColorTab(QColor color, QWidget *parent = nullptr)
        +getColor() QColor const
        +setTarget(bool target) void
        +displayColor(QColor color) QColor$
        +symbolFor(QColor color) QString$
        +nameFor(QColor color) QString$
        #paintEvent(QPaintEvent *event) void override
    }

    class ErrorCountLabel {
        -QTimer pulse
        +ErrorCountLabel(QWidget *parent)
        +highlight() void
        +clearHighlight() void
        #paintEvent(QPaintEvent *) void override
    }

    class GameLogic {
        -GameState m_state
        +GameLogic(QObject *parent = nullptr)
        +startLevel(int colorCount, qint64 bestTime) QVector~QColor~
        +abandonRound() void
        +checkClick(QColor color, qint64 elapsedMs) ClickResult
        +getCurrentTargetColor() QColor const
        +state() const GameState & const
    }

    class GameState {
        -int m_difficulty
        -QVector~QColor~ m_sequence
        -int m_position
        -int m_mistakes
        -Status m_status
        -qint64 m_finishTime
        -qint64 m_bestTime
        +isValidDifficulty(int colorCount) bool$
        +difficulty() int const
        +cleared() int const
        +tileCount() int const
        +mistakes() int const
        +status() Status const
        +finishTime() qint64 const
        +bestTime() qint64 const
    }

    class ScoreManager {
        +ScoreManager(QObject *parent = nullptr)
    }

    QMainWindow <|-- MainWindow
    QWidget <|-- HomeWidget
    QPushButton <|-- ColorTab
    QLabel <|-- ErrorCountLabel
    QObject <|-- GameLogic
    QObject <|-- ScoreManager

    MainWindow *-- HomeWidget : владеет через pages
    MainWindow *-- ErrorCountLabel : владеет через game
    MainWindow *-- ColorTab : создает
    MainWindow *-- GameLogic : владеет
    GameLogic *-- GameState : хранит по значению
    MainWindow ..> GameState : использует
    HomeWidget ..> GameState : проверяет сложность
```