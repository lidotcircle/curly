#include <gtest/gtest.h>
#include <vector>
#include <set>

#define DEBUG 1
#include "rbtree.hpp"


template<typename M>
static void set_constructor(int mvals) {
    M m;
    std::vector<int> hals;

    for (int i=0;i<mvals;i++) {
        m.insert(i);
        hals.push_back(i);
    }
    ASSERT_EQ(m.size(), mvals);

    M ux(hals.begin(), hals.end());
    ux = M({});
    ux = { };

    m.clear();
    ASSERT_TRUE(m.empty());
    ASSERT_EQ(m.size(), 0);

    ASSERT_EQ(m.erase(0), 0);

    m.insert(hals.begin(), hals.end());
    ASSERT_EQ(m.size(), hals.size());
    m.erase(m.begin(), m.end());
    ASSERT_TRUE(m.empty());
}

TEST(set, constructor) {
    set_constructor<std::set<int>>(1000);
    set_constructor<set2<int>>(1000);
    set_constructor<pset<int>>(1000);
}
