#include <benchmark/benchmark.h>
#include "rbtree.hpp"
#include <set>
#include <iostream>
#include <random>
#include <stdexcept>
using namespace curly;


#define MIN_AB(a, b) ((a) > (b) ? (b) : (a))
#define T_ASSERT(expr) if (!(expr)) { benchmark::DoNotOptimize(expr); throw std::logic_error("assert failure"); }

#define REG_SINGLE_TEST(group, cls, n) \
    BENCHMARK_TEMPLATE1(BM_##group, cls)->Arg(n)->Name(#group"/"#cls)

#define BM_func(group, cls) \
REG_SINGLE_TEST(group, cls, 10); \
REG_SINGLE_TEST(group, cls, 100); \
REG_SINGLE_TEST(group, cls, 1000); \
REG_SINGLE_TEST(group, cls, 10000); \
REG_SINGLE_TEST(group, cls, 100000); \
REG_SINGLE_TEST(group, cls, 1000000)


template<typename S>
void BM_advance_random(benchmark::State& state) {
    auto n_vals = state.range(0);
    std::default_random_engine generator(state.range(0));
    S st;
    for (size_t i=0;i<state.range(0);i++) {
        auto val = i;
        st.insert(val);
    }

    std::uniform_int_distribution<size_t> dist(0,n_vals);
    for (auto _: state) {
        auto mval = dist(generator);
        mval %= (st.size() + 1);
        auto it = st.begin();
        auto i2 = it;
        std::advance(i2, mval);
        T_ASSERT(i2 == st.end() || *i2 == mval);
    }
}
BM_func(advance_random, std::set<size_t>);
BM_func(advance_random, set2<size_t>);
BM_func(advance_random, pset<size_t>);


template<typename S>
void BM_distance_random(benchmark::State& state) {
    auto n_vals = state.range(0);
    std::default_random_engine generator(state.range(0));
    std::uniform_int_distribution<size_t> dist(0,n_vals * 3);
    S st;
    for (size_t i=0;i<state.range(0);i++) {
        auto val = dist(generator);
        st.insert(val);
    }

    size_t ans = 0;
    for (auto _: state) {
        auto val1 = dist(generator);
        auto val2 = dist(generator);
        if (val1 > val2) std::swap(val1, val2);
        auto it1 = st.lower_bound(val1);
        auto it2 = st.lower_bound(val2);
        auto dis =  distance(it1, it2);
        ans += dis;
        T_ASSERT(dis >= 0);
    }
    benchmark::DoNotOptimize(ans);
}
BM_func(distance_random, std::set<size_t>);
BM_func(distance_random, set2<size_t>);
BM_func(distance_random, pset<size_t>);


template<typename S>
void BM_advance_distance_random(benchmark::State& state) {
    auto n_vals = state.range(0);
    std::default_random_engine generator(state.range(0));
    std::uniform_int_distribution<size_t> distribution(0,n_vals*3);
    S st;
    for (size_t i=0;i<state.range(0);i++) {
        auto val = distribution(generator);
        st.insert(val);
    }

    std::uniform_int_distribution<size_t> dist(0,n_vals);
    for (auto _: state) {
        auto mval = dist(generator);
        mval %= (st.size() + 1);
        auto it = st.begin();
        auto i2 = it;
        std::advance(i2, mval);
        auto dis = std::distance(it, i2);
        T_ASSERT(mval == dis);
    }
}
BM_func(advance_distance_random, std::set<size_t>);
BM_func(advance_distance_random, set2<size_t>);
BM_func(advance_distance_random, pset<size_t>);


template<typename S>
void BM_copy_incremental(benchmark::State& state) {
    S st;
    for (size_t i=0;i<state.range(0);i++) {
        st.insert(i);
    }

    for (auto _: state) {
        S vals = st;
        vals.clear();
    }
}
BM_func(copy_incremental, std::set<size_t>);
BM_func(copy_incremental, set2<size_t>);
BM_func(copy_incremental, pset<size_t>);


template<typename S>
void BM_copy_random(benchmark::State& state) {
    auto n_vals = state.range(0);
    std::default_random_engine generator(state.range(0));
    std::uniform_int_distribution<size_t> distribution(0,n_vals*3);
    S st;
    for (size_t i=0;i<state.range(0);i++) {
        st.insert(distribution(generator));
    }

    for (auto _: state) {
        S vals = st;
        vals.clear();
    }
}
BM_func(copy_random, std::set<size_t>);
BM_func(copy_random, set2<size_t>);
BM_func(copy_random, pset<size_t>);


template<typename S>
void BM_erase_random(benchmark::State& state) {
    auto n_vals = state.range(0);
    std::default_random_engine generator(state.range(0));
    std::uniform_int_distribution<size_t> distribution(0,n_vals*3);
    std::vector<size_t> erase_vals;;
    std::set<size_t> has_vals;
    S st;
    for (size_t i=0;i<state.range(0);i++) {
        auto val = distribution(generator);
        st.insert(val);
        if (has_vals.find(val) == has_vals.end()) {
            erase_vals.push_back(val);
        }
        has_vals.insert(val);
    }

    for (auto _: state) {
        S vals = st;

        for (size_t i=0;i<erase_vals.size();i++) {
            const auto val = erase_vals[i];
            vals.erase(vals.find(val));
        }
    }
}
BM_func(erase_random, std::set<size_t>);
BM_func(erase_random, set2<size_t>);
BM_func(erase_random, pset<size_t>);


template<typename S>
void BM_insert_incremental(benchmark::State& state) {
    for (auto _: state) {
        S vals;

        for (size_t i=0;i<state.range(0);i++) {
            vals.insert(i);
        }
    }
}
BM_func(insert_incremental, std::set<size_t>);
BM_func(insert_incremental, set2<size_t>);
BM_func(insert_incremental, pset<size_t>);


template<typename S>
void BM_insert_hint_incremental(benchmark::State& state) {
    auto n_vals = state.range(0);

    for (auto _: state) {
        S vals;
        auto hint = vals.end();

        for (size_t i=0;i<n_vals;i++) {
            hint = vals.insert(hint, i);
        }
    }
}
BM_func(insert_hint_incremental, std::set<size_t>);
BM_func(insert_hint_incremental, set2<size_t>);
BM_func(insert_hint_incremental, pset<size_t>);


template<typename S>
void BM_insert_decremental(benchmark::State& state) {
    for (auto _: state) {
        S vals;

        for (size_t i=state.range(0);i>0;i--) {
            vals.insert(i);
        }
    }
}
BM_func(insert_decremental, std::set<size_t>);
BM_func(insert_decremental, set2<size_t>);
BM_func(insert_decremental, pset<size_t>);


template<typename S>
void BM_insert_hint_decremental(benchmark::State& state) {
    auto n_vals = state.range(0);

    for (auto _: state) {
        S vals;
        auto hint = vals.end();

        for (size_t i=0;i<n_vals;i++) {
            hint = vals.insert(hint, n_vals - i);
        }
    }
}
BM_func(insert_hint_decremental, std::set<size_t>);
BM_func(insert_hint_decremental, set2<size_t>);
BM_func(insert_hint_decremental, pset<size_t>);


template<typename S>
void BM_insert_random(benchmark::State& state) {
    auto n_vals = state.range(0);
    std::default_random_engine generator(state.range(0));
    std::uniform_int_distribution<size_t> distribution(0,n_vals*3);

    for (auto _: state) {
        S vals;

        for (size_t i=0;i<n_vals;i++) {
            auto v = distribution(generator);
            vals.insert(v);
        }
    }
}
BM_func(insert_random, std::set<size_t>);
BM_func(insert_random, set2<size_t>);
BM_func(insert_random, pset<size_t>);


template<typename S>
void BM_insert_hint_random(benchmark::State& state) {
    auto n_vals = state.range(0);
    std::default_random_engine generator(state.range(0));
    std::uniform_int_distribution<size_t> distribution(0,n_vals*3);

    for (auto _: state) {
        S vals;
        auto hint = vals.end();

        for (size_t i=0;i<n_vals;i++) {
            auto v = distribution(generator);
            hint = vals.insert(hint, v);
        }
    }
}
BM_func(insert_hint_random, std::set<size_t>);
BM_func(insert_hint_random, set2<size_t>);
BM_func(insert_hint_random, pset<size_t>);


BENCHMARK_MAIN();
