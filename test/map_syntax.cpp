#include <gtest/gtest.h>
#include <vector>
#include <map>

#define DEBUG 1
#include "rbtree.hpp"


template<typename M>
static void map_constructor(int mvals) {
    M m;
    std::vector<std::pair<int,int>> hals;

    for (int i=0;i<mvals;i++) {
        m.insert(std::make_pair(i,i));
        hals.push_back(std::make_pair(i,i));
    }
    ASSERT_EQ(m.size(), mvals);

    M ux(hals.begin(), hals.end());
    ux = M({});
    ux = { };

    m.clear();
    ASSERT_TRUE(m.empty());
    ASSERT_EQ(m.size(), 0);

    ASSERT_EQ(m.erase(0), 0);
    EXPECT_THROW(
        m.at(0);,
        std::out_of_range
    );

    m.insert(hals.begin(), hals.end());
    ASSERT_EQ(m.size(), hals.size());
    m.erase(m.begin(), m.end());
    ASSERT_TRUE(m.empty());
}

TEST(map, constructor) {
    map_constructor<std::map<int,int>>(1000);
    map_constructor<map2<int,int>>(1000);
    map_constructor<pmap<int,int>>(1000);
}
