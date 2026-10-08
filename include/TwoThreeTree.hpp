#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <optional>
#include <vector>

// Общий интерфейс для включения дерева в иерархию классов.
// При интеграции с практической работой №2 интерфейс можно заменить базовым
// классом из той работы, сохранив публичные методы TwoThreeTree.
class AbstractSearchTree {
public:
    virtual ~AbstractSearchTree() = default;
    virtual bool insert(int key) = 0;
    virtual bool erase(int key) = 0;
    virtual bool contains(int key) const = 0;
    virtual std::optional<int> successor(int key) const = 0;
    virtual std::optional<int> predecessor(int key) const = 0;
    virtual std::vector<int> nextElements(int key, std::size_t m) const = 0;
    virtual std::size_t size() const noexcept = 0;
    virtual void printStructure(std::ostream& out) const = 0;
};

class TwoThreeTree final : public AbstractSearchTree {
public:
    TwoThreeTree() = default;
    explicit TwoThreeTree(const std::vector<int>& elements);
    TwoThreeTree(const TwoThreeTree& other);
    TwoThreeTree& operator=(const TwoThreeTree& other);
    TwoThreeTree(TwoThreeTree&& other) noexcept;
    TwoThreeTree& operator=(TwoThreeTree&& other) noexcept;
    ~TwoThreeTree() override = default;

    bool insert(int key) override;   // false, если ключ уже существует
    bool erase(int key) override;    // false, если ключа нет
    bool contains(int key) const override;
    std::optional<int> successor(int key) const override;   // строго больше key
    std::optional<int> predecessor(int key) const override; // строго меньше key
    std::vector<int> nextElements(int key, std::size_t m) const override;
    std::size_t size() const noexcept override { return size_; }
    bool empty() const noexcept { return size_ == 0; }

    std::vector<int> toVector() const;
    TwoThreeTree merged(const TwoThreeTree& other) const; // объединение множеств
    void clear() noexcept;
    void printStructure(std::ostream& out) const override;
    bool validate() const; // Проверка инвариантов для тестов

    // Формат: N K1 K2 ... KN (сортированные уникальные ключи).
    // Потоки могут быть std::cin/std::cout или std::ifstream/std::ofstream.
    friend std::ostream& operator<<(std::ostream& out, const TwoThreeTree& tree);
    friend std::istream& operator>>(std::istream& in, TwoThreeTree& tree);

private:
    struct Node {
        std::vector<int> keys;
        std::vector<std::unique_ptr<Node>> children;
        Node() = default;
        explicit Node(int value) : keys{value} {}
        bool isLeaf() const noexcept { return children.empty(); }
    };

    std::unique_ptr<Node> root_;
    std::size_t size_ = 0;

    static void insertRecursive(Node& node, int key);
    static void splitChild(Node& parent, std::size_t index);
    static bool eraseRecursive(Node& node, int key);
    static void fixUnderflow(Node& parent, std::size_t index);
    static void inorder(const Node* node, std::vector<int>& result);
    static void nextRecursive(const Node* node, int key, std::size_t m,
                              std::vector<int>& result);
    static bool validateRecursive(const Node* node, std::optional<int> lower,
                                  std::optional<int> upper, std::size_t depth,
                                  std::optional<std::size_t>& leafDepth,
                                  std::size_t& keyCount);
};

std::ostream& operator<<(std::ostream& out, const TwoThreeTree& tree);
std::istream& operator>>(std::istream& in, TwoThreeTree& tree);
