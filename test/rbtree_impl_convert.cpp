#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <set>
#include <algorithm>

#define DEBUG 1
#include "rbtree.hpp"
using namespace std;
using namespace curly;


std::default_random_engine generator;
static void convert_test(const size_t n_vals) {
    RBTreeImpl<int, void, true> tree_multi, co2;
    std::uniform_int_distribution<int> distribution(-n_vals*3,n_vals);

    for (size_t i=0;i<n_vals;i++) {
        auto val = distribution(generator);
        tree_multi.insert(val);
    }

    tree_multi.copy_to(co2);
    tree_multi.convert2BST();

    tree_multi.check_consistency();
    ASSERT_EQ(tree_multi.size(), co2.size());
}

TEST(rbtree_impl, convert) {
    for (size_t i=1;i<=100;i++) {
        convert_test(i);
        convert_test(i * 10);
        convert_test(i * 100);
        convert_test(i * 1000);
    }
}
