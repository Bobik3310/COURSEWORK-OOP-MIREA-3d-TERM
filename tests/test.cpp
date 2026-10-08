#include "TwoThreeTree.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <optional>
#include <random>
#include <set>
#include <sstream>
#include <vector>
#include <iostream>

static void check(const TwoThreeTree& tree, const std::set<int>& reference) {
    assert(tree.validate());
    assert(tree.size() == reference.size());
    assert(tree.toVector() == std::vector<int>(reference.begin(), reference.end()));
    for (int x : {-1000, -301, -1, 0, 1, 300, 301, 1000}) {
        auto upper = reference.upper_bound(x);
        auto lower = reference.lower_bound(x);
        const auto expectedNext = upper == reference.end() ? std::nullopt
                                                            : std::optional<int>(*upper);
        const auto expectedPrev = lower == reference.begin() ? std::nullopt
                                                 : std::optional<int>(*std::prev(lower));
        assert(tree.successor(x) == expectedNext);
        assert(tree.predecessor(x) == expectedPrev);
        for (std::size_t m : {0u, 1u, 3u, 20u}) {
            std::vector<int> expected;
            for (auto it = upper; it != reference.end() && expected.size() < m; ++it)
                expected.push_back(*it);
            assert(tree.nextElements(x, m) == expected);
        }
    }
}

int main() {
    TwoThreeTree tree;
    std::set<int> reference;
    check(tree, reference);

    // Возрастание (требует многократных разбиений и роста корня).
    for (int x = 0; x < 1000; ++x) {
        assert(tree.insert(x));
        reference.insert(x);
        check(tree, reference);
    }
    assert(!tree.insert(500));

    // Убывание (многократное объединение, заимствование и понижение корня).
    for (int x = 999; x >= 0; --x) {
        assert(tree.erase(x));
        reference.erase(x);
        check(tree, reference);
    }
    assert(!tree.erase(500));

    // Случайные операции сравниваются с эталонным std::set.
    std::mt19937 generator(2026);
    std::uniform_int_distribution<int> value(-300, 300);
    for (int step = 0; step < 25000; ++step) {
        int key = value(generator);
        if (generator() & 1) {
            bool expected = reference.insert(key).second;
            assert(tree.insert(key) == expected);
        } else {
            bool expected = reference.erase(key) != 0;
            assert(tree.erase(key) == expected);
        }
        check(tree, reference);
        int x = value(generator);
        assert(tree.contains(x) == (reference.find(x) != reference.end()));
        auto next = reference.upper_bound(x);
        assert(tree.successor(x) == (next == reference.end()
                ? std::nullopt : std::optional<int>(*next)));
    }

    // Особые значения, чтение/запись, сохранение дерева при ошибке парсинга.
    tree.insert(std::numeric_limits<int>::min());
    tree.insert(std::numeric_limits<int>::max());
    assert(tree.validate());
    std::stringstream stream;
    stream << tree;
    TwoThreeTree restored;
    stream >> restored;
    assert(restored.toVector() == tree.toVector());
    assert(restored.validate());

    std::stringstream invalid("3 1 2");
    const auto before = restored.toVector();
    invalid >> restored;
    assert(invalid.fail());
    assert(restored.toVector() == before);

    std::stringstream duplicates("5 7 7 2 2 8");
    TwoThreeTree fromSet;
    duplicates >> fromSet;
    assert((fromSet.toVector() == std::vector<int>{2, 7, 8}));

    TwoThreeTree first({-2, 1, 3, 10});
    TwoThreeTree second({1, 2, 10, 11});
    auto merged = first.merged(second);
    assert((merged.toVector() == std::vector<int>{-2, 1, 2, 3, 10, 11}));
    assert((first.toVector() == std::vector<int>{-2, 1, 3, 10}));
    assert(merged.validate());

    TwoThreeTree copied(merged);
    TwoThreeTree moved(std::move(copied));
    assert(moved.validate() && copied.validate());
    assert(moved.toVector() == merged.toVector());
    std::cout << "ALL TESTS PASSED\n";
}
