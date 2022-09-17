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
    RBTreeImpl<int, void, false> tree;
    RBTreeImpl<int, void, true> tree_multi, tree_multi2;

    ASSERT_EQ(tree.size(), 0);
    ASSERT_EQ(tree_multi.size(), 0);
    ASSERT_EQ(tree.begin(), nullptr);
    ASSERT_EQ(tree_multi.begin(), nullptr);

    const size_t freq = n_vals / 8 > 0 ? n_vals / 8 : 1;
    std::uniform_int_distribution<int> distribution(-n_vals * 3,n_vals * 3);
    std::set<int> snnn;
    std::vector<int> vnnn;
    for (size_t i=0;i<n_vals;i++) {
        auto val = distribution(generator);
        tree.insert(val);
        tree_multi.insert(val);

        if (i % freq == 0) {
            tree.check_consistency();
            tree_multi.check_consistency();
        }

        snnn.insert(val);
        vnnn.push_back(val);

        ASSERT_EQ(tree.size(), snnn.size());
        ASSERT_EQ(tree_multi.size(), vnnn.size());
    }

    tree_multi.copy_to(tree_multi2);
    ASSERT_EQ(tree_multi.size(), tree_multi2.size());
    tree_multi2.check_consistency();
    auto b1=tree_multi.begin(),b2=tree_multi2.begin();
    for (;b1!=nullptr;b1=tree_multi.advance(b1,1),b2=tree_multi2.advance(b2,1)) {
        ASSERT_EQ(b1->value, b2->value);
        ASSERT_EQ(b1->black, b2->black);
        ASSERT_EQ(b1->num_nodes, b2->num_nodes);
    }
    ASSERT_EQ(b2, nullptr);

    auto vp = tree.begin();
    auto v = vp->value;
    for (vp=tree.advance(vp,1);vp!=nullptr;v=vp->value, vp=tree.advance(vp,1))
        ASSERT_LT(v, vp->value);

    auto mvp = tree_multi.begin();
    auto mv = mvp->value;
    for (mvp=tree_multi.advance(mvp,1);mvp!=nullptr;mv=mvp->value, mvp=tree_multi.advance(mvp,1))
        ASSERT_LE(mv, mvp->value);


    std::sort(vnnn.begin(), vnnn.end());

    ASSERT_EQ(tree.size(), snnn.size());
    ASSERT_EQ(tree_multi.size(), vnnn.size());

    size_t i=0; auto tp = tree.begin();
    for (auto sp=snnn.begin();sp!=snnn.end();sp++, i++, tp=tree.advance(tp, 1)) {
        ASSERT_EQ(*sp, tp->value);
        ASSERT_EQ(tree.indexof(tp), i);
    }
    ASSERT_EQ(tp, nullptr);
    tp = tree.advance(tp, -1);
    ASSERT_EQ(tree.indexof(tp), tree.size() - 1);

    i=0; auto tmp = tree_multi.begin();
    for (auto vp=vnnn.begin();vp!=vnnn.end();vp++, i++, tmp=tree_multi.advance(tmp, 1)) {
        ASSERT_EQ(*vp, tmp->value);
        ASSERT_EQ(tree_multi.indexof(tmp), i);
    }
    ASSERT_EQ(tmp, nullptr);
    tmp = tree_multi.advance(tmp, -1);
    ASSERT_EQ(tree_multi.indexof(tmp), tree_multi.size() - 1);
}

TEST(rbtree_impl, insert) {
    for (size_t i=1;i<=100;i++) {
        insert_test(i);
        insert_test(i * 10);
        insert_test(i * 100);
        insert_test(i * 1000);
    }
}
