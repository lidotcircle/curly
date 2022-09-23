<div align="center">
  <h1><code>curly</code></h1>

  <p>
    <a href="https://github.com/lidotcircle/curly/actions"><img src="https://github.com/lidotcircle/curly/actions/workflows/test.yml/badge.svg" alt="CI Status" /></a>
    sorted container template library
  </p>
</div>

### Introduction

curly implements same sorted containers which respectively compatible with `std::map`, `std::multimap`, `std::set` and `std::multiset`.
The difference between these containers and containers of STL is iterator of
these containers is '**random access**'(not completely, because which isn't satisfying O(1) random access, but O(lg n)).

Following figure is benchmark result of `insert`, `erase`, `copy`, `std::distance` and `std::advance` between different set container

![benchmark result](https://github.com/lidotcircle/curly/releases/download/master/benchmark_set_vs_set2_vs_pset.png)

Time complexity comparison between `std::set`, `curly::pset` and `curly::set2`

| Operation | `std::set` | `curly::set2` | `curly::pset` |
|:--:|--|--|--|
| **insert** | O(lg n)  | O(lg n) | O(lg n) |
| **erase**  | O(lg n), amortized O(n)  | O(lg n) | O(lg n), amortized O(n) |
| **copy assignment**  | O(n)  | O(n) | O(n) |
| **std::distance**  | O(n)  | O(lg n) | O(n) |
| **std::advance(iter, k)**  | O(k)  | O(lg n) | O(k) |

### Usage

just include [rbtree.hpp](./include/rbtree.hpp)

| STL container | curly container | curly container without *random access iterator* |
|--|--|--|
| `std::set` | `curly::pset` | `curly::set2` |
| `std::multiset` | `curly::pmultiset` | `curly::multiset2` |
| `std::map` | `curly::pmap` | `curly::map2` |
| `std::multimap` | `curly::pmultimap` | `curly::multimap2` |

