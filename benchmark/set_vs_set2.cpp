#include <benchmark/benchmark.h>
#include "rbtree.hpp"
#include <set>
#include <iostream>
#include <random>
#include <assert.h>


#define MIN_AB(a, b) ((a) > (b) ? (b) : (a))

#define BM_func(func, cls) \
BENCHMARK_TEMPLATE1(func, cls)->Arg(10); \
BENCHMARK_TEMPLATE1(func, cls)->Arg(100); \
BENCHMARK_TEMPLATE1(func, cls)->Arg(1000); \
BENCHMARK_TEMPLATE1(func, cls)->Arg(10000); \
BENCHMARK_TEMPLATE1(func, cls)->Arg(100000); \
BENCHMARK_TEMPLATE1(func, cls)->Arg(500000); \
BENCHMARK_TEMPLATE1(func, cls)->Arg(1000000)


template<typename S>
void BM_random_move(benchmark::State& state) {
    auto n_vals = state.range(0);
    std::default_random_engine generator(state.range(0));
    S st;
    for (size_t i=0;i<state.range(0);i++) {
        auto val = i;
        st.insert(val);
    }

    for (auto _: state) {
        S vals = st;
        std::uniform_int_distribution<size_t> dist(0,n_vals);

        auto mval = dist(generator);
        mval %= (st.size() + 1);
        auto it = st.begin();
        if constexpr (std::is_same_v<S,std::set<size_t>>) {
            auto i2 = it;
            for(size_t k=0;k<mval;k++, i2++);
            if (i2 != st.end())
                assert(*i2 == mval);
        } else {
            auto i2 = it + mval;
            if (i2 != st.end())
                assert(*i2 == mval);
        }
    }
}
BM_func(BM_random_move, std::set<size_t>);
BM_func(BM_random_move, set2<size_t>);
BM_func(BM_random_move, pset<size_t>);


template<typename S>
void BM_random_value_position(benchmark::State& state) {
    auto n_vals = state.range(0);
    std::default_random_engine generator(state.range(0));
    std::uniform_int_distribution<size_t> dist(0,n_vals * 3);
    S st;
    for (size_t i=0;i<state.range(0);i++) {
        auto val = dist(generator);
        st.insert(val);
    }

    for (auto _: state) {
        S vals = st;

        auto mval = dist(generator);
        auto it = st.lower_bound(mval);
        auto dis = distance(st.begin(), it);
    }
}
BM_func(BM_random_value_position, std::set<size_t>);
BM_func(BM_random_value_position, set2<size_t>);
BM_func(BM_random_value_position, pset<size_t>);


template<typename S>
void BM_random_move_dis(benchmark::State& state) {
    auto n_vals = state.range(0);
    std::default_random_engine generator(state.range(0));
    std::uniform_int_distribution<size_t> distribution(0,n_vals*3);
    S st;
    for (size_t i=0;i<state.range(0);i++) {
        auto val = distribution(generator);
        st.insert(val);
    }

    for (auto _: state) {
        S vals = st;
        std::uniform_int_distribution<size_t> dist(0,n_vals);

        auto mval = dist(generator);
        mval %= (st.size() + 1);
        auto it = st.begin();
        if constexpr (std::is_same_v<S,std::set<size_t>>) {
            auto i2 = it;
            for(size_t k=0;k<mval;k++, i2++);
            auto dis = distance(it, i2);
            assert(mval == dis);
        } else {
            auto i2 = it + mval;
            auto dis = distance(it, i2);
            assert(mval == dis);
        }
    }
}
BM_func(BM_random_move_dis, std::set<size_t>);
BM_func(BM_random_move_dis, set2<size_t>);
BM_func(BM_random_move_dis, pset<size_t>);


template<typename S>
void BM_set_copy(benchmark::State& state) {
    S st;
    for (size_t i=0;i<state.range(0);i++) {
        st.insert(i);
    }

    for (auto _: state) {
        S vals = st;
        vals.clear();
    }
}
BM_func(BM_set_copy, std::set<size_t>);
BM_func(BM_set_copy, set2<size_t>);
BM_func(BM_set_copy, pset<size_t>);


template<typename S>
void BM_random_set_copy(benchmark::State& state) {
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
BM_func(BM_random_set_copy, std::set<size_t>);
BM_func(BM_random_set_copy, set2<size_t>);
BM_func(BM_random_set_copy, pset<size_t>);


template<typename S>
void BM_random_set_erase(benchmark::State& state) {
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
BM_func(BM_random_set_erase, std::set<size_t>);
BM_func(BM_random_set_erase, set2<size_t>);
BM_func(BM_random_set_erase, pset<size_t>);


template<typename S>
void BM_set_inc(benchmark::State& state) {
    for (auto _: state) {
        S vals;

        for (size_t i=0;i<state.range(0);i++) {
            vals.insert(i);
        }
    }
}
BM_func(BM_set_inc, std::set<size_t>);
BM_func(BM_set_inc, set2<size_t>);
BM_func(BM_set_inc, pset<size_t>);


template<typename S>
void BM_set_inc_hint(benchmark::State& state) {
    auto n_vals = state.range(0);

    for (auto _: state) {
        S vals;
        auto hint = vals.end();

        for (size_t i=0;i<n_vals;i++) {
            hint = vals.insert(hint, i);
        }
    }
}
BM_func(BM_set_inc_hint, std::set<size_t>);
BM_func(BM_set_inc_hint, set2<size_t>);
BM_func(BM_set_inc_hint, pset<size_t>);


template<typename S>
void BM_set_dec(benchmark::State& state) {
    for (auto _: state) {
        S vals;

        for (size_t i=state.range(0);i>0;i--) {
            vals.insert(i);
        }
    }
}
BM_func(BM_set_dec, std::set<size_t>);
BM_func(BM_set_dec, set2<size_t>);
BM_func(BM_set_dec, pset<size_t>);


template<typename S>
void BM_set_dec_hint(benchmark::State& state) {
    auto n_vals = state.range(0);

    for (auto _: state) {
        S vals;
        auto hint = vals.end();

        for (size_t i=0;i<n_vals;i++) {
            hint = vals.insert(hint, n_vals - i);
        }
    }
}
BM_func(BM_set_dec_hint, std::set<size_t>);
BM_func(BM_set_dec_hint, set2<size_t>);
BM_func(BM_set_dec_hint, pset<size_t>);


template<typename S>
void BM_set_random(benchmark::State& state) {
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
BM_func(BM_set_random, std::set<size_t>);
BM_func(BM_set_random, set2<size_t>);
BM_func(BM_set_random, pset<size_t>);


template<typename S>
void BM_set_random_hint(benchmark::State& state) {
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
BM_func(BM_set_random_hint, std::set<size_t>);
BM_func(BM_set_random_hint, set2<size_t>);
BM_func(BM_set_random_hint, pset<size_t>);


BENCHMARK_MAIN();
