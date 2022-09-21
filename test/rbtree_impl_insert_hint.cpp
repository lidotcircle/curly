#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <set>
#include <algorithm>

#define DEBUG 1
#include "rbtree.hpp"
using namespace std;


std::default_random_engine generator;
static void insert_hint_test(const size_t n_vals) {
    RBTreeImpl<int, void, false> tree;
    RBTreeImpl<int, void, true> tree_multi;

    ASSERT_EQ(tree.size(), 0);
    ASSERT_EQ(tree_multi.size(), 0);
    ASSERT_EQ(tree.begin(), nullptr);
    ASSERT_EQ(tree_multi.begin(), nullptr);

    decltype(tree.begin()) hint = nullptr;
    decltype(tree_multi.begin()) hint_multi = nullptr;

    const size_t freq = n_vals / 8 > 0 ? n_vals / 8 : 1;
    std::uniform_int_distribution<int> distribution(-n_vals * 3,n_vals * 3);
    std::set<int> snnn;
    std::vector<int> vnnn;
    for (size_t i=0;i<n_vals;i++) {
        auto val = distribution(generator);
        auto v1 = tree.insert(hint, val);
        auto v2 = tree_multi.insert(hint_multi, val);
        hint = v1.first;
        hint_multi = v2.first;
        ASSERT_TRUE(v2.second);

        if (i % freq == 0) {
            tree.check_consistency();
            tree_multi.check_consistency();
        }

        snnn.insert(val);
        vnnn.push_back(val);

        ASSERT_EQ(tree.size(), snnn.size());
        ASSERT_EQ(tree_multi.size(), vnnn.size());
    }

    auto vp = tree.begin();
    auto v = vp->value.get();
    for (vp=tree.advance(vp,1);vp!=nullptr;v=vp->value.get(), vp=tree.advance(vp,1))
        ASSERT_LT(v, vp->value.get());

    auto mvp = tree_multi.begin();
    auto mv = mvp->value.get();
    for (mvp=tree_multi.advance(mvp,1);mvp!=nullptr;mv=mvp->value.get(), mvp=tree_multi.advance(mvp,1))
        ASSERT_LE(mv, mvp->value.get());


    std::sort(vnnn.begin(), vnnn.end());

    ASSERT_EQ(tree.size(), snnn.size());
    ASSERT_EQ(tree_multi.size(), vnnn.size());

    size_t i=0; auto tp = tree.begin();
    for (auto sp=snnn.begin();sp!=snnn.end();sp++, i++, tp=tree.advance(tp, 1)) {
        ASSERT_EQ(*sp, tp->value.get());
        ASSERT_EQ(tree.indexof(tp), i);
    }
    ASSERT_EQ(tp, nullptr);
    tp = tree.advance(tp, -1);
    ASSERT_EQ(tree.indexof(tp), tree.size() - 1);

    i=0; auto tmp = tree_multi.begin();
    for (auto vp=vnnn.begin();vp!=vnnn.end();vp++, i++, tmp=tree_multi.advance(tmp, 1)) {
        ASSERT_EQ(*vp, tmp->value.get());
        ASSERT_EQ(tree_multi.indexof(tmp), i);
    }
    ASSERT_EQ(tmp, nullptr);
    tmp = tree_multi.advance(tmp, -1);
    ASSERT_EQ(tree_multi.indexof(tmp), tree_multi.size() - 1);
}

TEST(rbtree_impl, insert_hint) {
    for (size_t i=1;i<=100;i++) {
        insert_hint_test(i);
        insert_hint_test(i * 10);
        insert_hint_test(i * 100);
        insert_hint_test(i * 1000);
    }
}
