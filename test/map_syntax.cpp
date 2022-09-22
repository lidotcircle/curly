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

    m = M(std::less<int>());
    m = M(std::allocator<int>());
    m = M(std::less<int>(), std::allocator<int>());
    m = M(hals.begin(), hals.end());
    m = M(hals.begin(), hals.end(), std::less<int>());
    m = M(hals.begin(), hals.end(), std::less<int>(), std::allocator<int>());
    m = M(hals.begin(), hals.end(), std::allocator<int>());
    m = M(m);
    m = M(m, std::allocator<int>());
    m = M(std::initializer_list<std::pair<const int,int>>({ std::make_pair(1,1), {2,2}, {3,3}, {4,4} }));
    // m = M(std::initializer_list<std::pair<int,int>>({ {1,1}, {2,2}, {3,3}, {4,4} }));
    // m = M({ 1, 2, 3, 4 });
    // m = M({ 1, 2, 3, 4 }, std::less<int>());
    // m = M({ 1, 2, 3, 4 }, std::allocator<int>());
    // m = M({ 1, 2, 3, 4 }, std::less<int>(), std::allocator<int>());

    ASSERT_EQ(m.size(), 4);
    auto beg = m.begin();
    ASSERT_EQ(beg->first, 1);
    beg++;
    ASSERT_EQ(beg->first, 2);
    beg++;
    ASSERT_EQ(beg->first, 3);
    beg++;
    ASSERT_EQ(beg->first, 4);
    beg++;
    ASSERT_EQ(beg, m.end());
}


template<typename M>
static M setupmap(int mvals) {
    M m;
    for (size_t i=0;i<mvals;i++) m.insert(std::make_pair(i,i));

    return m;
}


template<typename M>
static void map_assignment(int mvals) {
    M m = setupmap<M>(mvals);
    M b = setupmap<M>(mvals);
    m = b;
    m = std::move(b);
    ASSERT_TRUE(b.empty());
}


template<typename M>
static void map_iterator(int mvals) {
    M m = setupmap<M>(mvals);
    const M cm = setupmap<M>(mvals);

    {
        auto beg = m.begin();
        auto end = m.end();
        auto cm_beg = cm.begin();
        auto cm_end = cm.end();
        auto cm_cbeg = cm.cbegin();
        auto cm_cend = cm.cend();
        ASSERT_EQ(cm_beg, cm_cbeg);
        ASSERT_EQ(cm_end, cm_cend);
        ASSERT_EQ(std::distance(beg, end), m.size());
        ASSERT_EQ(std::distance(cm_beg, cm_end), m.size());
        ASSERT_EQ(std::distance(cm_cbeg, cm_cend), m.size());
    }

    {
        auto rbeg = m.rbegin();
        auto rend = m.rend();
        auto cm_rbeg = cm.rbegin();
        auto cm_rend = cm.rend();
        auto cm_crbeg = cm.crbegin();
        auto cm_crend = cm.crend();
        ASSERT_EQ(cm_rbeg, cm_crbeg);
        ASSERT_EQ(cm_rend, cm_crend);
        ASSERT_EQ(std::distance(rbeg, rend), m.size());
        ASSERT_EQ(std::distance(cm_rbeg, cm_rend), m.size());
        ASSERT_EQ(std::distance(cm_crbeg, cm_crend), m.size());
    }
}

template<typename M>
static void map_capacity(int mvals) {
    M m;
    ASSERT_TRUE(m.empty());
    ASSERT_EQ(m.size(), 0);
    ASSERT_GT(m.max_size(), 0);

    const M cm;
    ASSERT_TRUE(cm.empty());
    ASSERT_EQ(cm.size(), 0);
    ASSERT_GT(cm.max_size(), 0);
}

template<typename M>
static void map_clear(int mvals) {
    M m;
    m.clear();
}

template<typename M>
static void map_insert(int mvals) {
    M m;
    auto val = std::make_pair(0,0);
    auto result = m.insert(val);
    ASSERT_TRUE(result.second);
    ASSERT_EQ(result.first, m.begin());
    result = m.insert(std::move(val));
    ASSERT_FALSE(result.second);
    ASSERT_EQ(result.first, m.begin());

    result.first = m.insert(result.first, std::make_pair(1,1));
    result.first = m.insert(result.first, std::make_pair(2,2));
    result.first = m.insert(m.cbegin(), val);
    result.first = m.insert(m.cbegin(), std::move(val));
    result.first = m.insert(m.cbegin(), std::make_pair(3,3));
    std::vector<std::pair<int,int>> vals = { {5,5}, {6,6}, {7,7}, {8,8} };
    m.insert(vals.begin(), vals.end());
    m.insert({std::make_pair(1,1),{2,2},{3,3},{4,4}});
    // m.insert({1,1},{2,2},{3,3},{4,4}});
    ASSERT_EQ(m.size(), 9);
}

template<typename M>
static void map_emplace(int mvals) {
    M m;
    auto val = std::make_pair(0,0);
    auto result = m.insert(val);
    ASSERT_TRUE(result.second);
    ASSERT_EQ(result.first, m.begin());

    auto v = std::make_pair(100,100);
    auto r1 =  m.emplace(v);
    m.emplace(std::make_pair(100,100));
    auto r2 = m.emplace_hint(result.first, std::make_pair(2, 2));
    ASSERT_TRUE(r1.second);
    ASSERT_EQ(r2, ++m.begin());
    ASSERT_EQ(m.size(), 3);
}

template<typename M>
static void map_erase(int mvals) {
    mvals = std::max(500, mvals);
    M m = setupmap<M>(mvals);
    auto n = m.erase(m.begin());
    ASSERT_EQ(n->first, 1);
    n = m.erase(m.cbegin());
    ASSERT_EQ(n->first, 2);

    auto cbeg = m.cbegin();
    std::advance(cbeg, 100);
    n = m.erase(m.cbegin(), cbeg);
    ASSERT_EQ(n->first, 102);
    n = m.erase(m.begin(), m.end());
    ASSERT_EQ(n, m.end());
}

template<typename M>
static void map_swap(int mvals) {
    M m1 = setupmap<M>(mvals);
    M m2 = setupmap<M>(mvals);
    m1.erase(m1.begin());

    auto beg = m2.begin();
    auto begx = m1.begin();
    m1.swap(m2);

    ASSERT_EQ(beg, m1.begin());
    ASSERT_EQ(beg->first, 0);
    ASSERT_EQ(begx, m2.begin());
    ASSERT_EQ(begx->first, 1);
    ASSERT_EQ(m1.size(), mvals);
    ASSERT_EQ(m2.size(), mvals - 1);
}

template<typename M>
static void map_lookup(int mvals) {
    M m = setupmap<M>(mvals);
    const M cm = setupmap<M>(mvals);

    {
        ASSERT_EQ(m.count(0), 1);
        ASSERT_EQ(m.count(-1), 0);

        auto u = m.find(0);
        ASSERT_NE(u, m.end());
        ASSERT_EQ(u->first, 0);

        auto uend = m.find(-1);
        ASSERT_EQ(uend, m.end());

        auto lb = m.lower_bound(0);
        auto ub = m.upper_bound(0);
        auto rg = m.equal_range(0);
        ASSERT_EQ(lb, rg.first);
        ASSERT_EQ(ub, rg.second);
        ASSERT_EQ(std::distance(lb, ub), 1);
    }

    {
        ASSERT_EQ(cm.count(0), 1);
        ASSERT_EQ(cm.count(-1), 0);

        auto u = cm.find(0);
        ASSERT_NE(u, cm.end());
        ASSERT_EQ(u->first, 0);

        auto uend = cm.find(-1);
        ASSERT_EQ(uend, cm.end());

        auto lb = cm.lower_bound(0);
        auto ub = cm.upper_bound(0);
        auto rg = cm.equal_range(0);
        ASSERT_EQ(lb, rg.first);
        ASSERT_EQ(ub, rg.second);
        ASSERT_EQ(std::distance(lb, ub), 1);
    }
}

template<typename M>
static void map_access(int mvals) {
    M m = setupmap<M>(mvals);
    const M cm = setupmap<M>(mvals);

    ASSERT_THROW(
        m.at(-1);,
        std::out_of_range
    );

    ASSERT_EQ(m.at(0), 0);
    ASSERT_EQ(cm.at(0), 0);

    m[-1];
    ASSERT_EQ(m.at(-1), 0);
    m[-1] = 1;
    ASSERT_EQ(m.at(-1), 1);
}

template<typename M>
static void map_equality(int mvals) {
    M m1 = setupmap<M>(mvals);
    M m2 = setupmap<M>(mvals);
    ASSERT_TRUE(m1 == m2);
    ASSERT_TRUE(m2 == m1);

    m1.erase(m1.begin());
    ASSERT_FALSE(m1 == m2);
    ASSERT_FALSE(m2 == m1);

    ASSERT_TRUE(m1 != m2);
    ASSERT_TRUE(m2 != m1);
}

#define test(name) \
    TEST(map, name) { \
        map_##name<std::map<int,int>>(1000); \
        map_##name<map2<int,int>>(1000); \
        map_##name<pmap<int,int>>(1000); \
    }


test(constructor);
test(assignment);
test(iterator);
test(capacity);
test(insert);
test(emplace);
test(erase);
test(swap);
test(lookup);
test(access);
test(equality);
