#include <benchmark/benchmark.h>
#include "rbtree.hpp"
#include <set>
#include <iostream>
#include <random>


#define BM_func(func, cls) \
BENCHMARK_TEMPLATE1(func, cls)->Arg(10); \
BENCHMARK_TEMPLATE1(func, cls)->Arg(100); \
BENCHMARK_TEMPLATE1(func, cls)->Arg(1000); \
BENCHMARK_TEMPLATE1(func, cls)->Arg(10000); \
BENCHMARK_TEMPLATE1(func, cls)->Arg(100000); \
BENCHMARK_TEMPLATE1(func, cls)->Arg(500000); \
BENCHMARK_TEMPLATE1(func, cls)->Arg(1000000)


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

        for (size_t i;i<erase_vals.size();i++) {
            const auto val = erase_vals[i];
            vals.erase(vals.find(val));
        }
    }
}
BM_func(BM_random_set_erase, std::set<size_t>);
BM_func(BM_random_set_erase, set2<size_t>);


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


BENCHMARK_MAIN();
