#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <set>
#include <algorithm>

#define DEBUG 1
#include "rbtree.hpp"
using namespace std;


std::default_random_engine generator;
static void insert_test(const size_t n_vals) {
    set2<int> ordered_set;
    std::set<int> stl_set;
    ASSERT_EQ(ordered_set.size(), 0);

    const size_t freq = n_vals / 8 > 0 ? n_vals / 8 : 1;
    std::uniform_int_distribution<int> distribution(-n_vals * 3,n_vals * 3);
    for (size_t i=0;i<n_vals;i++) {
        auto val = distribution(generator);
        ordered_set.insert(val);
        stl_set.insert(val);
    }

    auto stl_iter = stl_set.begin();
    for (auto& val: ordered_set) {
        ASSERT_EQ(*stl_iter++, val);
    }
}

TEST(rbtree_impl, insert) {
    for (size_t i=1;i<=100;i++) {
        insert_test(i);
        insert_test(i * 10);
        insert_test(i * 100);
        insert_test(i * 1000);
    }
}
