#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <set>
#include <algorithm>

#define DEBUG 1
#include "rbtree.hpp"
using namespace std;


std::default_random_engine generator;
static void bound_test(const size_t n_vals) {
    RBTreeImpl<int, false> tree;
    RBTreeImpl<int, true> tree_multi;
    std::uniform_int_distribution<int> distribution(-n_vals*3,n_vals);

    std::set<int> snnn;
    std::vector<int> vnnn;
    for (size_t i=0;i<n_vals;i++) {
        auto val = distribution(generator);
        tree.insert(val);
        tree_multi.insert(val);

        snnn.insert(val);
        vnnn.push_back(val);

        ASSERT_EQ(tree.size(), snnn.size());
        ASSERT_EQ(tree_multi.size(), vnnn.size());
    }

    std::sort(vnnn.begin(), vnnn.end());
    tree.check_consistency();
    tree_multi.check_consistency();

    for (size_t i=0;i<200;i++) {
        auto val = distribution(generator);
        auto tlb = tree.lower_bound(val);
        auto tub = tree.upper_bound(val);
        auto slb = snnn.lower_bound(val);
        auto sub = snnn.upper_bound(val);
        auto tmlb = tree_multi.lower_bound(val);
        auto tmub = tree_multi.upper_bound(val);
        auto vlb = std::lower_bound(vnnn.begin(), vnnn.end(), val);
        auto vub = std::upper_bound(vnnn.begin(), vnnn.end(), val);


        ASSERT_EQ(tree.indexof(tlb), std::distance(snnn.begin(), slb));
        ASSERT_EQ(tree.indexof(tub), std::distance(snnn.begin(), sub));
        ASSERT_EQ(tree_multi.indexof(tmlb), std::distance(vnnn.begin(), vlb));
        ASSERT_EQ(tree_multi.indexof(tmub), std::distance(vnnn.begin(), vub));
    }
}

TEST(rbtree_impl, bound) {
    for (size_t i=1;i<=100;i++) {
        bound_test(i);
        bound_test(i * 10);
        bound_test(i * 100);
        bound_test(i * 1000);
    }
}
