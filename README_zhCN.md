<div align="center">
  <h1><code>curly</code></h1>

  <p>
    <a href="https://github.com/lidotcircle/curly/actions"><img src="https://github.com/lidotcircle/curly/actions/workflows/test.yml/badge.svg" alt="CI Status" /></a>
    C++有序容器模板库
  </p>
</div>

### 介绍

curly 实现了`std::map`, `std::multimap`, `std::set` 和 `std::multiset` 的替代品。区别在于 curly 实现的容器的迭代器是 **random access iterartor**(严格来说并不是, 因为单个寻址操作的时间复杂度为 O(lg n), 而指针为 O(1))。

下图是和STL容器的性能对比, 包括`insert`, `erase`, `copy`, `std::distance3` 和 `std::advance`。

![benchmark result](https://github.com/lidotcircle/curly/releases/download/master/benchmark_set_vs_set2_vs_pset.png)

时间复杂度的对比 `std::set` vs `curly::pset` vs `curly::set2` (`curly::set2`的迭代器和`std::set`一样是`bidirectional iterator`, 仅用于测试 红黑树 实现的性能)

| 操作 | `std::set` | `curly::set2` | `curly::pset` |
|:--:|--|--|--|
| **insert** | O(lg n)  | O(lg n) | O(lg n) |
| **erase**  | O(lg n), amortized O(n)  | O(lg n) | O(lg n), amortized O(n) |
| **copy assignment**  | O(n)  | O(n) | O(n) |
| **std::distance**  | O(n)  | O(lg n) | O(n) |
| **std::advance(iter, k)**  | O(k)  | O(lg n) | O(k) |

### 用法

包含单个文件 include [rbtree.hpp](./include/rbtree.hpp) 即可

| STL container | curly container | curly container without *random access iterator* |
|--|--|--|
| `std::set` | `curly::pset` | `curly::set2` |
| `std::multiset` | `curly::pmultiset` | `curly::multiset2` |
| `std::map` | `curly::pmap` | `curly::map2` |
| `std::multimap` | `curly::pmultimap` | `curly::multimap2` |

