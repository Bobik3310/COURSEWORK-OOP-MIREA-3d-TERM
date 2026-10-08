#include "TwoThreeTree.hpp"

#include <algorithm>
#include <istream>
#include <ostream>
#include <queue>
#include <utility>

TwoThreeTree::TwoThreeTree(const std::vector<int>& elements) {
    for (int key : elements) insert(key);
}

TwoThreeTree::TwoThreeTree(const TwoThreeTree& other)
    : TwoThreeTree(other.toVector()) {}

TwoThreeTree& TwoThreeTree::operator=(const TwoThreeTree& other) {
    if (this != &other) {
        TwoThreeTree copy(other);
        *this = std::move(copy);
    }
    return *this;
}

TwoThreeTree::TwoThreeTree(TwoThreeTree&& other) noexcept
    : root_(std::move(other.root_)), size_(std::exchange(other.size_, 0)) {}

TwoThreeTree& TwoThreeTree::operator=(TwoThreeTree&& other) noexcept {
    if (this != &other) {
        root_ = std::move(other.root_);
        size_ = std::exchange(other.size_, 0);
    }
    return *this;
}

void TwoThreeTree::clear() noexcept {
    root_.reset();
    size_ = 0;
}

bool TwoThreeTree::contains(int key) const {
    const Node* node = root_.get();
    while (node) {
        auto it = std::lower_bound(node->keys.begin(), node->keys.end(), key);
        if (it != node->keys.end() && *it == key) return true;
        if (node->isLeaf()) return false;
        node = node->children[static_cast<std::size_t>(it - node->keys.begin())].get();
    }
    return false;
}

// Временное переполнение до трех ключей разрешается на пути обратно вверх.
void TwoThreeTree::insertRecursive(Node& node, int key) {
    auto it = std::lower_bound(node.keys.begin(), node.keys.end(), key);
    const auto index = static_cast<std::size_t>(it - node.keys.begin());
    if (node.isLeaf()) {
        node.keys.insert(it, key);
    } else {
        insertRecursive(*node.children[index], key);
        if (node.children[index]->keys.size() == 3) splitChild(node, index);
    }
}

// [a | b | c] превращается в [a] + b(в родителя) + [c].
void TwoThreeTree::splitChild(Node& parent, std::size_t index) {
    Node& left = *parent.children[index];
    const int middle = left.keys[1];
    auto right = std::make_unique<Node>(left.keys[2]);
    left.keys.resize(1);
    if (!left.isLeaf()) {
        right->children.push_back(std::move(left.children[2]));
        right->children.push_back(std::move(left.children[3]));
        left.children.resize(2);
    }
    parent.keys.insert(parent.keys.begin() + static_cast<std::ptrdiff_t>(index), middle);
    parent.children.insert(parent.children.begin() + static_cast<std::ptrdiff_t>(index + 1),
                           std::move(right));
}

bool TwoThreeTree::insert(int key) {
    if (contains(key)) return false;
    if (!root_) {
        root_ = std::make_unique<Node>(key);
    } else {
        insertRecursive(*root_, key);
        if (root_->keys.size() == 3) {
            auto newRoot = std::make_unique<Node>();
            newRoot->children.push_back(std::move(root_));
            splitChild(*newRoot, 0);
            root_ = std::move(newRoot);
        }
    }
    ++size_;
    return true;
}

// Восстанавливаем узел с 0 ключей: занимаем ключ у соседа с 2 ключами
// либо объединяем узлы через разделитель из родителя.
void TwoThreeTree::fixUnderflow(Node& parent, std::size_t index) {
    Node& deficient = *parent.children[index];

    // Заимствование из левого соседа.
    if (index > 0 && parent.children[index - 1]->keys.size() == 2) {
        Node& left = *parent.children[index - 1];
        deficient.keys.insert(deficient.keys.begin(), parent.keys[index - 1]);
        parent.keys[index - 1] = left.keys.back();
        left.keys.pop_back();
        if (!left.isLeaf()) {
            deficient.children.insert(deficient.children.begin(),
                                      std::move(left.children.back()));
            left.children.pop_back();
        }
        return;
    }

    // Заимствование из правого соседа.
    if (index + 1 < parent.children.size() &&
        parent.children[index + 1]->keys.size() == 2) {
        Node& right = *parent.children[index + 1];
        deficient.keys.push_back(parent.keys[index]);
        parent.keys[index] = right.keys.front();
        right.keys.erase(right.keys.begin());
        if (!right.isLeaf()) {
            deficient.children.push_back(std::move(right.children.front()));
            right.children.erase(right.children.begin());
        }
        return;
    }

    // У соседей только по одному ключу: объединяем через родителя.
    if (index > 0) {
        Node& left = *parent.children[index - 1];
        left.keys.push_back(parent.keys[index - 1]);
        left.keys.insert(left.keys.end(), deficient.keys.begin(), deficient.keys.end());
        for (auto& child : deficient.children)
            left.children.push_back(std::move(child));
        parent.keys.erase(parent.keys.begin() + static_cast<std::ptrdiff_t>(index - 1));
        parent.children.erase(parent.children.begin() + static_cast<std::ptrdiff_t>(index));
    } else {
        Node& right = *parent.children[1];
        deficient.keys.push_back(parent.keys[0]);
        deficient.keys.insert(deficient.keys.end(), right.keys.begin(), right.keys.end());
        for (auto& child : right.children)
            deficient.children.push_back(std::move(child));
        parent.keys.erase(parent.keys.begin());
        parent.children.erase(parent.children.begin() + 1);
    }
}

bool TwoThreeTree::eraseRecursive(Node& node, int key) {
    auto it = std::lower_bound(node.keys.begin(), node.keys.end(), key);
    const auto index = static_cast<std::size_t>(it - node.keys.begin());

    if (it != node.keys.end() && *it == key) {
        if (node.isLeaf()) {
            node.keys.erase(it);
        } else {
            // Ключ внутреннего узла заменяем его предшественником из листа.
            Node* previous = node.children[index].get();
            while (!previous->isLeaf()) previous = previous->children.back().get();
            const int predecessorKey = previous->keys.back();
            node.keys[index] = predecessorKey;
            eraseRecursive(*node.children[index], predecessorKey);
            if (node.children[index]->keys.empty()) fixUnderflow(node, index);
        }
        return true;
    }

    if (node.isLeaf()) return false;
    if (!eraseRecursive(*node.children[index], key)) return false;
    if (node.children[index]->keys.empty()) fixUnderflow(node, index);
    return true;
}

bool TwoThreeTree::erase(int key) {
    if (!root_ || !eraseRecursive(*root_, key)) return false;
    --size_;
    if (root_->keys.empty()) {
        if (root_->isLeaf()) root_.reset();
        else root_ = std::move(root_->children.front()); // понижение высоты
    }
    return true;
}

std::optional<int> TwoThreeTree::successor(int key) const {
    std::optional<int> result;
    const Node* node = root_.get();
    while (node) {
        const auto it = std::upper_bound(node->keys.begin(), node->keys.end(), key);
        const auto i = static_cast<std::size_t>(it - node->keys.begin());
        if (it != node->keys.end()) result = *it;
        node = node->isLeaf() ? nullptr : node->children[i].get();
    }
    return result;
}

std::optional<int> TwoThreeTree::predecessor(int key) const {
    std::optional<int> result;
    const Node* node = root_.get();
    while (node) {
        const auto it = std::lower_bound(node->keys.begin(), node->keys.end(), key);
        const auto i = static_cast<std::size_t>(it - node->keys.begin());
        if (i > 0) result = node->keys[i - 1];
        node = node->isLeaf() ? nullptr : node->children[i].get();
    }
    return result;
}

void TwoThreeTree::inorder(const Node* node, std::vector<int>& result) {
    if (!node) return;
    for (std::size_t i = 0; i < node->keys.size(); ++i) {
        if (!node->isLeaf()) inorder(node->children[i].get(), result);
        result.push_back(node->keys[i]);
    }
    if (!node->isLeaf()) inorder(node->children.back().get(), result);
}

std::vector<int> TwoThreeTree::toVector() const {
    std::vector<int> result;
    result.reserve(size_);
    inorder(root_.get(), result);
    return result;
}

// Отсекаем все поддеревья, ключи которых гарантированно <= key.
// Обход останавливается, как только собрано m значений.
void TwoThreeTree::nextRecursive(const Node* node, int key, std::size_t m,
                                 std::vector<int>& result) {
    if (!node || result.size() >= m) return;
    for (std::size_t i = 0; i < node->keys.size(); ++i) {
        if (node->keys[i] <= key) continue;
        if (!node->isLeaf()) nextRecursive(node->children[i].get(), key, m, result);
        if (result.size() >= m) return;
        result.push_back(node->keys[i]);
        if (result.size() >= m) return;
    }
    if (!node->isLeaf()) nextRecursive(node->children.back().get(), key, m, result);
}

std::vector<int> TwoThreeTree::nextElements(int key, std::size_t m) const {
    std::vector<int> result;
    result.reserve(std::min(m, size_));
    nextRecursive(root_.get(), key, m, result);
    return result;
}

TwoThreeTree TwoThreeTree::merged(const TwoThreeTree& other) const {
    TwoThreeTree result(*this);
    for (int key : other.toVector()) result.insert(key);
    return result;
}

void TwoThreeTree::printStructure(std::ostream& out) const {
    if (!root_) {
        out << "(пустое дерево)\n";
        return;
    }
    std::queue<const Node*> nodes;
    nodes.push(root_.get());
    std::size_t level = 0;
    while (!nodes.empty()) {
        std::size_t onLevel = nodes.size();
        out << "Уровень " << level++ << ": ";
        while (onLevel--) {
            const Node* node = nodes.front();
            nodes.pop();
            out << '[';
            for (std::size_t i = 0; i < node->keys.size(); ++i) {
                if (i) out << '|';
                out << node->keys[i];
            }
            out << "] ";
            for (const auto& child : node->children) nodes.push(child.get());
        }
        out << '\n';
    }
}

bool TwoThreeTree::validateRecursive(const Node* node, std::optional<int> lower,
                                     std::optional<int> upper, std::size_t depth,
                                     std::optional<std::size_t>& leafDepth,
                                     std::size_t& keyCount) {
    if (!node || node->keys.size() < 1 || node->keys.size() > 2) return false;
    for (std::size_t i = 0; i < node->keys.size(); ++i) {
        if ((lower && node->keys[i] <= *lower) ||
            (upper && node->keys[i] >= *upper) ||
            (i > 0 && node->keys[i - 1] >= node->keys[i])) return false;
    }
    keyCount += node->keys.size();
    if (node->isLeaf()) {
        if (!leafDepth) leafDepth = depth;
        return *leafDepth == depth;
    }
    if (node->children.size() != node->keys.size() + 1) return false;
    for (std::size_t i = 0; i < node->children.size(); ++i) {
        const auto childLower = i == 0 ? lower : std::optional<int>(node->keys[i - 1]);
        const auto childUpper = i == node->keys.size() ? upper
                                                       : std::optional<int>(node->keys[i]);
        if (!validateRecursive(node->children[i].get(), childLower, childUpper,
                               depth + 1, leafDepth, keyCount)) return false;
    }
    return true;
}

bool TwoThreeTree::validate() const {
    if (!root_) return size_ == 0;
    std::size_t counted = 0;
    std::optional<std::size_t> leafDepth;
    return validateRecursive(root_.get(), std::nullopt, std::nullopt, 0,
                             leafDepth, counted) && counted == size_;
}

std::ostream& operator<<(std::ostream& out, const TwoThreeTree& tree) {
    out << tree.size_;
    for (int value : tree.toVector()) out << ' ' << value;
    return out;
}

std::istream& operator>>(std::istream& in, TwoThreeTree& tree) {
    long long count;
    if (!(in >> count)) return in;
    if (count < 0) {
        in.setstate(std::ios::failbit);
        return in;
    }
    TwoThreeTree parsed;
    for (long long i = 0; i < count; ++i) {
        int key;
        if (!(in >> key)) return in; // исходное дерево не меняется при ошибке
        parsed.insert(key);         // повторы элементов множества игнорируются
    }
    tree = std::move(parsed);
    return in;
}
