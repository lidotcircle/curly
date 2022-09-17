#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <set>
#include <algorithm>

#define DEBUG 1
#include "rbtree.hpp"
using namespace std;


std::default_random_engine generator;
static void move_test(const size_t n_vals)
{
    RBTreeImpl<int, void, true> tree_multi;
    std::uniform_int_distribution<int> distribution(-100000,100000);

    for (size_t i=0;i<n_vals;i++) {
        auto val = distribution(generator);
        tree_multi.insert(val);
    }

    auto node = tree_multi.begin();
    long pos = 0;
    std::uniform_int_distribution<int> mdist(0,tree_multi.size());
    for (size_t i=0;i<10000;i++) {
        auto val = mdist(generator) - pos;
        node = tree_multi.advance(node, val);
        pos = pos + val;
        ASSERT_EQ(tree_multi.indexof(node), pos);
    }
}

TEST(rbtree_impl, move) {
    for (size_t i=1;i<=100;i++) {
        move_test(i);
        move_test(i * 10);
        move_test(i * 100);
        move_test(i * 1000);
    }
}
