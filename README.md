# Моделирование игры "Крестики-нолики"

## Нужно:
1. Реализовать необходимый набор классов для хранения информации о положении на игровом поле (поле – произвольного или «достаточно» большого размера)
2. Реализовать процесс игры бота против человека
3. Реализовать отсечение неэффективных ходов при переборе возможных вариантов (альфа-бета отсечение)

## Условие:
1. Количество крестиков или ноликов, необходимых для победы, - параметр игры
2. Отразить логику поиска, оценивания и выбора следующего хода
3. Проиллюстрировать отбрасывание неэффективных ходов из рассмотрения

## Как реализовано?
1. Поле проивзольного размера - Класс `Board`, размер `N x N`
2. Количество символов, необходимых для победы - параметр `k`
3. Игра бота против человека - через консоль
4. Поиск оптимального хода - алгоритм ![Minimax](https://ru.wikipedia.org/wiki/%D0%9C%D0%B8%D0%BD%D0%B8%D0%BC%D0%B0%D0%BA%D1%81)
5. Отсечение неэффективных ходов - оптимизация ![Alpha-Beta Pruning](https://ru.wikipedia.org/wiki/%D0%90%D0%BB%D1%8C%D1%84%D0%B0-%D0%B1%D0%B5%D1%82%D0%B0-%D0%BE%D1%82%D1%81%D0%B5%D1%87%D0%B5%D0%BD%D0%B8%D0%B5)

## Структура программы
- Класс `Board` отвечает за:
    1. Хранение поля;
    2. Постановку и отмену ходов;
    3. Генерацию возможных ходов.
- Класс `Bot` отвечает за:
    1. Поиск лучшего хода;
    2. Оценивание позиций;
    3. Сортировку кандидатов;
    4. Альфа-бета отсечение.
- Структура `SearchStats` отвечает за:
    1. Число посещённых узлов;
    2. Отсечений;
    3. Пропущенных ходов.

```mermaid
classDiagram
    direction TB

    class Move {
        <<struct>>
        +int r
        +int c
    }

    class Board {
        -int n_
        -int k_
        -vector~char~ cells_
        -int occupied_
        +Board(int n, int k)
        +size() int
        +goal() int
        +inside(int r, int c) bool
        +at(int r, int c) char
        +empty(int r, int c) bool
        +full() bool
        +occupied() int
        +put(Move m, char who) void
        +undo(Move m) void
        +winsFrom(Move m, char who) bool
        +print() void
        +candidates() vector~Move~
    }

    class SearchStats {
        <<struct>>
        +long long nodes
        +long long cutoffs
        +long long skipped
    }

    class Bot {
        -int WIN$
        -Board& board_
        -char me_
        -char human_
        -int depth_
        -SearchStats stats_
        -bool showTree_
        -int shownLines_
        -int MAX_TREE_LINES$
        -int MAX_TREE_LEVEL$
        -opponent(char who)$ char
        -evaluate() long long
        -orderedMoves(char player) vector~Move~
        -minimax(int depth, long long alpha, long long beta, char player, Move last, char previous, int level) long long
        -trace(int level, string message) void
        +Bot(Board& board, char botChar, int depth, bool showTree)
        +stats() SearchStats
        +choose() Move
    }

    class Input {
        <<utility>>
        +readInt(string prompt, int low, int high)$ int
    }

    class Main {
        <<entry point>>
        +main() int
    }

    Bot --> Board : использует
    Bot *-- SearchStats : хранит
    Bot ..> Move : выбирает ход
    Board ..> Move : принимает ходы
    Main ..> Board : создаёт
    Main ..> Bot : создаёт
    Main ..> Input : вызывает
    Main ..> Move : использует
```
