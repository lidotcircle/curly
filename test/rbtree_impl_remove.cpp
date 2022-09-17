#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <set>
#include <algorithm>

#define DEBUG 1
#include "rbtree.hpp"
using namespace std;


std::default_random_engine generator;
static void remove_test(const size_t n_vals) {
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
        tree_multi.erase(node);
        ASSERT_EQ(tree_multi.size(), total - i - 1);
        if (i % freq == 0) {
            tree_multi.check_consistency();
        }
    }
}

TEST(rbtree_impl, remove) {
    for (size_t i=1;i<=100;i++) {
        remove_test(i);
        remove_test(i * 10);
        remove_test(i * 100);
        remove_test(i * 1000);
    }
}
