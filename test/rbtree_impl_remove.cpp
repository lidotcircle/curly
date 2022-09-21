#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <set>
#include <algorithm>

#define DEBUG 1
#include "rbtree.hpp"
using namespace std;


std::default_random_engine generator;
static void multiset_remove_test(const size_t n_vals) {
    RBTreeImpl<int, void, true> tree_multi;
    std::uniform_int_distribution<int> distribution(-n_vals*3,n_vals*3);
    const size_t freq = n_vals / 8 > 0 ? n_vals / 8 : 1;

    for (size_t i=0;i<n_vals;i++) {
        auto val = distribution(generator);
        tree_multi.insert(val);
    }

    std::uniform_int_distribution<int> mdist(0,tree_multi.size());
    const auto total = tree_multi.size();
    for (size_t i=0;i<total;i++) {
        auto pos = mdist(generator) % tree_multi.size();
        auto node = tree_multi.advance(tree_multi.begin(), pos);
        tree_multi.erase(node, false);
        ASSERT_EQ(tree_multi.size(), total - i - 1);
        if (i % freq == 0) {
            tree_multi.check_consistency();
        }
    }
}

static void set_remove_test(const size_t n_vals) {
    RBTreeImpl<int, void, false> tree;
    std::set<int> stl_set;
    std::uniform_int_distribution<int> distribution(-n_vals*3,n_vals*3);
    const size_t freq = n_vals / 8 > 0 ? n_vals / 8 : 1;

    for (size_t i=0;i<n_vals;i++) {
        auto val = distribution(generator);
        tree.insert(val);
        stl_set.insert(val);
    }

    std::uniform_int_distribution<int> mdist(0,tree.size());
    const auto total = tree.size();
    for (size_t i=0;i<total;i++) {
        auto pos = mdist(generator) % tree.size();
        auto node = tree.advance(tree.begin(), pos);
        auto node_next = tree.advance(node, 1);
        auto stl_erase = stl_set.find(node->value.get());
        ASSERT_NE(stl_erase, stl_set.end());

        auto next_node = tree.erase(node, true);
        auto stl_next = stl_set.erase(stl_erase);
        ASSERT_EQ(next_node, node_next);
        if (next_node != nullptr) {
            ASSERT_NE(stl_next, stl_set.end());
            ASSERT_EQ(next_node->value.get(), *stl_next);
        } else {
            ASSERT_EQ(stl_next, stl_set.end());
        }

        ASSERT_EQ(tree.size(), total - i - 1);
        if (i % freq == 0) {
            tree.check_consistency();
        }
    }
}

TEST(rbtree_impl, set_remove_and_return_checking) {
    for (size_t i=1;i<=100;i++) {
        set_remove_test(i);
        set_remove_test(i * 10);
        set_remove_test(i * 100);
        set_remove_test(i * 1000);
    }
}

TEST(rbtree_impl, multi_remove) {
    for (size_t i=1;i<=100;i++) {
        multiset_remove_test(i);
        multiset_remove_test(i * 10);
        multiset_remove_test(i * 100);
        multiset_remove_test(i * 1000);
    }
}
