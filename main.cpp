
#include <algorithm>  // max, min, stable_sort
#include <array>
#include <climits>     // LLONG_MIN, LLONG_MAX
#include <cmath>
#include <iostream>    // cin, cout
#include <limits>      // numeric_limits
#include <stdexcept>   // исключения
#include <string>
#include <utility>     // pair
#include <vector>      // vector

using namespace std;

// Структура для хранения координат хода.
// r - номер строки, c - номер столбца.
// -1 означает, что ход не задан.
struct Move {
    int r = -1;
    int c = -1;
};

// Класс Board отвечает за игровое поле.
// Здесь хранятся клетки, размер поля
// и количество символов для победы.
class Board {
    int n_;                 // Размер поля N x N
    int k_;                 // Сколько символов подряд нужно для победы
    vector<char> cells_;   // Клетки поля
    int occupied_ = 0;     // Количество занятых клеток

public:
    // Конструктор создаёт пустое игровое поле.
    // Все клетки сначала заполнены символом '.'.
    Board(int n, int k) : n_(n), k_(k), cells_(n * n, '.') {
        if (n < 3 || k < 3 || k > n)
            throw invalid_argument("Требуется 3 <= K <= N");
    }

    // Получаем размер поля и условие победы.
    int size() const { return n_; }
    int goal() const { return k_; }

    // Проверяем, находятся ли координаты внутри поля.
    bool inside(int r, int c) const {
        return r >= 0 && r < n_ && c >= 0 && c < n_;
    }

    // Возвращаем символ из указанной клетки.
    // Двумерное поле хранится в одномерном векторе.
    char at(int r, int c) const {
        return cells_[r * n_ + c];
    }

    // Проверяем, свободна ли клетка.
    bool empty(int r, int c) const {
        return inside(r, c) && at(r, c) == '.';
    }

    // Проверяем, заполнено ли всё поле.
    bool full() const {
        return occupied_ == n_ * n_;
    }

    int occupied() const {
        return occupied_;
    }

    // Ставим X или O в указанную клетку.
    void put(Move m, char who) {
        if (!empty(m.r, m.c))
            throw invalid_argument("Недопустимый ход");

        cells_[m.r * n_ + m.c] = who;
        ++occupied_;
    }

    // Отменяем ход.
    // Это необходимо для Minimax:
    // бот пробует ход, оценивает результат,
    // а затем возвращает поле в прежнее состояние.
    void undo(Move m) {
        if (!inside(m.r, m.c) || empty(m.r, m.c))
            throw invalid_argument("Нельзя отменить ход");

        cells_[m.r * n_ + m.c] = '.';
        --occupied_;
    }

    // Проверяем, победил ли игрок после последнего хода.
    bool winsFrom(Move m, char who) const {

        // Проверяем четыре направления:
        // вертикаль, горизонталь и две диагонали.
        constexpr int dr[4] = {1, 0, 1, 1};
        constexpr int dc[4] = {0, 1, 1, -1};

        for (int d = 0; d < 4; ++d) {
            int count = 1;

            // Проверяем клетки в обе стороны
            // от последнего поставленного символа.
            for (int sign : {-1, 1}) {
                int r = m.r + sign * dr[d];
                int c = m.c + sign * dc[d];

                while (inside(r, c) && at(r, c) == who) {
                    ++count;

                    r += sign * dr[d];
                    c += sign * dc[d];
                }
            }

            // Если нашли K символов подряд,
            // значит игрок победил.
            if (count >= k_)
                return true;
        }

        return false;
    }

    // Выводим игровое поле в консоль.
    void print() const {
        cout << "\n    ";

        for (int c = 0; c < n_; ++c)
            cout << c + 1 << (c + 1 < 10 ? "  " : " ");

        cout << "\n";

        for (int r = 0; r < n_; ++r) {
            cout << (r + 1 < 10 ? " " : "") << r + 1 << "  ";

            for (int c = 0; c < n_; ++c)
                cout << at(r, c) << "  ";

            cout << '\n';
        }
    }

    // Формируем список возможных ходов.
    // Чтобы ускорить поиск, рассматриваем только
    // пустые клетки рядом с уже занятыми.
    vector<Move> candidates() const {
        vector<Move> result;

        // Если поле пустое, начинаем с центра.
        if (occupied_ == 0)
            return {{n_ / 2, n_ / 2}};

        for (int r = 0; r < n_; ++r) {
            for (int c = 0; c < n_; ++c) {

                // Занятые клетки пропускаем.
                if (!empty(r, c))
                    continue;

                bool near = false;

                // Проверяем восемь соседних клеток.
                for (int dr = -1; dr <= 1 && !near; ++dr) {
                    for (int dc = -1; dc <= 1; ++dc) {
                        if ((dr || dc) &&
                            inside(r + dr, c + dc) &&
                            at(r + dr, c + dc) != '.') {
                            near = true;
                        }
                    }
                }

                // Добавляем клетку, если рядом
                // находится хотя бы один символ.
                if (near)
                    result.push_back({r, c});
            }
        }

        return result;
    }
};

// Структура для статистики поиска.
// Нужна для демонстрации альфа-бета отсечения.
struct SearchStats {
    long long nodes = 0;    // Сколько позиций исследовано
    long long cutoffs = 0;  // Сколько раз произошло отсечение
    long long skipped = 0;  // Сколько ходов пропущено
};

// Класс Bot реализует искусственный интеллект.
// Используется алгоритм Minimax
// с альфа-бета отсечением.
class Bot {
    // Очень большая оценка для выигрышной позиции.
    static constexpr int WIN = 100000000;

    Board& board_;  // Ссылка на текущее игровое поле
    char me_;       // Символ бота
    char human_;    // Символ человека
    int depth_;     // Глубина поиска
    SearchStats stats_;

    // Возвращаем символ противника.
    static char opponent(char who) {
        return who == 'X' ? 'O' : 'X';
    }

    // Эвристическая функция оценки позиции.
    // Положительная оценка выгодна боту,
    // отрицательная - человеку.
    long long evaluate() const {

        // Направления проверки линий.
        constexpr int dr[4] = {1, 0, 1, 1};
        constexpr int dc[4] = {0, 1, 1, -1};

        long long total = 0;

        int n = board_.size();
        int k = board_.goal();

        // Перебираем все возможные отрезки длины K.
        for (int r = 0; r < n; ++r) {
            for (int c = 0; c < n; ++c) {
                for (int d = 0; d < 4; ++d) {

                    // Находим последнюю клетку отрезка.
                    int endR = r + (k - 1) * dr[d];
                    int endC = c + (k - 1) * dc[d];

                    // Если отрезок выходит за поле,
                    // не рассматриваем его.
                    if (!board_.inside(endR, endC))
                        continue;

                    int mine = 0;   // Символы бота
                    int theirs = 0; // Символы человека

                    // Считаем символы в отрезке.
                    for (int t = 0; t < k; ++t) {
                        char x = board_.at(
                            r + t * dr[d],
                            c + t * dc[d]
                        );

                        mine += (x == me_);
                        theirs += (x == human_);
                    }

                    // Если в отрезке есть и X, и O,
                    // такую линию уже нельзя завершить.
                    if (mine && theirs)
                        continue;

                    // Полностью пустой отрезок пропускаем.
                    if (!mine && !theirs)
                        continue;

                    int count = mine ? mine : theirs;

                    // Чем больше одинаковых символов,
                    // тем выше ценность линии.
                    // Например: 1 символ -> 8 очков,
                    // 2 символа -> 64, 3 -> 512.
                    long long weight = 1;

                    for (int i = 0; i < count; ++i)
                        weight = min(weight * 8LL, 1000000LL);

                    // За линии бота прибавляем очки,
                    // за линии человека вычитаем.
                    total += mine ? weight : -weight;
                }
            }
        }

        // Ограничиваем оценку, чтобы она была
        // меньше оценки реальной победы WIN.
        return max(
            -static_cast<long long>(WIN) / 2,
            min(static_cast<long long>(WIN) / 2, total)
        );
    }

    // Сортируем возможные ходы по перспективности.
    // Хорошие ходы проверяются первыми,
    // что повышает эффективность альфа-бета отсечения.
    vector<Move> orderedMoves(char player) {

        vector<pair<long long, Move>> scored;

        auto moves = board_.candidates();

        for (Move m : moves) {

            // Временно делаем ход.
            board_.put(m, player);

            // Проверяем, приводит ли ход к победе.
            bool victory = board_.winsFrom(m, player);

            // Победный ход получает максимальную оценку.
            // Остальные оцениваются эвристикой.
            long long score = victory
                ? static_cast<long long>(WIN)
                : evaluate();

            // Возвращаем поле в исходное состояние.
            board_.undo(m);

            // Если ход делает человек,
            // меняем знак оценки для сортировки.
            scored.push_back({
                player == me_ ? score : -score,
                m
            });
        }

        // Сначала рассматриваем самые перспективные ходы.
        stable_sort(
            scored.begin(),
            scored.end(),
            [](const auto& a, const auto& b) {
                return a.first > b.first;
            }
        );

        vector<Move> sorted;

        for (auto& item : scored)
            sorted.push_back(item.second);

        return sorted;
    }

    // Основной алгоритм Minimax с альфа-бета отсечением.
    //
    // depth - оставшаяся глубина поиска
    // alpha - лучший результат для максимизирующего игрока
    // beta  - лучший результат для минимизирующего игрока
    // player - игрок, который должен ходить
    // last - предыдущий ход
    // previous - игрок, сделавший предыдущий ход
    long long minimax(
        int depth,
        long long alpha,
        long long beta,
        char player,
        Move last,
        char previous
    ) {
        // Учитываем посещённую позицию.
        ++stats_.nodes;

        // Если предыдущий ход привёл к победе,
        // завершаем поиск в этой ветке.
        if (last.r != -1 && board_.winsFrom(last, previous))
            return previous == me_
                ? WIN + depth
                : -WIN - depth;

        // Если поле заполнено, получилась ничья.
        if (board_.full())
            return 0;

        // Если достигли ограничения глубины,
        // оцениваем текущую позицию эвристикой.
        if (depth == 0)
            return evaluate();

        // Бот максимизирует оценку,
        // человек минимизирует.
        bool maximizing = player == me_;

        long long best = maximizing
            ? LLONG_MIN
            : LLONG_MAX;

        // Получаем отсортированные возможные ходы.
        auto moves = orderedMoves(player);

        for (size_t i = 0; i < moves.size(); ++i) {
            Move m = moves[i];

            // Делаем пробный ход.
            board_.put(m, player);

            // Рекурсивно рассматриваем ответ противника.
            long long val = minimax(
                depth - 1,
                alpha,
                beta,
                opponent(player),
                m,
                player
            );

            // Отменяем пробный ход.
            board_.undo(m);

            if (maximizing) {
                // Бот выбирает максимальную оценку.
                best = max(best, val);
                alpha = max(alpha, best);
            } else {
                // Человек выбирает минимальную оценку.
                best = min(best, val);
                beta = min(beta, best);
            }

            // АЛЬФА-БЕТА ОТСЕЧЕНИЕ.
            //
            // Если alpha >= beta, дальнейшие
            // варианты этой ветки не способны
            // улучшить решение вышестоящего игрока.
            // Поэтому оставшиеся ходы пропускаем.
            if (alpha >= beta) {
                ++stats_.cutoffs;

                // Считаем количество пропущенных
                // непосредственных ходов.
                stats_.skipped += static_cast<long long>(
                    moves.size() - i - 1
                );

                break;
            }
        }

        // Возвращаем лучшую найденную оценку.
        return best;
    }

public:
    // Создаём бота с заданным символом
    // и глубиной поиска.
    Bot(Board& board, char botChar, int depth)
        : board_(board),
          me_(botChar),
          human_(opponent(botChar)),
          depth_(depth) {}

    // Возвращаем статистику поиска.
    SearchStats stats() const {
        return stats_;
    }

    // Выбираем лучший ход бота.
    Move choose() {

        // Обнуляем статистику перед новым поиском.
        stats_ = {};

        auto moves = orderedMoves(me_);

        if (moves.empty())
            return {-1, -1};

        long long best = LLONG_MIN;
        long long alpha = LLONG_MIN;
        long long beta = LLONG_MAX;

        // По умолчанию выбираем первый возможный ход.
        Move answer = moves.front();

        cout << "\nПоиск бота (глубина "
             << depth_ << "):\n";

        // Проверяем возможные ходы бота.
        for (Move m : moves) {

            // Временно ставим символ бота.
            board_.put(m, me_);

            // Запускаем Minimax для ответа человека.
            long long value = minimax(
                depth_ - 1,
                alpha,
                beta,
                human_,
                m,
                me_
            );

            // Отменяем ход.
            board_.undo(m);

            // Если оценка лучше предыдущей,
            // запоминаем этот ход.
            if (value > best) {
                best = value;
                answer = m;
            }

            // Обновляем alpha.
            alpha = max(alpha, best);

            // Выводим оценку хода.
            cout << "  ход (" << m.r + 1
                 << "," << m.c + 1
                 << ") -> оценка " << value << '\n';

            // Если найден гарантированный выигрыш,
            // дальнейший поиск не нужен.
            if (best >= WIN)
                break;
        }

        // Показываем, как сработал поиск.
        cout << "Узлов: " << stats_.nodes
             << "; альфа-бета отсечений: "
             << stats_.cutoffs
             << "; нерассмотренных ходов из-за отсечений: "
             << stats_.skipped << "\n";

        return answer;
    }
};

// Функция безопасного ввода целого числа.
// Если пользователь ввёл неправильное значение,
// программа попросит повторить ввод.
int readInt(const string& prompt, int low, int high) {
    while (true) {
        cout << prompt;

        int x;

        if (cin >> x && x >= low && x <= high)
            return x;

        cout << "Введите целое число от "
             << low << " до " << high << ".\n";

        cin.clear();

        cin.ignore(
            numeric_limits<streamsize>::max(),
            '\n'
        );
    }
}

// Главная функция программы.
int main() {
    try {
        cout << "Крестики-нолики: человек против бота\n";

        // Пользователь задаёт параметры игры.
        int n = readInt("Размер поля N (3..20): ", 3, 20);

        int k = readInt(
            "Сколько подряд для победы K (3..N): ",
            3, n
        );

        int depth = readInt(
            "Глубина поиска (1..5; для больших полей лучше 2-3): ",
            1, 5
        );

        int first = readInt(
            "Кто ходит первым? 1 - человек (X), 2 - бот (X): ",
            1, 2
        );

        // Создаём поле и назначаем символы игрокам.
        Board board(n, k);

        char human = first == 1 ? 'X' : 'O';
        char botChar = first == 1 ? 'O' : 'X';

        // Создаём бота.
        Bot bot(board, botChar, depth);

        // Определяем, чей сейчас ход.
        bool humanTurn = (first == 1);

        // Основной игровой цикл.
        while (true) {

            board.print();

            Move m;

            char who = humanTurn ? human : botChar;

            if (humanTurn) {

                // Ход человека.
                while (true) {
                    int r = readInt("Строка: ", 1, n) - 1;
                    int c = readInt("Столбец: ", 1, n) - 1;

                    if (board.empty(r, c)) {
                        m = {r, c};
                        break;
                    }

                    cout << "Клетка занята.\n";
                }

            } else {

                // Ход бота.
                // Бот сам вычисляет лучший вариант.
                m = bot.choose();

                cout << "Бот " << botChar
                     << " ходит: (" << m.r + 1
                     << "," << m.c + 1 << ")\n";
            }

            // Выполняем выбранный ход.
            board.put(m, who);

            // Проверяем победу.
            if (board.winsFrom(m, who)) {
                board.print();

                cout << (
                    humanTurn
                    ? "Вы победили!"
                    : "Победил бот!"
                ) << '\n';

                break;
            }

            // Проверяем ничью.
            if (board.full()) {
                board.print();
                cout << "Ничья!\n";
                break;
            }

            // Передаём ход другому игроку.
            humanTurn = !humanTurn;
        }

    } catch (const exception& e) {

        // Обрабатываем возможные ошибки.
        cerr << "Ошибка: " << e.what() << '\n';
        return 1;
    }
}
