#include "TwoThreeTree.hpp"

#include <fstream>
#include <iostream>
#include <string>

namespace {
void menu() {
    std::cout << "\n=== 2-3 ДЕРЕВО ===\n"
              << "1. Создать дерево (N ключ1 ... ключN)\n"
              << "2. Показать дерево и его уровни\n"
              << "3. Добавить ключ\n"
              << "4. Удалить ключ\n"
              << "5. Найти ключ\n"
              << "6. Следующий и предыдущий ключи\n"
              << "7. Найти m следующих элементов\n"
              << "8. Объединить с другим деревом\n"
              << "9. Прочитать дерево из файла\n"
              << "10. Записать дерево в файл\n"
              << "11. Проверить инварианты дерева\n"
              << "0. Выход\n> ";
}

void show(const std::optional<int>& value) {
    if (value) std::cout << *value;
    else std::cout << "не существует";
}
} // namespace

int main() {
    TwoThreeTree tree;
    int choice = 0;
    while (true) {
        menu();
        if (!(std::cin >> choice) || choice == 0) break;
        int key;
        switch (choice) {
            case 1:
                std::cout << "Введите N и N целых ключей: ";
                if (!(std::cin >> tree)) return 1;
                std::cout << "Создано. Размер: " << tree.size() << '\n';
                break;
            case 2:
                std::cout << "Сериализация: " << tree << '\n';
                tree.printStructure(std::cout);
                break;
            case 3:
                std::cout << "Ключ: ";
                if (!(std::cin >> key)) return 1;
                std::cout << (tree.insert(key) ? "Добавлен\n" : "Уже существует\n");
                break;
            case 4:
                std::cout << "Ключ: ";
                if (!(std::cin >> key)) return 1;
                std::cout << (tree.erase(key) ? "Удалён\n" : "Не найден\n");
                break;
            case 5:
                std::cout << "Ключ: ";
                if (!(std::cin >> key)) return 1;
                std::cout << (tree.contains(key) ? "Найден\n" : "Не найден\n");
                break;
            case 6:
                std::cout << "Ключ: ";
                if (!(std::cin >> key)) return 1;
                std::cout << "Следующий: "; show(tree.successor(key));
                std::cout << "; предыдущий: "; show(tree.predecessor(key));
                std::cout << '\n';
                break;
            case 7: {
                std::size_t m;
                std::cout << "Ключ и m: ";
                if (!(std::cin >> key >> m)) return 1;
                const auto result = tree.nextElements(key, m);
                std::cout << "Найдено " << result.size() << ":";
                for (int value : result) std::cout << ' ' << value;
                std::cout << '\n';
                break;
            }
            case 8: {
                TwoThreeTree other;
                std::cout << "Второе дерево (N ключ1 ... ключN): ";
                if (!(std::cin >> other)) return 1;
                tree = tree.merged(other);
                std::cout << "Объединено. Размер: " << tree.size() << '\n';
                break;
            }
            case 9: {
                std::string path;
                std::cout << "Имя файла: ";
                if (!(std::cin >> path)) return 1;
                std::ifstream file(path);
                if (file >> tree) std::cout << "Прочитано: " << tree.size() << " элементов\n";
                else std::cout << "Ошибка чтения. Исходное дерево сохранено.\n";
                break;
            }
            case 10: {
                std::string path;
                std::cout << "Имя файла: ";
                if (!(std::cin >> path)) return 1;
                std::ofstream file(path);
                if (file && (file << tree << '\n')) std::cout << "Записано\n";
                else std::cout << "Ошибка записи\n";
                break;
            }
            case 11:
                std::cout << (tree.validate() ? "Инварианты соблюдены\n"
                                               : "ОШИБКА в структуре дерева\n");
                break;
            default:
                std::cout << "Неизвестная команда\n";
        }
    }
}
