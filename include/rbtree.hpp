#pragma once
#include <functional>
#include <utility>
#include <iterator>
#include <type_traits>
#include <memory>
#include <stdexcept>
#include <limits>
#if __cplusplus >= 201703
#include <memory_resource>
#endif // __cplusplus >= 201703
#if __cplusplus >= 202002
#include <concepts>
#endif // __cplusplus >= 202002


#ifdef DEBUG
#include <assert.h>
#include <queue>
#define RB_ASSERT(x) assert(x)
#else
#define RB_ASSERT(x)
#endif


namespace curly {

template<typename T1, typename T2>
struct is_same_value_type:
    std::is_same<
        typename std::remove_reference<typename std::remove_const<typename std::remove_reference<T1>::type>::type>::type,
        typename std::remove_reference<typename std::remove_const<typename std::remove_reference<T2>::type>::type>::type
    > {};
#if __cplusplus >= 202002
template <typename T1, typename T2>
concept C_is_same_value_type = is_same_value_type<T1,T2>::value;
#endif // __cplusplus >= 202002

template<typename K, typename V>
struct RBTreeValueKV: public std::pair<const K,V> {
public:
    using key_type = K;
    using storage_type_base = std::pair<const K,V>;

    RBTreeValueKV() = delete;

#if __cplusplus >= 202002
    template<typename T>
        requires ( std::constructible_from<std::pair<const K,V>, T&&> )
#else
    template<typename T, typename std::enable_if<!std::is_convertible<T,RBTreeValueKV>::value,bool>::type = true>
#endif // __cplusplus >= 202002
    RBTreeValueKV(T&& v): std::pair<const K,V>(std::forward<T>(v)) {}

    template<typename T1, typename T2>
 #if __cplusplus >= 202002
        requires ( std::constructible_from<const K,T1&&> && std::constructible_from<V,T2&&> )
#endif // __cplusplus >= 202002
    RBTreeValueKV(T1&& v1, T2&& v2): std::pair<const K,V>(std::forward<T1>(v1), std::forward<T2>(v2)) {}

    RBTreeValueKV(const RBTreeValueKV&) = default;
    RBTreeValueKV(RBTreeValueKV&&) = default;

    RBTreeValueKV& assign_value(const RBTreeValueKV& oth) {
        RB_ASSERT(this->first == oth.first);
        this->second = oth.second;
        return *this;
    }
    RBTreeValueKV& assign_value(RBTreeValueKV&& oth) {
        RB_ASSERT(this->first == oth.first);
        this->second = std::move(oth.second);
        return *this;
    }

    RBTreeValueKV& operator=(const RBTreeValueKV&) = delete;
    RBTreeValueKV& operator=(RBTreeValueKV&&) = delete;
    bool operator==(const RBTreeValueKV&) = delete;

    const storage_type_base& get() const {
        return *this;
    }

    storage_type_base& get() {
        return *this;
    }
};
template<typename T>
struct IsRBTreeValueKV: std::false_type {};
template<typename K, typename V>
struct IsRBTreeValueKV<RBTreeValueKV<K,V>>: std::true_type {};


template<typename K>
struct RBTreeValueK {
public:
    const K key;
    using key_type = K;
    using storage_type_base = const K;

    RBTreeValueK() = delete;
#if __cplusplus >= 202002
    template<typename T> requires std::constructible_from<K,T&&>
#else
    template<typename T, typename std::enable_if<std::is_convertible<T,K>::value,bool>::type = true>
#endif // __cplusplus >= 202002
    RBTreeValueK(T&& k): key(std::forward<T>(k)) {}

    template<typename T1, typename T2>
#if __cplusplus >= 202002
        requires std::constructible_from<K, T1&&, T2&&>
#endif // __cplusplus >= 202002
    RBTreeValueK(T1&& k, T2&& v2): key(std::forward<T1>(k), std::forward<T2>(v2)) {}

    template<typename T1, typename T2, typename ... Args>
#if __cplusplus >= 202002
        requires std::constructible_from<K, T1&&, T2&&, Args&&...>
#endif // __cplusplus >= 202002
    RBTreeValueK(T1&& k, T2&& v2, Args ... args): key(std::forward<T1>(k), std::forward<T2>(v2), std::forward<Args>(args)...) {}

    RBTreeValueK(const RBTreeValueK&) = default;
    RBTreeValueK(RBTreeValueK&&) = default;

    RBTreeValueK& assign_value(const RBTreeValueK& oth) {
        RB_ASSERT(this->key == oth.key);
        return *this;
    }
    RBTreeValueK& assign_value(RBTreeValueK&& oth) {
        RB_ASSERT(this->key == oth.key);
        return *this;
    }

    RBTreeValueK& operator=(const RBTreeValueK&) = delete;
    RBTreeValueK& operator=(RBTreeValueK&&) = delete;
    bool operator==(const RBTreeValueK&) = delete;

    storage_type_base& get() const {
        return this->key;
    }
};
template<typename T>
struct IsRBTreeValueK: std::false_type {};
template<typename K>
struct IsRBTreeValueK<RBTreeValueK<K>>: std::true_type {};

#if __cplusplus >= 202002
template <typename T>
concept C_RBTreeValueKV = IsRBTreeValueKV<T>::value;
template <typename T>
concept C_RBTreeValueK = IsRBTreeValueK<T>::value;
template <typename Compare, typename T>
concept C_RBTreeValueCompare = std::predicate<Compare, const typename T::key_type&,const typename T::key_type&>;
#endif // __cplusplus >= 202002


#if __cplusplus >= 202002
template <C_RBTreeValueKV T, C_RBTreeValueCompare<T> Compare>
#else
template<typename T, typename Compare, typename std::enable_if<IsRBTreeValueKV<T>::value,bool>::type = true>
#endif // __cplusplus >= 202002
inline bool rbvalue_compare(Compare cmp, const T& v1, const T& v2) {
    return cmp(v1.first,v2.first);
}

#if __cplusplus >= 202002
template<C_RBTreeValueKV T1, typename T2, C_RBTreeValueCompare<T1> Compare> requires ( !C_RBTreeValueKV<T2> )
#else
template<
    typename T1, typename T2, typename Compare,
    typename std::enable_if<IsRBTreeValueKV<T1>::value && !IsRBTreeValueKV<T2>::value,bool>::type = true>
#endif // __cplusplus >= 202002
inline bool rbvalue_compare(Compare cmp, const T1& v1, const T2& v2) {
    return cmp(v1.first,v2);
}

#if __cplusplus >= 202002
template<typename T1, C_RBTreeValueKV T2, C_RBTreeValueCompare<T2> Compare> requires ( !C_RBTreeValueKV<T1> )
#else
template<
    typename T1, typename T2, typename Compare,
    typename std::enable_if<!IsRBTreeValueKV<T1>::value && IsRBTreeValueKV<T2>::value,bool>::type = true>
#endif // __cplusplus >= 202002
inline bool rbvalue_compare(Compare cmp, const T1& v1, const T2& v2) {
    return cmp(v1,v2.first);
}

#if __cplusplus >= 202002
template<C_RBTreeValueK T, C_RBTreeValueCompare<T> Compare>
#else
template<typename T, typename Compare, typename std::enable_if<IsRBTreeValueK<T>::value,bool>::type = true>
#endif // __cplusplus >= 202002
inline bool rbvalue_compare(Compare cmp, const T& v1, const T& v2) {
    return cmp(v1.key,v2.key);
}

#if __cplusplus >= 202002
template<C_RBTreeValueK T1, typename T2, C_RBTreeValueCompare<T1> Compare> requires ( !C_RBTreeValueK<T2> )
#else
template<
    typename T1, typename T2, typename Compare,
    typename std::enable_if<IsRBTreeValueK<T1>::value && !IsRBTreeValueK<T2>::value,bool>::type = true>
#endif // __cplusplus >= 202002
inline bool rbvalue_compare(Compare cmp, const T1& v1, const T2& v2) {
    return cmp(v1.key,v2);
}

#if __cplusplus >= 202002
template<typename T1, C_RBTreeValueK T2, C_RBTreeValueCompare<T2> Compare> requires ( !C_RBTreeValueK<T1> )
#else
template<
    typename T1, typename T2, typename Compare,
    typename std::enable_if<!IsRBTreeValueK<T1>::value && IsRBTreeValueK<T2>::value,bool>::type = true>
#endif // __cplusplus >= 202002
inline bool rbvalue_compare(Compare cmp, const T1& v1, const T2& v2) {
    return cmp(v1,v2.key);
}

#if __cplusplus >= 202002
template <C_RBTreeValueKV T>
#else
template<typename T, typename std::enable_if<IsRBTreeValueKV<T>::value,bool>::type = true>
#endif // __cplusplus >= 202002
inline bool rbvalue_equal(const T& v1, const T& v2) {
    return v1.first == v2.first;
}

#if __cplusplus >= 202002
template<C_RBTreeValueKV T1, typename T2> requires ( !C_RBTreeValueKV<T2> )
#else
template<
    typename T1, typename T2,
    typename std::enable_if<IsRBTreeValueKV<T1>::value && !IsRBTreeValueKV<T2>::value,bool>::type = true>
#endif // __cplusplus >= 202002
inline bool rbvalue_equal(const T1& v1, const T2& v2) {
    return v1.first == v2;
}

#if __cplusplus >= 202002
template<typename T1, C_RBTreeValueKV T2> requires ( !C_RBTreeValueKV<T1> )
#else
template<
    typename T1, typename T2,
    typename std::enable_if<!IsRBTreeValueKV<T1>::value && IsRBTreeValueKV<T2>::value,bool>::type = true>
#endif // __cplusplus >= 202002
inline bool rbvalue_equal(const T1& v1, const T2& v2) {
    return v1 == v2.first;
}

#if __cplusplus >= 202002
template<C_RBTreeValueK T>
#else
template<typename T, typename std::enable_if<IsRBTreeValueK<T>::value,bool>::type = true>
#endif // __cplusplus >= 202002
inline bool rbvalue_equal(const T& v1, const T& v2) {
    return v1.key == v2.key;
}
#if __cplusplus >= 202002
template<C_RBTreeValueK T1, typename T2> requires ( !C_RBTreeValueK<T2> )
#else
template<
    typename T1, typename T2,
    typename std::enable_if<IsRBTreeValueK<T1>::value && !IsRBTreeValueK<T2>::value,bool>::type = true>
#endif // __cplusplus >= 202002
inline bool rbvalue_equal(const T1& v1, const T2& v2) {
    return v1.key == v2;
}

#if __cplusplus >= 202002
template<typename T1, C_RBTreeValueK T2> requires ( !C_RBTreeValueK<T1> )
#else
template<
    typename T1, typename T2,
    typename std::enable_if<!IsRBTreeValueK<T1>::value && IsRBTreeValueK<T2>::value,bool>::type = true>
#endif // __cplusplus >= 202002
inline bool rbvalue_equal(const T1& v1, const T2& v2) {
    return v1 == v2.key;
}


template<typename S, typename N>
struct RBTreeNodeBasic {
public:
    using storage_type = S;
    using nodeptr_t = N;
    using rbtree_node_type = typename std::remove_pointer<nodeptr_t>::type;
    using size_type = size_t;
    using const_nodeptr_t = const rbtree_node_type*;

public:
    inline nodeptr_t minimum() {
        auto node = static_cast<nodeptr_t>(this);
        for (;node->left;node=node->left);

        return node;
    }

    inline const_nodeptr_t minimum() const {
        return const_cast<RBTreeNodeBasic*>(this)->minimum();
    }

    inline nodeptr_t maximum() {
        auto node = static_cast<nodeptr_t>(this);
        for (;node->right;node=node->right);

        return node;
    }

    inline const_nodeptr_t maximum() const {
        return const_cast<RBTreeNodeBasic*>(this)->maximum();
    }

    nodeptr_t next() {
        auto node = static_cast<nodeptr_t>(this);

        if (node->right) return node->right->minimum();
        for (;node;node=node->parent) {
            if (node->parent && node->parent->left == node) {
                node = node->parent;
                break;
            }
        }

        return node;
    }

    inline const_nodeptr_t next() const {
        return const_cast<RBTreeNodeBasic*>(this)->next();
    }

    nodeptr_t prev() {
        auto node = static_cast<nodeptr_t>(this);

        if (node->left) return node->left->maximum();
        for (;node;node=node->parent) {
            if (node->parent && node->parent->right == node) {
                node = node->parent;
                break;
            }
        }

        return node;
    }

    inline const_nodeptr_t prev() const {
        return const_cast<RBTreeNodeBasic*>(this)->prev();
    }

    nodeptr_t root() {
        auto node = static_cast<nodeptr_t>(this);
        for (;node->parent;node=node->parent);
        return node;
    }

    inline const_nodeptr_t root() const {
        return const_cast<RBTreeNodeBasic*>(this)->root();
    }

public:
    nodeptr_t left, right, parent;
    storage_type value;
    bool black;

    RBTreeNodeBasic(RBTreeNodeBasic&& oth):
        left(oth.left), right(oth.right), parent(oth.parent),
        black(oth.black), value(std::move(oth.value))
    {
        oth.parent = oth.right = oth.left = nullptr;
    }

    RBTreeNodeBasic(const RBTreeNodeBasic& oth):
        left(nullptr), right(nullptr), parent(nullptr),
        black(oth.black), value(oth.value)
    {
    }

    RBTreeNodeBasic& operator=(const RBTreeNodeBasic& oth)
    {
        RB_ASSERT(this->left == nullptr);
        RB_ASSERT(this->right == nullptr);
        this->black = oth.black;
        this->value = oth.value;
    }

    RBTreeNodeBasic& operator=(RBTreeNodeBasic&& oth) {
        if (this->left) {
            delete this->left;
            this->left = oth.left;
        }
        if (this->right) {
            delete this->right;
            this->right = oth.right;
        }
        this->parent = oth.parent;
        oth.parent = nullptr;

        this->value = std::move(oth.value);
        this->black = oth.black;
    }

    size_t num_of_left_children() const {
        RB_ASSERT(false && "never call this function");
        throw std::logic_error("not implement");
    }

    size_t num_of_right_children() const {
        RB_ASSERT(false && "never call this function");
        throw std::logic_error("not implement");
    }

    size_t num_of_nodes() const {
        RB_ASSERT(false && "never call this function");
        throw std::logic_error("not implement");
    }

    void update_position_info(nodeptr_t to) {
    }

    const_nodeptr_t advance(long n) const {
        return const_cast<RBTreeNodeBasic*>(this)->advance(n);
    }

    nodeptr_t advance(long n) {
        auto node = static_cast<nodeptr_t>(this);
        for (;n!=0 && node;) {
            if (n < 0) {
                node = node->prev();
                n++;
            } else {
                node = node->next();
                n--;
            }
        }
        return node;
    }

    size_type indexof() const {
        auto root = this->root();
        auto begin = root->minimum();

        size_t i=0;
        for(;begin && begin!=this;i++, begin=begin->advance(1));

        return i;
    }

    nodeptr_t flatten2List() {
        RB_ASSERT(this->parent == nullptr);
        auto node = this->minimum();
        auto head = node;

        for(;node!=nullptr;) {
            decltype(node) next = nullptr;

            if (node->right) {
                next = node->right->minimum();
            } else {
                for (next=node;next;next=next->parent) {
                    if (next->parent && next->parent->left == next) {
                        next = next->parent;
                        break;
                    }
                }
            }

            node->right = next;
            node = next;
        }

        return head;
    }

    nodeptr_t fromList() {
        auto head = static_cast<nodeptr_t>(this);
        size_t size = 0;
        for (auto node=head;node!=nullptr;node=node->right,size++);

        size_t max_depth = 0;
        for (;(1<<max_depth) < size;max_depth++);
        bool always_black = (1<<max_depth) == size;

        auto node = head;
        const std::function<nodeptr_t(long,long,size_t)> inorderHelper = 
            [&inorderHelper, max_depth, &node, always_black](long start, long end, size_t depth) -> nodeptr_t
        {
            if (start > end) return nullptr;

            int mid = start + (end - start) / 2;
            auto left = inorderHelper(start, mid-1, depth+1);

            auto pn = node;
            node = node->right;

            auto right = inorderHelper(mid+1, end, depth+1);

            if (left) left->parent = pn;
            if (right) right->parent = pn;
            pn->left = left;
            pn->right = right;
            pn->black = always_black || depth != max_depth;
            pn->update_position_info(pn->parent);
            return pn;
        };

        auto root = inorderHelper(0, size - 1, 0);
        root->parent = nullptr;
        return root;
    }

#if __cplusplus >= 202002
    template<typename St> requires (!C_is_same_value_type<St,RBTreeNodeBasic>)
#else
    template<typename St, typename std::enable_if<!is_same_value_type<St,RBTreeNodeBasic>::value, bool>::type = true>
#endif // __cplusplus >= 202002
    RBTreeNodeBasic(St&& val):
        left(nullptr), right(nullptr), parent(nullptr),
        black(false), value(std::forward<St>(val))
    {}

    ~RBTreeNodeBasic() {
        RB_ASSERT(this->left == nullptr);
        RB_ASSERT(this->right == nullptr);
    }
};

template<typename S>
struct RBTreeNode: public RBTreeNodeBasic<S,RBTreeNode<S>*> {
    using base_type = RBTreeNodeBasic<S,RBTreeNode<S>*>;

#if __cplusplus >= 202002
    template<typename St> requires (!C_is_same_value_type<St,RBTreeNode>)
#else
    template<typename St, typename std::enable_if<!is_same_value_type<St,RBTreeNode>::value, bool>::type = true>
#endif // __cplusplus >= 202002
    RBTreeNode(St&& val): base_type(std::forward<St>(val)) {}
};

template<typename S>
struct RBTreeNodePosInfo: public RBTreeNodeBasic<S,RBTreeNodePosInfo<S>*> {
private:
    size_t num_nodes;

public:
    using base_type = RBTreeNodeBasic<S,RBTreeNodePosInfo<S>*>;
    using size_type = typename base_type::size_type;
    using storage_type = typename base_type::storage_type;
    using nodeptr_t = typename base_type::nodeptr_t;
    using const_nodeptr_t = typename base_type::const_nodeptr_t;

    inline size_t num_of_left_children() const {
        return this->left ? this->left->num_of_nodes() : 0;
    }

    inline size_t num_of_right_children() const {
        return this->right ? this->right->num_of_nodes() : 0;
    }

    inline size_t num_of_nodes() const {
        return this->num_nodes;
    }

    void update_position_info(nodeptr_t to) {
        for (auto node=this;node!=to;node=node->parent) {
            size_t n = 1;
            if (node->left) n += node->left->num_nodes;
            if (node->right) n += node->right->num_nodes;

            if (node->num_nodes == n)
                break;
            node->num_nodes = n;
        }
    }

    const_nodeptr_t advance(long n) const {
        return const_cast<RBTreeNodePosInfo*>(this)->advance(n);
    }

    nodeptr_t advance(long n) {
        auto node=this;
        RB_ASSERT(node);
        for (;n!=0 && node!=nullptr;) {
            if (n < 0) {
                const long k = node->num_of_left_children();

                if (k + n >= 0) {
                    node = node->left;
                    long m = node->right == nullptr ? 0 : node->right->num_nodes;
                    n = n + m + 1;
                } else if (auto p = node->parent) {
                    if (p->left == node) {
                        long m = node->right == nullptr ? 0 : node->right->num_nodes;
                        n = n - m - 1;
                    } else {
                        RB_ASSERT(p->right == node);
                        n = n + k + 1;
                    }
                    node = p;
                } else {
                    node = nullptr;
                }
            } else {
                const long k = node->num_of_right_children();

                if (k - n >= 0) {
                    node = node->right;
                    long m = node->left == nullptr ? 0 : node->left->num_nodes;
                    n = n - m - 1;
                } else if (auto p = node->parent) {
                    if (p->right == node) {
                        long m = node->left == nullptr ? 0 : node->left->num_nodes;
                        n = n + m + 1;
                    } else {
                        RB_ASSERT(p->left == node);
                        n = n - k - 1;
                    }
                    node = p;
                } else {
                    node = nullptr;
                }
            }
        }

        return node;
    }

    size_type indexof() const {
        size_t ans = 0;
        auto node = this;
        RB_ASSERT(node);

        for (auto repr_node=node;repr_node!=nullptr;) {
            if (repr_node->left != nullptr) {
                ans += repr_node->left->num_nodes;
            }

            auto parent = repr_node->parent;
            while (parent && parent->left == repr_node) {
                repr_node = parent;
                parent = parent->parent;
            }

            repr_node = parent;
            if (repr_node != nullptr) ans++;
        }

        return ans;
    }

#if __cplusplus >= 202002
    template<typename St> requires (!C_is_same_value_type<St,RBTreeNodePosInfo>)
#else
    template<typename St, typename std::enable_if<!is_same_value_type<St,RBTreeNodePosInfo>::value, bool>::type = true>
#endif // __cplusplus >= 202002
    RBTreeNodePosInfo(St&& val): base_type(std::forward<St>(val)), num_nodes(1) {}
};


template<typename _Key, typename _Value>
using rbtree_storage_type = typename std::conditional<std::is_same<_Value,void>::value,RBTreeValueK<_Key>, RBTreeValueKV<_Key,_Value>>::type;
template<typename _Key, typename _Value>
using rbtree_compare_type = _Key;

template<typename S, bool keep_position_info>
using node_pointer = typename std::conditional<keep_position_info,RBTreeNodePosInfo<S>,RBTreeNode<S>>::type::nodeptr_t;
template<typename S, bool keep_position_info>
using const_node_pointer = typename std::conditional<keep_position_info,RBTreeNodePosInfo<S>,RBTreeNode<S>>::type::const_nodeptr_t;

template<typename _Key>
using default_compare_t = std::less<_Key>;
template<typename _Key, typename _Value>
using default_allocato_t = std::allocator<RBTreeNode<rbtree_storage_type<_Key,_Value>>>;

#if __cplusplus >= 202002
template<typename Compare, typename Key>
concept C_KeyCompare = std::predicate<Compare,Key,Key>;
#endif // __cplusplus >= 202002

template<
    typename _Key, typename _Value, bool multi, bool keep_position_info=true,
#if __cplusplus >= 202002
    C_KeyCompare<_Key> Compare = default_compare_t<_Key>,
#else
    typename Compare = default_compare_t<_Key>,
#endif // __cplusplus >= 202002
    typename Alloc = default_allocato_t<_Key,_Value>>
class RBTreeImpl {
    public:
        using storage_type = rbtree_storage_type<_Key,_Value>;
        using nodeptr_t = node_pointer<storage_type,keep_position_info>;
        using const_nodeptr_t = const_node_pointer<storage_type,keep_position_info>;
        using rbtree_node_type = typename std::remove_pointer<nodeptr_t>::type;
        using key_type = _Key;
        using mapped_type = _Value;
        using value_type = typename storage_type::storage_type_base;
        using size_type = typename rbtree_node_type::size_type;
        using difference_type = std::ptrdiff_t;
        using key_compare = Compare;
        using allocator_type = Alloc;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = typename std::allocator_traits<Alloc>::pointer;
        using const_pointer = typename std::allocator_traits<Alloc>::const_pointer;
        constexpr static bool PositionInformation = keep_position_info;
        using storage_allocator_ = typename std::allocator_traits<Alloc>::template rebind_alloc<rbtree_node_type>;

    private:
        nodeptr_t root;
        size_t _version, _size;
        Compare cmp;
        storage_allocator_ allocator;

        template<typename ... Args>
        inline nodeptr_t construct_node(Args&& ... args) {
            auto ptr = this->allocator.allocate(1);
            return new (ptr) rbtree_node_type(std::forward<Args>(args)...);
        }

        inline void delete_node(nodeptr_t node) {
#if __cplusplus >= 201703
            std::destroy_n(node, 1);
#else
            node->~rbtree_node_type();
#endif // __cplusplus >= 201703
            this->allocator.deallocate(node, 1);
        }

        template<typename T1, typename T2>
        inline bool rb_comp(const T1& a, const T2& b) const
        {
            return rbvalue_compare(this->cmp, a, b);
        }

        template<typename T1, typename T2>
        inline bool rb_equal(const T1& a, const T2& b) const
        {
            return rbvalue_equal(a, b);
        }

        inline void update_num_nodes(nodeptr_t node, nodeptr_t end) const
        {
            if (!keep_position_info) return;

            node->update_position_info(end);
        }

        inline void be_left_child(nodeptr_t parent, nodeptr_t child) const {
            parent->left = child;
            if (child) child->parent = parent;
            this->update_num_nodes(parent, parent->parent);
        }

        inline void be_right_child(nodeptr_t parent, nodeptr_t child) const {
            parent->right = child;
            if (child) child->parent = parent;
            this->update_num_nodes(parent, parent->parent);
        }

        inline nodeptr_t right_rotate(nodeptr_t node) {
            auto node_parent = node->parent;
            bool left = node_parent && node_parent->left == node;

            auto n6 = node;
            auto n4 = n6->left, n7 = n6->right;
            auto n2 = n4->left, n5 = n4->right;
            auto n1 = n2->left, n3 = n2->right;

            this->be_left_child(n2, n1);
            this->be_right_child(n2, n3);
            this->be_left_child(n6, n5);
            this->be_right_child(n6, n7);
            this->be_left_child(n4, n2);
            this->be_right_child(n4, n6);

            n4->parent = node_parent;
            if (node_parent) {
                if (left) {
                    node_parent->left = n4;
                } else {
                    node_parent->right = n4;
                }
            }
            return n4;
        }

        inline nodeptr_t right_left_rotate(nodeptr_t node) {
            auto node_parent = node->parent;
            bool left = node_parent && node_parent->left == node;

            auto n6 = node;
            auto n2 = n6->left, n7 = n6->right;
            auto n1 = n2->left, n4 = n2->right;
            auto n3 = n4->left, n5 = n4->right;

            this->be_left_child(n2, n1);
            this->be_right_child(n2, n3);
            this->be_left_child(n6, n5);
            this->be_right_child(n6, n7);
            this->be_left_child(n4, n2);
            this->be_right_child(n4, n6);

            n4->parent = node_parent;
            if (node_parent) {
                if (left) {
                    node_parent->left = n4;
                } else {
                    node_parent->right = n4;
                }
            }
            return n4;
        }

        inline nodeptr_t left_rotate(nodeptr_t node) {
            auto node_parent = node->parent;
            bool left = node_parent && node_parent->left == node;

            auto n2 = node;
            auto n1 = n2->left, n4 = n2->right;
            auto n3 = n4->left, n6 = n4->right;
            auto n5 = n6->left, n7 = n6->right;

            this->be_left_child(n2, n1);
            this->be_right_child(n2, n3);
            this->be_left_child(n6, n5);
            this->be_right_child(n6, n7);
            this->be_left_child(n4, n2);
            this->be_right_child(n4, n6);

            n4->parent = node_parent;
            if (node_parent) {
                if (left) {
                    node_parent->left = n4;
                } else {
                    node_parent->right = n4;
                }
            }
            return n4;
        }

        inline nodeptr_t left_right_rotate(nodeptr_t node) {
            auto node_parent = node->parent;
            bool left = node_parent && node_parent->left == node;

            auto n2 = node;
            auto n1 = n2->left, n6 = n2->right;
            auto n4 = n6->left, n7 = n6->right;
            auto n3 = n4->left, n5 = n4->right;

            this->be_left_child(n2, n1);
            this->be_right_child(n2, n3);
            this->be_left_child(n6, n5);
            this->be_right_child(n6, n7);
            this->be_left_child(n4, n2);
            this->be_right_child(n4, n6);

            n4->parent = node_parent;
            if (node_parent) {
                if (left) {
                    node_parent->left = n4;
                } else {
                    node_parent->right = n4;
                }
            }
            return n4;
        }

        inline void fix_redred(nodeptr_t node) {
            auto p = node->parent;
            auto pp = p->parent;
            RB_ASSERT(!node->black);
            RB_ASSERT(!p->black);
            RB_ASSERT(pp->black);
            this->update_num_nodes(p, pp->parent);

            for (;!node->black && p && !p->black;) {
                if (pp->left == p && p->left == node) {
                    node = this->right_rotate(pp);
                    node->left->black = true;
                } else if (pp->left == p && p->right == node) {
                    node = this->right_left_rotate(pp);
                    node->left->black = true;
                } else if (pp->right == p && p->right == node) {
                    node = this->left_rotate(pp);
                    node->right->black = true;
                } else if (pp->right == p && p->left == node) {
                    node = this->left_right_rotate(pp);
                    node->right->black = true;
                }

                RB_ASSERT(!node->black);
                if (node->parent) {
                    p = node->parent;
                    pp = p->parent ? p->parent : nullptr;
                } else {
                    node->black = true;
                    this->root = node;
                    p = pp = nullptr;
                    break;
                }
            }

            if (p) this->update_num_nodes(p, nullptr);
        }

        inline bool is_black_node(nodeptr_t node) const {
            return node == nullptr || node->black;
        }

        inline void fix_delete(nodeptr_t extra_parent, nodeptr_t extra_black, bool is_left_child) {
            for (;(extra_black == nullptr || extra_black->black) && extra_black != this->root;) {
                RB_ASSERT(extra_parent);
                
                if (is_left_child) {
                    auto sibling = extra_parent->right;
                    RB_ASSERT(sibling);

                    if (!sibling->black) {
                        RB_ASSERT(extra_parent->black);

                        if (extra_parent->parent == nullptr) {
                            RB_ASSERT(extra_parent == this->root);
                            this->root = sibling;
                        }

                        auto siblingx = this->left_rotate(extra_parent);
                        RB_ASSERT(siblingx == sibling);
                        sibling->black = true;
                        extra_parent->black = false;

                        extra_parent = sibling->left;
                        sibling = extra_parent->right;
                    }

                    RB_ASSERT(sibling->black);
                    if (this->is_black_node(sibling->left) && this->is_black_node(sibling->right)) {
                        sibling->black = false;
                        extra_black = extra_parent;
                        extra_parent = extra_black->parent;
                        is_left_child = extra_parent != nullptr ? extra_parent->left == extra_black : false;
                        if (extra_parent == nullptr) {
                            RB_ASSERT(extra_black == this->root);
                        }
                    } else {
                        if (this->is_black_node(sibling->right)) {
                            RB_ASSERT(!this->is_black_node(sibling->left));
                            auto sl = sibling->left;
                            this->be_left_child(sibling, sl->right);
                            this->be_right_child(sl, sibling);
                            this->be_right_child(extra_parent, sl);
                            sl->black = true;
                            sibling->black = false;
                            sibling = sl;
                        }

                        auto pp = extra_parent->parent;
                        bool pp_left = pp && pp->left == extra_parent;

                        auto extra_parent_color = extra_parent->black;

                        if (pp == nullptr) {
                            RB_ASSERT(extra_parent == this->root);
                            RB_ASSERT(extra_parent->black);
                            this->root = sibling;
                        }

                        RB_ASSERT(!this->is_black_node(sibling->right));
                        auto sl = sibling->left, sr = sibling->right;
                        this->be_right_child(extra_parent, sl);
                        this->be_left_child(sibling, extra_parent);
                        sibling->black = extra_parent_color;
                        extra_parent->black = true;
                        sr->black = true;

                        if (pp) {
                            if (pp_left) {
                                this->be_left_child(pp, sibling);
                            } else {
                                this->be_right_child(pp, sibling);
                            }
                        } else {
                            sibling->parent = pp;
                        }
                        extra_black = this->root;
                        extra_parent = nullptr;
                    }
                } else {
                    auto sibling = extra_parent->left;
                    RB_ASSERT(sibling);

                    if (!sibling->black) {
                        RB_ASSERT(extra_parent->black);

                        if (extra_parent->parent == nullptr) {
                            RB_ASSERT(extra_parent == this->root);
                            this->root = sibling;
                        }

                        auto siblingx = this->right_rotate(extra_parent);
                        RB_ASSERT(siblingx == sibling);
                        sibling->black = true;
                        extra_parent->black = false;

                        extra_parent = sibling->right;
                        sibling = extra_parent->left;
                    }

                    RB_ASSERT(sibling->black);
                    if (this->is_black_node(sibling->right) && this->is_black_node(sibling->left)) {
                        sibling->black = false;
                        extra_black = extra_parent;
                        extra_parent = extra_black->parent;
                        is_left_child = extra_parent != nullptr ? extra_parent->left == extra_black : false;
                        if (extra_parent == nullptr) {
                            RB_ASSERT(extra_black == this->root);
                        }
                    } else {
                        if (this->is_black_node(sibling->left)) {
                            RB_ASSERT(!this->is_black_node(sibling->right));
                            auto sl = sibling->right;
                            this->be_right_child(sibling, sl->left);
                            this->be_left_child(sl, sibling);
                            this->be_left_child(extra_parent, sl);
                            sl->black = true;
                            sibling->black = false;
                            sibling = sl;
                        }

                        auto pp = extra_parent->parent;
                        bool pp_right = pp && pp->right == extra_parent;

                        auto extra_parent_color = extra_parent->black;

                        if (pp == nullptr) {
                            RB_ASSERT(extra_parent == this->root);
                            RB_ASSERT(extra_parent->black);
                            this->root = sibling;
                        }

                        RB_ASSERT(!this->is_black_node(sibling->left));
                        auto sl = sibling->right, sr = sibling->left;
                        this->be_left_child(extra_parent, sl);
                        this->be_right_child(sibling, extra_parent);
                        sibling->black = extra_parent_color;
                        extra_parent->black = true;
                        sr->black = true;

                        if (pp) {
                            if (pp_right) {
                                this->be_right_child(pp, sibling);
                            } else {
                                this->be_left_child(pp, sibling);
                            }
                        } else {
                            sibling->parent = pp;
                        }
                        extra_black = this->root;
                        extra_parent = nullptr;
                    }
                }
            }

            RB_ASSERT(extra_black);
            RB_ASSERT(!extra_black->black || extra_black == this->root);
            extra_black->black = true;
        }

        inline nodeptr_t minimum(nodeptr_t node) const {
            return node->minimum();
        }

        inline nodeptr_t maximum(nodeptr_t node) const {
            return node->maximum();
        }

        inline void swap_node(nodeptr_t n1, nodeptr_t n2) const {
            RB_ASSERT(n1);
            RB_ASSERT(n2);
            std::swap(n1->black, n2->black);

            if (n1->parent == n2)
                std::swap(n1, n2);

            if (n2->parent == n1) {
                auto p = n1->parent;
                const bool is_left_child = n1->left == n2;
                const bool n1_is_left_child = p ? p ->left == n1 : false;
                const auto n1_left = n1->left, n1_right = n1->right;

                this->be_left_child(n1, n2->left);
                this->be_right_child(n1, n2->right);
                if (is_left_child) {
                    this->be_left_child(n2, n1);
                    this->be_right_child(n2, n1_right);
                } else {
                    this->be_right_child(n2, n1);
                    this->be_left_child(n2, n1_left);
                }
                if (p) {
                    if (n1_is_left_child) {
                        this->be_left_child(p, n2);
                    } else {
                        this->be_right_child(p, n2);
                    }
                } else {
                    n2->parent = nullptr;
                }
            } else {
                auto n1_parent = n1->parent;
                bool n1_is_left = n1_parent ? n1_parent->left == n1 : false;
                auto n1_left = n1->left, n1_right = n1->right;

                auto n2_parent = n2->parent;
                bool n2_is_left = n2_parent ? n2_parent->left == n2 : false;
                auto n2_left = n2->left, n2_right = n2->right;

                std::swap(n1->parent, n2->parent);
                this->be_left_child(n2, n1_left);
                this->be_right_child(n2, n1_right);
                this->be_left_child(n1, n2_left);
                this->be_right_child(n1, n2_right);

                if (n1_parent) {
                    if (n1_is_left) {
                        n1_parent->left = n2;
                    } else {
                        n1_parent->right = n2;
                    }
                }
                if (n2_parent) {
                    if (n2_is_left) {
                        n2_parent->left = n1;
                    } else {
                        n2_parent->right = n1;
                    }
                }
            }
        }

    public:
        void touch() {
            this->_version++;
        }

        template<typename Sx>
        std::pair<nodeptr_t,bool> insert(nodeptr_t hint, Sx&& val) {
            return this->emplace(hint, std::forward<Sx>(val));
        }

        template<typename ... Args >
        std::pair<nodeptr_t,bool> emplace(nodeptr_t hint, Args&& ...args)
        {
            this->_version++;
            auto node = this->construct_node(std::forward<Args>(args)...);
            auto result = this->insert_node(hint, node);
            if (std::get<1>(result)) {
                auto rnode = std::get<1>(result);
                RB_ASSERT(rnode == node);
                this->delete_node(rnode);
            }
            return std::make_pair(std::get<0>(result), std::get<2>(result));
        }

        std::tuple<nodeptr_t,nodeptr_t,bool> insert_node(nodeptr_t hint, nodeptr_t node) {
            const auto& val = node->value;
            if (this->root == nullptr) {
                this->root = node;
                this->root->black = true;
                RB_ASSERT(this->_size == 0);
                this->_size++;
                return std::make_tuple(this->root, nullptr, true);
            }

            auto cn = this->root;
            if (hint != nullptr) {
                bool left_is_ok = false, right_is_ok = false;
                auto left_node = hint, right_node = hint;

                for (;(!left_is_ok && left_node) || (!right_is_ok && right_node);) {
                    if (!left_is_ok && left_node) {
                        if (this->rb_comp(left_node->value, val)) {
                            left_is_ok = true;
                        } else {
                            left_node = left_node->left;
                        }
                    }

                    if (!right_is_ok && right_node) {
                        if (this->rb_comp(val, right_node->value)) {
                            right_is_ok = true;
                        } else {
                            right_node = right_node->right;
                        }
                    }
                }

                for (auto tp=hint->parent;(!left_is_ok || !right_is_ok) && tp;tp=tp->parent) {
                    if (!left_is_ok && this->rb_comp(tp->value, val)) {
                        hint = tp;
                        left_is_ok = true;
                    }

                    if (!right_is_ok && this->rb_comp(val, tp->value)) {
                        hint = tp;
                        right_is_ok = true;
                    }
                }

                if (left_is_ok && right_is_ok) {
                    cn = hint;
                }
            }

            for(;;) {
                if (this->rb_comp(node->value, cn->value)) {
                    if (cn->left == nullptr) {
                        cn->left = node;
                        node->parent = cn;
                        break;
                    } else {
                        cn = cn->left;
                    }
                } else if (!multi && this->rb_equal(node->value, cn->value)) {
                    cn->value.assign_value(std::move(node->value));
                    return std::make_tuple(cn, node, false);
                } else {
                    if (cn->right == nullptr) {
                        cn->right = node;
                        node->parent = cn;
                        break;
                    } else {
                        cn = cn->right;
                    }
                }
            }

            this->_size++;
            if (cn->black) {
                this->update_num_nodes(cn, nullptr);
            } else {
                this->fix_redred(node);
            }
            return std::make_tuple(node, nullptr, true);
        }

        template<typename Sx>
        std::pair<nodeptr_t,bool> insert(Sx&& val) {
            return this->insert(nullptr, std::forward<Sx>(val));
        }

#ifdef DEBUG
        void check_consistency() const {
            RB_ASSERT(this->is_black_node(this->root));
            if (!this->root) return;

            std::queue<std::pair<nodeptr_t,size_t>> queue;
            queue.push(std::make_pair(this->root, 0));
            size_t black_depth = 0;

            for (;!queue.empty();queue.pop()) {
                auto front = queue.front();
                auto node = front.first;
                auto bdepth = front.second + (node->black ? 1 : 0);
                black_depth = black_depth > bdepth ? black_depth : bdepth;

                if (keep_position_info) {
                    auto left_n = node->num_of_left_children();
                    auto right_n = node->num_of_right_children();

                    RB_ASSERT(node->num_of_nodes() == left_n + right_n + 1);
                }

                if (!node->black) {
                    RB_ASSERT(this->is_black_node(node->left));
                    RB_ASSERT(this->is_black_node(node->right));
                }

                if (node->left) {
                    RB_ASSERT(node->left->parent == node);
                    RB_ASSERT(this->rb_comp(node->left->value, node->value) || (multi && this->rb_equal(node->left->value, node->value)));
                    queue.push(std::make_pair(node->left, bdepth));
                }
                if (node->right) {
                    RB_ASSERT(node->right->parent == node);
                    RB_ASSERT(!this->rb_comp(node->right->value, node->value) || (multi && this->rb_equal(node->right->value, node->value)));
                    queue.push(std::make_pair(node->right, bdepth));
                }

                if (!node->left && !node->right) {
                    RB_ASSERT(bdepth == black_depth);
                }
            }
        }
#endif // DEBUG

        std::pair<nodeptr_t,nodeptr_t> extract(nodeptr_t node, bool return_next_node)
        {
            RB_ASSERT(node != nullptr);
            nodeptr_t extra_black = nullptr;
            nodeptr_t extra_parent = nullptr;
            bool extra_is_left_child = true;

            nodeptr_t next_node = nullptr;

            if (node->left && node->right) {
                auto successor = this->minimum(node->right);
                RB_ASSERT(successor != nullptr);
                if (return_next_node) {
                    next_node = successor;
                }

                if (node == this->root) {
                    RB_ASSERT(node->parent == nullptr);
                    this->root = successor;
                }

                this->swap_node(successor, node);
            } else if (return_next_node) {
                next_node = this->advance(node, 1);
            }
            const auto node_parent = node->parent;

            bool need_extra_black = node->black && node_parent;
            if (node->left == nullptr) {
                if (node->right != nullptr) {
                    RB_ASSERT(!node->right->black);
                    need_extra_black = false;
                    node->right->black = true;
                }

                if (node_parent) {
                    if (node_parent->left == node) {
                        this->be_left_child(node_parent, node->right);
                    } else {
                        RB_ASSERT(node_parent->right == node);
                        this->be_right_child(node_parent, node->right);
                        extra_is_left_child = false;
                    }
                } else {
                    RB_ASSERT(node == this->root);
                    if (node->right != nullptr) {
                        node->right->parent = nullptr;
                    }
                    this->root = node->right;
                }
                extra_parent = need_extra_black ? node_parent : nullptr;
                extra_black = need_extra_black ? node->right : nullptr;
            } else {
                RB_ASSERT(node->right == nullptr);
                RB_ASSERT(node->left != nullptr);
                RB_ASSERT(!node->left->black);
                RB_ASSERT(node->black);
                need_extra_black = false;
                node->left->black = true;

                if (node_parent) {
                    if (node_parent->left == node) {
                        this->be_left_child(node_parent, node->left);
                    } else {
                        RB_ASSERT(node_parent->right == node);
                        this->be_right_child(node_parent, node->left);
                    }
                } else {
                    RB_ASSERT(node == this->root);
                    node->left->parent = nullptr;
                    this->root = node->left;
                }
            }
            node->left = nullptr;
            node->right = nullptr;
            node->parent = nullptr;
            RB_ASSERT(this->_size > 0);
            this->_size--;

            if (this->root != nullptr) {
                RB_ASSERT(this->root->parent == nullptr);
                if (node_parent && node_parent->parent) {
                    this->update_num_nodes(node_parent->parent, nullptr);
                }

                if (need_extra_black) {
                    RB_ASSERT(extra_parent);
                    this->fix_delete(extra_parent, extra_black, extra_is_left_child);
                }
            }

            return std::make_pair(node, next_node);
        }

        inline nodeptr_t erase(nodeptr_t node, bool return_next_node) {
            auto result = this->extract(node, return_next_node);
            this->_version++;
            this->delete_node(result.first);
            return result.second;
        }

        size_type indexof(nodeptr_t node) const {
            size_type ans = 0;
            if (node == nullptr)
                return this->size();
            return node->indexof();
        }

        size_type indexof(const_nodeptr_t node) const {
            size_type ans = 0;
            if (node == nullptr)
                return this->size();
            return node->indexof();
        }

        template<typename _K>
        nodeptr_t lower_bound(const _K& val) {
            auto root = this->root;
            nodeptr_t ans = nullptr;
            if (!root) return ans;

            for (auto node=root;node!=nullptr;) {
                if (!this->rb_comp(node->value, val)) {
                    if (ans == nullptr || !this->rb_comp(ans->value, node->value) || this->rb_equal(node->value, ans->value)) {
                        ans = node;
                    }

                    node = node->left;
                } else {
                    node = node->right;
                }
            }

            return ans;
        }

        template<typename _K>
        const_nodeptr_t lower_bound(const _K& val) const {
            return const_cast<RBTreeImpl*>(this)->lower_bound(val);
        }

        template<typename _K>
        nodeptr_t upper_bound(const _K& val) {
            auto root = this->root;
            nodeptr_t ans = nullptr;
            if (!root) return ans;

            for (auto node=root;node!=nullptr;) {
                if (this->rb_comp(val, node->value)) {
                    if (ans == nullptr || this->rb_comp(node->value, ans->value) || this->rb_equal(node->value, ans->value)) {
                        ans = node;
                    }

                    node = node->left;
                } else {
                    node = node->right;
                }
            }

            return ans;
        }

        template<typename _K>
        const_nodeptr_t upper_bound(const _K& val) const {
            return const_cast<RBTreeImpl*>(this)->upper_bound(val);
        }

        template<typename _K>
        nodeptr_t find(const _K& val) {
            auto node = this->lower_bound(val);
            return node && this->rb_equal(node->value, val) ? node : nullptr;
        }

        template<typename _K>
        const_nodeptr_t find(const _K& val) const {
            auto node = this->lower_bound(val);
            return node && this->rb_equal(node->value, val) ? node : nullptr;
        }

        template<typename _K>
        size_type count(const _K& val) const {
            auto lb = this->lower_bound(val);
            auto ub = this->upper_bound(val);
            return this->indexof(ub) - this->indexof(lb);
        }

        nodeptr_t begin() {
            if (!this->root) return nullptr;

            return this->minimum(this->root);
        }

        const_nodeptr_t begin() const {
            if (!this->root) return nullptr;

            return this->minimum(this->root);
        }

        nodeptr_t rbegin() {
            if (!this->root) return nullptr;

            return this->maximum(this->root);
        }

        const_nodeptr_t rbegin() const {
            if (!this->root) return nullptr;

            return this->maximum(this->root);
        }

        // TODO prototype
        nodeptr_t advance(nodeptr_t node, long n) const {
            // node == nullptr represent end
            if (node == nullptr) {
                // TODO why this->root (shoud be const qualified) can assign to node
                node = this->root;
                n += node && node->right ? node->right->num_of_nodes() + 1 : 1;
            }

            return node->advance(n);
        }

        inline size_type size() const {
            return this->_size;
        }

        inline size_t version() const {
            return this->_version;
        }

        void copy_to(RBTreeImpl& target) const {
            target.~RBTreeImpl();
            target._version++;
            target._size = this->_size;
            if (!this->root) return;

            // preorder traversing
            nodeptr_t parent_node = nullptr;
            nodeptr_t* pptr = &target.root;
            for (nodeptr_t node=this->root;node!=nullptr;) {
                auto val = node->value;
                nodeptr_t new_node = target.construct_node(*node);
                new_node->parent = parent_node;
                *pptr = new_node;

                if (node->left) {
                    node = node->left;
                    parent_node = new_node;
                    pptr = &parent_node->left;
                    continue;
                }

                if (node->right) {
                    node = node->right;
                    parent_node = new_node;
                    pptr = &parent_node->right;
                    continue;
                }

                pptr = nullptr;
                for (;node && node->parent;) {
                    auto old_node = node;
                    node = node->parent;
                    RB_ASSERT(parent_node);

                    if (node->left == old_node && node->right) {
                        node = node->right;
                        pptr = &parent_node->right;
                        break;
                    } else {
                        parent_node = parent_node->parent;
                    }
                }
                RB_ASSERT(pptr != nullptr || node == this->root);
                if (node == this->root) {
                    break;
                }
            }
        }

        Compare cmp_object() const {
            return this->cmp;
        }

        Alloc get_allocator() const {
            return this->allocator;
        }

        void clear() {
            if (!this->root) return;

            for(auto node=this->root;node!=nullptr;) {
                if (node->left) {
                    node = node->left;
                } else if (node->right) {
                    node = node->right;
                } else {
                    auto deadnode = node;
                    node = node->parent;
                    if (node) {
                        if (node->left == deadnode) {
                            node->left = nullptr;
                        } else {
                            RB_ASSERT(node->right == deadnode);
                            node->right = nullptr;
                        }
                    }
                    this->delete_node(deadnode);
                }
            }
            this->root = nullptr;
            this->_version++;
            this->_size = 0;
        }

        void convert2BST() {
            if (this->root == nullptr) return;

            auto head = this->root->flatten2List();
            this->root = head->fromList();
        }

        void construct_from_nodelist(nodeptr_t head) {
            this->clear();
            if (head == nullptr) return;
            size_type size = 0;
            for (auto h=head;h!=nullptr;h=h->right,size++) {}
            this->root = head->fromList();
            this->_size = size;
        }

#if __cplusplus >= 202002
        template<std::forward_iterator Iter>
#else
        template<typename Iter>
#endif // __cplusplus >= 202002
        bool construct_from_asc_iter(Iter begin, Iter end) {
            nodeptr_t head = nullptr, node = nullptr;
            bool failure = false;
            for (;begin!=end;begin++) {
                auto n = this->construct_node(*begin);
                if (head == nullptr) {
                    head = n;
                    node = n;
                } else {
                    if (!(rbvalue_compare(this->cmp, node->value, n->value) || (multi && rbvalue_equal(node->value,n->value)))) {
                        failure = true;
                        break;
                    }

                    node->right = n;
                    node = n;
                }
            }

            if (failure) {
                for (auto n=head;n!=nullptr;n=n->right) {
                    this->delete_node(n);
                }
            } else {
                this->construct_from_nodelist(head);
            }
            return !failure;
        }

        RBTreeImpl(): root(nullptr), _version(0), _size(0) {
        }
        RBTreeImpl(const Compare& cmp, const Alloc& alloc): root(nullptr), _version(0), _size(0), cmp(cmp), allocator(alloc) {
        }
        RBTreeImpl(const Alloc& alloc): root(nullptr), _version(0), _size(0), allocator(alloc) {
        }

        ~RBTreeImpl() {
            this->clear();
        }
};


template<typename T>
struct IsRBTreeImpl : std::false_type {};
template<typename T1, typename T2, bool V1, bool V2, typename T4, typename T5>
struct IsRBTreeImpl<RBTreeImpl<T1,T2,V1,V2,T4,T5>> : std::true_type {};
#if __cplusplus >= 202002
template<typename T>
concept C_RBTreeImpl = IsRBTreeImpl<T>::value;
#endif // __cplusplus >= 202002


template<bool reverse, bool const_iterator, typename RBTreeType>
struct DummyIterator {
    using rbtree_t = RBTreeType;
    using nodeptr_t = typename rbtree_t::nodeptr_t;

    DummyIterator(std::weak_ptr<rbtree_t> tree, nodeptr_t node, size_t version) {}
};

#if __cplusplus >= 202002
template<bool reverse, bool const_iterator, C_RBTreeImpl RBTreeType>
#else
template<bool reverse, bool const_iterator, typename RBTreeType, typename std::enable_if<IsRBTreeImpl<RBTreeType>::value,bool>::type = true>
#endif // __cplusplus >= 202002
class RBTreeImplIterator {
    public:
        using rbtree_t = RBTreeType;
        using storage_type = typename rbtree_t::storage_type;
        using nodeptr_t = typename rbtree_t::nodeptr_t;
        using const_nodeptr_t = typename rbtree_t::const_nodeptr_t;
        using const_iterator_alt_t = typename std::conditional<const_iterator,DummyIterator<reverse,true,RBTreeType>,RBTreeImplIterator<reverse,true,RBTreeType>>::type;

        using iterator_category = typename std::conditional<RBTreeType::PositionInformation, std::random_access_iterator_tag, std::bidirectional_iterator_tag>::type;
        using value_type = typename storage_type::storage_type_base;
        using difference_type = long;
        using pointer = value_type*;
        using reference = typename std::conditional<const_iterator, const value_type&, value_type&>::type;
        using const_reference = const value_type&;

    private:
        std::weak_ptr<rbtree_t> tree;
        nodeptr_t node;
        size_t version;

        std::shared_ptr<rbtree_t> check_version() const {
            auto tree = this->tree.lock();
            if (!tree) {
                throw std::logic_error("access invalid iterator");
            }

            if (tree->version() != this->version) {
            }

            return tree;
        }

    protected:
        template<
            typename _Key, typename _Value, bool multi, bool keep_position_info,
#if __cplusplus >= 202002
            C_KeyCompare<_Key> Compare,
#else
            typename Compare,
#endif // __cplusplus >= 202002
            typename Alloc>
        friend class generic_container;

        void sync_version() {
            auto tree = this->tree.lock();
            if (!tree) {
                throw std::logic_error("access invalid iterator");
            }

            this->version = tree->version();
        }

    public:
        const_nodeptr_t nodeptr() const { return this->node; }
        nodeptr_t nodeptr() { return this->node; }
        ptrdiff_t treeid() const { return reinterpret_cast<std::ptrdiff_t>(this->tree.lock().get()); }

        size_t indexof() const {
            auto tree = this->check_version();
            return tree->indexof(this->node);
        }

        explicit operator bool() const {
            this->check_version();
            return this->node != nullptr;
        }

        operator const_iterator_alt_t() const {
            return const_iterator_alt_t(this->tree, this->node, this->version);
        }

        const pointer operator->() const {
            this->check_version();
            if (this->node == nullptr) {
                throw std::out_of_range("dereference end of a container");
            }
            return &node->value.get();
        }

        pointer operator->() {
            this->check_version();
            if (this->node == nullptr) {
                throw std::out_of_range("dereference end of a container");
            }
            return &node->value.get();
        }

        const_reference operator*() const {
            this->check_version();
            if (this->node == nullptr) {
                throw std::out_of_range("dereference end of a container");
            }
            return node->value.get();
        }

        reference operator*() {
            this->check_version();
            if (this->node == nullptr) {
                throw std::out_of_range("dereference end of a container");
            }
            return node->value.get();
        }

        const_reference operator[](difference_type n) const {
            auto val = this->operator+(n);
            return *val;
        }

        reference operator[](difference_type n) {
            auto val = this->operator+(n);
            return *val;
        }

        RBTreeImplIterator& operator++() {
            auto tree = this->check_version();
            if (this->node == nullptr) {
                throw std::out_of_range("increment end iterator");
            }

            this->node = tree->advance(this->node, reverse ? -1 : 1);
            return *this;
        }

        RBTreeImplIterator operator++(int) {
            auto ans = *this;
            this->operator++();
            return ans;
        }

        RBTreeImplIterator& operator--() {
            auto tree = this->check_version();
            if (reverse && this->node == nullptr && tree->size() > 0) {
                this->node = tree->begin();
                return *this;
            }

            auto dec = tree->advance(this->node, reverse ? 1 : -1);
            if (dec == nullptr) {
                throw std::out_of_range("decrement begin iterator");
            } else {
                this->node = dec;
            }
            return *this;
        }

        RBTreeImplIterator operator--(int) {
            auto ans = *this;
            this->operator--();
            return ans;
        }

        RBTreeImplIterator& operator+=(int n) {
            auto tree = this->check_version();
            if (n == 0) return *this;

            if (reverse && this->node == nullptr && n < 0 && tree->size() > 0) {
                this->node = tree->begin();
                n += 1;
            }

            auto m = tree->advance(this->node, reverse ? -n : n);
            if (m == nullptr) {
                long idx = tree->indexof(this->node);
                if (0 > idx + n || idx + n > tree->size()) {
                    throw std::out_of_range(
                            "out of range by adding '" + std::to_string(n) + 
                            "', current_idx: " + std::to_string(idx) + 
                            ", size: " + std::to_string(tree->size()));
                }
            }
            this->node = m;
            return *this;
        }

        RBTreeImplIterator& operator-=(difference_type n) {
            return this->operator+=(-n);
        }

        RBTreeImplIterator operator+(difference_type n) const {
            auto ans = *this;
            return ans.operator+=(n);
        }

        RBTreeImplIterator operator-(difference_type n) const {
            auto ans = *this;
            return ans.operator-=(n);
        }

        difference_type operator-(RBTreeImplIterator iter) const {
            difference_type idx1 = this->indexof(), idx2 = iter.indexof();

            if (reverse) {
                auto tree = this->check_version();
                auto size = tree->size();
                idx1 = idx1 != size ? size - 1 - idx1 : idx1;
                idx2 = idx2 != size ? size - 1 - idx2 : idx2;
            }

            return idx1 - idx2;
        }

        bool operator==(const RBTreeImplIterator& oth) const {
            auto t1 = this->check_version(), t2 = oth.check_version();
            return t1 == t2 && oth.node == this->node;
        }

        bool operator!=(const RBTreeImplIterator& oth) const {
            return !this->operator==(oth);
        }

        bool operator<(const RBTreeImplIterator& oth) const {
            auto t1 = this->check_version(), t2 = oth.check_version();
            if (t1 != t2) {
                throw std::logic_error("it's invalid to compare iterators from different container");
            }
            const auto idx1 = t1->indexof(this->node), idx2 = t1->indexof(oth.node);
            return reverse ? idx1 > idx2 :  idx1 < idx2;
        }

        bool operator>(const RBTreeImplIterator& oth) const {
            auto t1 = this->check_version(), t2 = oth.check_version();
            if (t1 != t2) {
                throw std::logic_error("it's invalid to compare iterators from different container");
            }
            const auto idx1 = t1->indexof(this->node), idx2 = t1->indexof(oth.node);
            return reverse ? idx1 < idx2 :  idx1 > idx2;
        }

        inline bool operator<=(const RBTreeImplIterator& oth) const {
            return oth.operator>(*this);
        }

        inline bool operator>=(const RBTreeImplIterator& oth) const {
            return oth.operator<(*this);
        }

        RBTreeImplIterator(std::weak_ptr<rbtree_t> tree, nodeptr_t node, size_t version): tree(tree), node(node), version(version) {}
};


#if __cplusplus >= 202002
template<bool reverse, bool const_iterator, C_RBTreeImpl RBTreeType>
#else
template<bool reverse, bool const_iterator, typename RBTreeType, typename std::enable_if<IsRBTreeImpl<RBTreeType>::value,bool>::type = true>
#endif // __cplusplus >= 202002
RBTreeImplIterator<reverse,const_iterator,RBTreeType> 
operator+(
        typename  std::iterator_traits<RBTreeImplIterator<reverse,const_iterator,RBTreeType>>::difference_type n,
        RBTreeImplIterator<reverse,const_iterator,RBTreeType> iter)
{
    return iter + n;
}


template<
    typename _Key, typename _Value, bool multi, bool keep_position_info,
#if __cplusplus >= 202002
    C_KeyCompare<_Key> Compare = default_compare_t<_Key>,
#else
    typename Compare = default_compare_t<_Key>,
#endif // __cplusplus >= 202002
    typename Alloc = default_allocato_t<_Key,_Value>>
class generic_container {
    protected:
        using rbtree_t = RBTreeImpl<_Key,_Value,multi,keep_position_info,Compare,Alloc>;
        std::shared_ptr<rbtree_t> rbtree;

    public:
        using rbtree_storage_type = typename rbtree_t::storage_type;
        using rbtree_storage_type_base = typename rbtree_t::storage_type::storage_type_base;

        using key_type               = typename rbtree_t::key_type;
        using mapped_type            = typename rbtree_t::mapped_type;
        using value_type             = typename rbtree_t::value_type;
        using size_type              = typename rbtree_t::size_type;
        using difference_type        = typename rbtree_t::difference_type;
        using key_compare            = typename rbtree_t::key_compare;
        using allocator_type         = typename rbtree_t::allocator_type;
        using reference              = typename rbtree_t::reference;
        using const_reference        = typename rbtree_t::const_reference;
        using pointer                = typename rbtree_t::pointer;
        using const_pointer          = typename rbtree_t::const_pointer;
        using iterator               = RBTreeImplIterator<false,false,rbtree_t>;
        using const_iterator         = RBTreeImplIterator<false,true,rbtree_t>;
        using reverse_iterator       = RBTreeImplIterator<true,false,rbtree_t>;
        using reverse_const_iterator = RBTreeImplIterator<true,true,rbtree_t>;

    private:
        class node_type_generic {
        public:
            using key_type       = typename generic_container::key_type;
            using value_type     = typename generic_container::value_type;
            using mapped_type    = typename generic_container::mapped_type;
            using allocator_type = typename generic_container::allocator_type;

        private:
            using storage_allocator_ = typename std::allocator_traits<allocator_type>::template rebind_alloc<typename rbtree_t::rbtree_node_type>;
            storage_allocator_ _allocator;

        protected:
            using nodeptr_t        = typename rbtree_t::nodeptr_t;
            using rbtree_node_type = typename rbtree_t::rbtree_node_type;
            nodeptr_t _node;
            friend class generic_container;

            inline nodeptr_t get() {
                if (this->_node == nullptr) {
                    throw std::logic_error("get empty node");
                }

                auto ans = this->_node;
                this->_node = nullptr;
                return ans;
            }

            inline void restore(nodeptr_t node) {
                if (this->_node != nullptr) {
                    throw std::logic_error("restore to an non-empty node");
                }
                this->_node = node;
            }

            node_type_generic(nodeptr_t node, const storage_allocator_& alloc): _node(node), _allocator(alloc) {
            }

            inline const rbtree_node_type& get_node() const {
                if (this->_node == nullptr) {
                    throw std::logic_error("access empty node");
                }
                return *this->_node;
            }

        public:
            constexpr node_type_generic(): _node(nullptr) {}
            node_type_generic(const node_type_generic& oth) = delete;
            node_type_generic(node_type_generic&& oth): _node(oth._node), _allocator(oth._allocator)
            {
                oth._node = nullptr;
            }

            node_type_generic& operator=(const node_type_generic& oth) = delete;
            node_type_generic& operator=(node_type_generic&& oth) {
                this->~node_type_generic();
                this->_node = oth._node;
                oth._node = nullptr;
                return *this;
            }

#if __cplusplus >= 202002
            [[ nodiscard ]] bool empty() const noexcept
#else
            bool empty() const noexcept
#endif // __cplusplus >= 202002
            {
                return this->_node == nullptr;
            }

            explicit operator bool() const noexcept {
                return this->_node != nullptr;
            }

            allocator_type get_allocator() const {
                return this->_allocator;
            }

            void swap(node_type_generic& oth) noexcept(
                    std::allocator_traits<allocator_type>::propagate_on_container_swap::value ||
                    std::allocator_traits<allocator_type>::is_always_equal::value)
            {
                const bool propagate_on_container_swap = 
                    std::allocator_traits<allocator_type>::propagate_on_container_swap;

                if (!(this->_node == nullptr || oth._node == nullptr || 
                      propagate_on_container_swap || this->_allocator == oth._allocator))
                {
                    throw std::logic_error("undefined behavior");
                }

                if (propagate_on_container_swap || this->_allocator == oth._allocator)
                {
                    std::swap(this->_node, oth._node);
                    std::swap(this->_allocator, oth._allocator);
                } else {
                    nodeptr_t pp1 = nullptr, pp2 = nullptr;
                    if (this->_node != nullptr) {
                        auto ptr1 = this->_allocator.allocate(1);
                        pp1 = new (ptr1) rbtree_node_type(*oth._node);
                    }

                    if (oth._node != nullptr) {
                        auto ptr2 = oth._allocator.allocate(1);
                        pp2 = new (ptr2) rbtree_node_type(*this->_node);
                    }

                    if (this->_node) this->~node_type_generic();
                    if (oth._node)   oth.~node_type_generic();

                    this->_node = pp1;
                    oth._node = pp2;
                }
            }

            ~node_type_generic() {
                if (this->_node != nullptr) {
#if __cplusplus >= 201703
                    std::destroy_n(this->_node, 1);
#else
                    _node->~rbtree_node_type();
#endif // __cplusplus >= 201703
                    this->_allocator.deallocate(this->_node, 1);
                    this->_node = nullptr;
                }
            }
        };
        class node_type_set: public node_type_generic {
        public:
            using key_type       = typename node_type_generic::key_type;
            using value_type     = typename node_type_generic::value_type;
            using allocator_type = typename node_type_generic::allocator_type;

            template <typename ... Args>
            node_type_set(Args&&... args): node_type_generic(std::forward<Args>(args)...) { }

            value_type& value() const {
                return const_cast<value_type&>(this->get_node().get());
            }
        };
        class node_type_map: public node_type_generic {
        public:
            using key_type       = typename node_type_generic::key_type;
            using mapped_type    = typename node_type_generic::mapped_type;
            using allocator_type = typename node_type_generic::allocator_type;

            template <typename ... Args>
            node_type_map(Args&&... args): node_type_generic(std::forward<Args>(args)...) { }

            key_type& key() const {
                return const_cast<key_type&>(this->get_node().get().first);
            }

            mapped_type& mapped() const {
                return const_cast<mapped_type&>(this->get_node().get().second);
            }
        };

    public:
        using node_type              = typename std::conditional<std::is_same<_Value,void>::value,node_type_set,node_type_map>::type;
        using insert_return_type     = struct insert_return_type {
            iterator position;
            bool inserted;
            node_type node;

            insert_return_type(iterator pos, bool inserted, node_type&& nh): position(pos), inserted(inserted), node(std::move(nh)) {}
        };

        generic_container(): rbtree(std::make_shared<rbtree_t>()) {}
        explicit generic_container(const Compare& cmp, const Alloc& alloc = Alloc()): rbtree(std::make_shared<rbtree_t>(cmp, alloc)) {
        }
        explicit generic_container(const Alloc& alloc): rbtree(std::make_shared<rbtree_t>(alloc)) {
        }

        generic_container(const generic_container& oth): rbtree(std::make_shared<rbtree_t>(oth.rbtree->cmp_object(), oth.rbtree->get_allocator()))
        {
            oth.rbtree->copy_to(*this->rbtree);
        }
        generic_container(const generic_container& oth, const Alloc& alloc): rbtree(std::make_shared<rbtree_t>(oth.rbtree->cmp_object(), alloc))
        {
            oth.rbtree->copy_to(*this->rbtree);
        }

        generic_container(generic_container&& oth): rbtree(std::make_shared<rbtree_t>(oth.rbtree->cmp_object(), oth.rbtree->get_allocator()))
        {
            std::swap(oth.rbtree, this->rbtree);
        }
        generic_container(generic_container&& oth, const Alloc& alloc): rbtree(std::make_shared<rbtree_t>(oth.rbtree->cmp_object(), alloc))
        {
            std::swap(oth.rbtree, this->rbtree);
        }

#if __cplusplus >= 202002
        template<std::forward_iterator InputIt>
            requires std::convertible_to<typename std::iterator_traits<InputIt>::value_type,rbtree_storage_type_base>
#else
        template<
            typename InputIt, 
            typename std::enable_if<
                std::is_convertible<typename std::iterator_traits<InputIt>::value_type,rbtree_storage_type_base>::value &&
                std::is_convertible<typename std::iterator_traits<InputIt>::iterator_category,std::forward_iterator_tag>::value,
                bool>::type = true>
#endif // __cplusplus >= 202002
        generic_container(InputIt begin, InputIt end, const Compare& cmp = Compare(), const Alloc& alloc = Alloc()):
            rbtree(std::make_shared<rbtree_t>(cmp, alloc))
        {
            this->insert(begin, end);
        }

#if __cplusplus >= 202002
        template<std::forward_iterator InputIt>
            requires std::convertible_to<typename std::iterator_traits<InputIt>::value_type,rbtree_storage_type_base>
#else
        template<
            typename InputIt, 
            typename std::enable_if<
                std::is_convertible<typename std::iterator_traits<InputIt>::value_type,rbtree_storage_type_base>::value &&
                std::is_convertible<typename std::iterator_traits<InputIt>::iterator_category,std::forward_iterator_tag>::value,
                bool>::type = true>
#endif // __cplusplus >= 202002
        generic_container(InputIt begin, InputIt end, const Alloc& alloc):
            rbtree(std::make_shared<rbtree_t>(alloc))
        {
            this->insert(begin, end);
        }

#if __cplusplus >= 202002
        template<typename T> requires std::convertible_to<T,rbtree_storage_type_base>
#else
        template<typename T, typename std::enable_if<std::is_convertible<T,rbtree_storage_type_base>::value, bool>::type = true>
#endif // __cplusplus >= 202002
        generic_container(std::initializer_list<T> init, const Compare& cmp = {}, const Alloc& alloc = {}): rbtree(std::make_shared<rbtree_t>(cmp, alloc)) {
            this->insert(init);
        }

#if __cplusplus >= 202002
        template<typename T> requires std::convertible_to<T,rbtree_storage_type_base>
#else
        template<typename T, typename std::enable_if<std::is_convertible<T,rbtree_storage_type_base>::value, bool>::type = true>
#endif // __cplusplus >= 202002
        generic_container(std::initializer_list<T> init, const Alloc& alloc): rbtree(std::make_shared<rbtree_t>(alloc)) {
            this->insert(init);
        }

        generic_container& operator=(const generic_container& oth) {
            oth.rbtree->copy_to(*this->rbtree);
            return *this;
        }
        generic_container& operator=(generic_container&& oth) {
            this->rbtree->clear();
            std::swap(this->rbtree, oth.rbtree);
            this->rbtree->touch();
            oth.rbtree->touch();
            return *this;
        }

        Alloc get_allocator() const noexcept {
            return this->rbtree->get_allocator();
        }
        Compare key_comp() const {
            return this->rbtree->cmp_object();
        }
        Compare value_comp() const {
            return this->rbtree->cmp_object();
        }

        inline iterator begin() { return iterator(this->rbtree, this->rbtree->begin(), this->rbtree->version()); }
        inline iterator end() { return iterator(this->rbtree, nullptr, this->rbtree->version()); }

        inline const_iterator begin() const { return const_iterator(this->rbtree, this->rbtree->begin(), this->rbtree->version()); }
        inline const_iterator end() const { return const_iterator(this->rbtree, nullptr, this->rbtree->version()); }

        inline const_iterator cbegin() const { return const_iterator(this->rbtree, this->rbtree->begin(), this->rbtree->version()); }
        inline const_iterator cend() const { return const_iterator(this->rbtree, nullptr, this->rbtree->version()); }

        inline reverse_iterator rbegin() { return reverse_iterator(this->rbtree, this->rbtree->rbegin(), this->rbtree->version()); }
        inline reverse_iterator rend() { return reverse_iterator(this->rbtree, nullptr, this->rbtree->version()); }

        inline reverse_const_iterator rbegin() const { return reverse_const_iterator(this->rbtree, this->rbtree->rbegin(), this->rbtree->version()); }
        inline reverse_const_iterator rend() const { return reverse_const_iterator(this->rbtree, nullptr, this->rbtree->version()); }

        inline reverse_const_iterator crbegin() const { return reverse_const_iterator(this->rbtree, this->rbtree->rbegin(), this->rbtree->version()); }
        inline reverse_const_iterator crend() const { return reverse_const_iterator(this->rbtree, nullptr, this->rbtree->version()); }

        template<typename _K>
        iterator lower_bound(const _K& key) {
            auto lb = this->rbtree->lower_bound(key);
            return iterator(this->rbtree, lb, this->rbtree->version());
        }

        template<typename _K>
        const_iterator lower_bound(const _K& key) const {
            auto lb = this->rbtree->lower_bound(key);
            return const_iterator(this->rbtree, lb, this->rbtree->version());
        }

        template<typename _K>
        iterator upper_bound(const _K& key) {
            auto ub = this->rbtree->upper_bound(key);
            return iterator(this->rbtree, ub, this->rbtree->version());
        }

        template<typename _K>
        const_iterator upper_bound(const _K& key) const {
            auto ub = this->rbtree->upper_bound(key);
            return const_iterator(this->rbtree, ub, this->rbtree->version());
        }

        template<typename _K>
        iterator find(const _K& key) {
            auto node = this->rbtree->find(key);
            return iterator(this->rbtree, node, this->rbtree->version());
        }

        template<typename _K>
        const_iterator find(const _K& key) const {
            auto node = this->rbtree->find(key);
            return const_iterator(this->rbtree, node, this->rbtree->version());
        }

        template<typename _K>
        std::pair<iterator,iterator> equal_range(const _K& key) {
            return std::make_pair(this->lower_bound(key), this->upper_bound(key));
        }

        template<typename _K>
        std::pair<const_iterator,const_iterator> equal_range(const _K& key) const {
            return std::make_pair(this->lower_bound(key), this->upper_bound(key));
        }

        template<typename _K>
        size_t count(const _K& key) const {
            return this->rbtree->count(key);
        }

        template<typename _K>
        bool contains(const _K& key) const {
            return this->find(key) != this->end();
        }

        inline size_t size() const { return this->rbtree->size(); }
        inline bool empty() const { return this->size() == 0; }
        inline size_t max_size() const noexcept { return std::numeric_limits<size_t>::max(); }

        template<typename ValType>
        std::pair<iterator,bool> insert(ValType&& val)
        {
            auto result = this->rbtree->insert(std::forward<ValType>(val));
            return make_pair(iterator(this->rbtree, result.first, this->rbtree->version()), result.second);
        }

        template<typename ValType>
        iterator insert(iterator hint, ValType&& val)
        {
            return this->emplace_hint(hint, std::forward<ValType>(val));
        }

        template<typename ValType>
        iterator insert(const_iterator hint, ValType&& val)
        {
            return this->emplace_hint(hint, std::forward<ValType>(val));
        }

#if __cplusplus >= 202002
        template<std::forward_iterator InputIt>
            requires (std::convertible_to<typename std::iterator_traits<InputIt>::value_type,rbtree_storage_type_base>)
#else
        template<
            typename InputIt, 
            typename std::enable_if<
                std::is_convertible<typename std::iterator_traits<InputIt>::value_type,rbtree_storage_type_base>::value &&
                std::is_convertible<typename std::iterator_traits<InputIt>::iterator_category,std::forward_iterator_tag>::value,
                bool>::type = true>
#endif // __cplusplus >= 202002
        void insert(InputIt first, InputIt last) {
            for(;first != last;first++) this->insert(*first);
        }

#if __cplusplus >= 202002
        template<typename T> requires std::convertible_to<T, rbtree_storage_type>
#else
        template<typename T, typename std::enable_if<std::is_convertible<T,rbtree_storage_type_base>::value, bool>::type = true>
#endif // __cplusplus >= 202002
        inline void insert(std::initializer_list<T> list) {
            this->insert(list.begin(), list.end());
        }

        insert_return_type insert(node_type&& nh) {
            if (!nh) {
                return insert_return_type(this->end(), false, std::move(nh));
            }

            if (nh.get_allocator() != this->rbtree->get_allocator()) {
                throw std::logic_error("allocator of node doesn't equal with allocator of container");
            } else {
                auto result = this->rbtree->insert_node(nullptr, nh.get());
                nh.restore(std::get<1>(result));
                iterator iter(this->rbtree, std::get<0>(result), this->rbtree->version());
                return insert_return_type(iter, std::get<2>(result), std::move(nh));
            }
        }

        iterator insert(const_iterator hint, node_type&& nh) {
            if (!nh) return this->end();

            if (nh.get_allocator() != this->rbtree->get_allocator()) {
                throw std::logic_error("allocator of node doesn't equal with allocator of container");
            } else {
                auto result = this->rbtree->insert_node(hint.nodeptr(), nh.get());
                nh.restore(std::get<1>(result));
                return iterator(this->rbtree, std::get<0>(result), this->rbtree->version());
            }
        }

        iterator insert(iterator hint, node_type&& nh) {
            return this->insert(const_iterator(hint), std::move(nh));
        }

        template< class... Args >
        inline std::pair<iterator,bool> emplace( Args&&... args ){
            auto result = this->rbtree->emplace(nullptr, std::forward<Args>(args)...);
            return std::make_pair(iterator(this->rbtree, result.first, this->rbtree->version()), result.second);
        }

        template< class... Args >
        iterator emplace_hint(const_iterator hint,  Args&&... args ){
            if (!hint.treeid()) {
                throw std::logic_error("hint is an invalid iterator");
            }

            auto result = this->rbtree->emplace(hint.nodeptr(), std::forward<Args>(args)...);
            return iterator(this->rbtree, result.first, this->rbtree->version());
        }

        iterator erase(iterator pos) {
            if (!pos.treeid()) {
                throw std::logic_error("erase an invalid iterator");
            }

            auto  node= pos.nodeptr();
            if (node == nullptr) {
                throw std::logic_error("erase end iterator");
            }
            auto next_ptr = this->rbtree->erase(node, true);
            return iterator(this->rbtree, next_ptr, this->rbtree->version());
        }

        iterator erase(const_iterator pos) {
            if (!pos.treeid()) {
                throw std::logic_error("erase an invalid iterator");
            }

            auto node = pos.nodeptr();
            if (node == nullptr) {
                throw std::logic_error("erase end iterator");
            }
            auto next_ptr = this->rbtree->erase(node, true);
            return iterator(this->rbtree, next_ptr, this->rbtree->version());
        }

        iterator erase(iterator first, iterator last) {
            return this->erase(const_iterator(first), const_iterator(last));
        }

        iterator erase(const_iterator first, const_iterator last) {
            if (first > last) {
                throw std::logic_error("invalid range");
            }

            iterator ans = this->end();
            for (;first!=this->cend() && first!=last;) {
                ans = this->erase(first);
                first = ans;
                last.sync_version();
            }
            return ans;
        }

        size_t erase(const _Key& key) {
            auto first = this->lower_bound(key);
            auto last = this->upper_bound(key);
            auto ans = std::distance(first, last);
            this->erase(first, last);
            return ans;
        }

        node_type extract(const_iterator position) {
            auto node = position.nodeptr();
            if (node == nullptr) {
                throw std::logic_error("extract end iterator");
            }

            auto extracted = this->rbtree->extract(node, false).first;
            return node_type_generic(extracted, this->rbtree->get_allocator());
        }

        node_type extract(const _Key& key) {
            auto pos = this->find(key);
            if (pos == this->end()) {
                return node_type_generic();
            } else {
                return extract(key);
            }
        }

        template <typename C2, bool m>
        inline void merge(generic_container<_Key,_Value,m,keep_position_info,C2,Alloc>&& source) {
            this->merge(source);
        }

#if __cplusplus >= 202002
        template<std::forward_iterator Iter>
#else
        template<typename Iter>
#endif // __cplusplus >= 202002
        inline bool emplace_asc(Iter begin, Iter end) {
            return this->rbtree->construct_from_asc_iter(begin, end);
        }

        template <typename C2, bool m>
        void merge(generic_container<_Key,_Value,m,keep_position_info,C2,Alloc>& source) {
            if (this->get_allocator() != source.get_allocator()) {
                throw std::logic_error("allocators don't equal");
            }

            decltype(((node_type*)nullptr)->get()) head = nullptr;
            auto node = head;
            size_t n_not_inserted = 0;
            for (;!source.empty();) {
                auto ext_node = source.extract(source.begin());
                auto result = this->insert(std::move(ext_node));
                if (!result.inserted) {
                    n_not_inserted++;
                    auto n = result.node.get();
                    if (node) {
                        node->right = n;
                        node = n;
                    } else {
                        head = n;
                        node = n;
                    }
                    node->right = nullptr;
                }
            }

            if (head != nullptr) {
                source.rbtree->construct_from_nodelist(head);
            }
        }

        void swap(generic_container& oth) noexcept {
            std::swap(this->rbtree, oth.rbtree);
        }

        void clear() {
            this->rbtree->clear();
        }
};


template<typename _Key, typename _Value, bool multi, bool keep_position_info, typename Compare, typename Alloc>
bool operator==(const generic_container<_Key,_Value,multi,keep_position_info,Compare,Alloc>& lhs,
                const generic_container<_Key,_Value,multi,keep_position_info,Compare,Alloc>& rhs)
{
    if (lhs.size() != rhs.size()) return false;

    for (auto lhs_begin=lhs.begin(),rhs_begin=rhs.begin();lhs_begin!=lhs.end();lhs_begin++,rhs_begin++) {
        if (*lhs_begin != *rhs_begin) {
            return false;
        }
    }

    return true;
}

template<typename _Key, typename _Value, bool multi, bool keep_position_info, typename Compare, typename Alloc>
bool operator!=(const generic_container<_Key,_Value,multi,keep_position_info,Compare,Alloc>& lhs,
                const generic_container<_Key,_Value,multi,keep_position_info,Compare,Alloc>& rhs)
{
    return !operator==(lhs, rhs);
}


template<
    typename _Key, typename _Value, bool multi, bool keep_position_info,
#if __cplusplus >= 202002
    C_KeyCompare<_Key> Compare = default_compare_t<_Key>,
#else
    typename Compare = default_compare_t<_Key>,
#endif // __cplusplus >= 202002
    typename Alloc = default_allocato_t<_Key,_Value>>
class generic_map: public generic_container<_Key,_Value,multi,keep_position_info,Compare,Alloc> {
    private:
        using base_t = generic_container<_Key,_Value,multi,keep_position_info,Compare,Alloc>;

    public:
        static_assert(!std::is_same<_Value,void>::value, "_Value must not be void");
        using rbtree_storage_type = typename base_t::rbtree_storage_type;
        using rbtree_storage_type_base = typename base_t::rbtree_storage_type_base;

        using key_type               = typename base_t::key_type;
        using mapped_type            = typename base_t::mapped_type;
        using value_type             = typename base_t::value_type;
        using size_type              = typename base_t::size_type;
        using difference_type        = typename base_t::difference_type;
        using key_compare            = typename base_t::key_compare;
        using allocator_type         = typename base_t::allocator_type;
        using reference              = typename base_t::reference;
        using const_reference        = typename base_t::const_reference;
        using pointer                = typename base_t::pointer;
        using const_pointer          = typename base_t::const_pointer;
        using iterator               = typename base_t::iterator;
        using const_iterator         = typename base_t::const_iterator;
        using reverse_iterator       = typename base_t::reverse_const_iterator;
        using reverse_const_iterator = typename base_t::reverse_const_iterator;
        using node_type              = typename base_t::node_type;
        using insert_return_type     = typename base_t::insert_return_type;

        generic_map() = default;
        template<typename ...Args>
        generic_map(Args&& ...args): base_t(std::forward<Args>(args)...) {}

        generic_map(std::initializer_list<rbtree_storage_type_base> init, const Compare& cmp = {}, const Alloc& alloc = {}): base_t(std::move(init), cmp, alloc) {
        }

        generic_map(std::initializer_list<rbtree_storage_type_base> init, const Alloc& alloc): base_t(std::move(init), alloc) {
        }

        using base_t::insert;
        inline void insert(std::initializer_list<rbtree_storage_type_base> list) {
            this->insert(list.begin(), list.end());
        }
};


template<
    typename _Key, bool multi, bool keep_position_info,
#if __cplusplus >= 202002
    C_KeyCompare<_Key> Compare = default_compare_t<_Key>,
#else
    typename Compare = default_compare_t<_Key>,
#endif // __cplusplus >= 202002
    typename Alloc = default_allocato_t<_Key,void>>
class generic_set: public generic_container<_Key,void,multi,keep_position_info,Compare,Alloc> {
    private:
        using base_t = generic_container<_Key,void,multi,keep_position_info,Compare,Alloc>;

    public:
        using rbtree_storage_type = typename base_t::rbtree_storage_type;
        using rbtree_storage_type_base= typename base_t::rbtree_storage_type_base;

        using key_type               = typename base_t::key_type;
        using value_type             = typename base_t::value_type;
        using size_type              = typename base_t::size_type;
        using difference_type        = typename base_t::difference_type;
        using key_compare            = typename base_t::key_compare;
        using allocator_type         = typename base_t::allocator_type;
        using reference              = typename base_t::reference;
        using const_reference        = typename base_t::const_reference;
        using pointer                = typename base_t::pointer;
        using const_pointer          = typename base_t::const_pointer;
        using iterator               = typename base_t::iterator;
        using const_iterator         = typename base_t::const_iterator;
        using reverse_iterator       = typename base_t::reverse_const_iterator;
        using reverse_const_iterator = typename base_t::reverse_const_iterator;
        using node_type              = typename base_t::node_type;
        using insert_return_type     = typename base_t::insert_return_type;

        generic_set() = default;
        template<typename ...Args>
        generic_set(Args&& ...args): base_t(std::forward<Args>(args)...) {}

        generic_set(std::initializer_list<rbtree_storage_type_base> init, const Compare& cmp = {}, const Alloc& alloc = {}): base_t(std::move(init), cmp, alloc) {
        }

        generic_set(std::initializer_list<rbtree_storage_type_base> init, const Alloc& alloc): base_t(std::move(init), alloc) {
        }
};


template<
    typename _Key, typename _Value, bool keep_position_info,
#if __cplusplus >= 202002
    C_KeyCompare<_Key> Compare = default_compare_t<_Key>,
#else
    typename Compare = default_compare_t<_Key>,
#endif // __cplusplus >= 202002
    typename Alloc = default_allocato_t<_Key,_Value>>
class generic_unimap: public generic_map<_Key,_Value,false,keep_position_info,Compare,Alloc> {
    private:
        using base_t = generic_map<_Key,_Value,false,keep_position_info,Compare,Alloc>;

    public:
        using rbtree_storage_type = typename base_t::rbtree_storage_type;
        using rbtree_storage_type_base = typename base_t::rbtree_storage_type_base;

        using key_type               = typename base_t::key_type;
        using mapped_type            = typename base_t::mapped_type;
        using value_type             = typename base_t::value_type;
        using size_type              = typename base_t::size_type;
        using difference_type        = typename base_t::difference_type;
        using key_compare            = typename base_t::key_compare;
        using allocator_type         = typename base_t::allocator_type;
        using reference              = typename base_t::reference;
        using const_reference        = typename base_t::const_reference;
        using pointer                = typename base_t::pointer;
        using const_pointer          = typename base_t::const_pointer;
        using iterator               = typename base_t::iterator;
        using const_iterator         = typename base_t::const_iterator;
        using reverse_iterator       = typename base_t::reverse_const_iterator;
        using reverse_const_iterator = typename base_t::reverse_const_iterator;
        using node_type              = typename base_t::node_type;
        using insert_return_type     = typename base_t::insert_return_type;

        generic_unimap() = default;
        template<typename ...Args>
        generic_unimap(Args&& ...args): base_t(std::forward<Args>(args)...) {}

        generic_unimap(std::initializer_list<rbtree_storage_type_base> init, const Compare& cmp = {}, const Alloc& alloc = {}): base_t(std::move(init), cmp, alloc) {
        }

        generic_unimap(std::initializer_list<rbtree_storage_type_base> init, const Alloc& alloc): base_t(std::move(init), alloc) {
        }

        _Value& at(const _Key& key) {
            auto at = this->find(key);
            if (at == this->end()) {
                throw std::out_of_range("out of range");
            }
            return at->second;
        }

        const _Value& at(const _Key& key) const {
            return const_cast<generic_unimap*>(this)->at(key);
        }

        template<typename K>
        _Value& operator[](K&& key) {
            auto at = this->find(key);
            if (at == this->end()) {
                auto ins_result = this->insert(rbtree_storage_type(std::forward<K>(key), _Value()));
                RB_ASSERT(ins_result.second);
                return ins_result.first->second;
            } else {
                return at->second;
            }
        }
};


template<
    typename _Key,
#if __cplusplus >= 202002
    C_KeyCompare<_Key> Compare = default_compare_t<_Key>,
#else
    typename Compare = default_compare_t<_Key>,
#endif // __cplusplus >= 202002
    typename Alloc = default_allocato_t<_Key,void>>
using set2 = generic_set<_Key,false,false,Compare,Alloc>;

template<
    typename _Key,
#if __cplusplus >= 202002
    C_KeyCompare<_Key> Compare = default_compare_t<_Key>,
#else
    typename Compare = default_compare_t<_Key>,
#endif // __cplusplus >= 202002
    typename Alloc = default_allocato_t<_Key,void>>
using pset = generic_set<_Key,false,true,Compare,Alloc>;

template<
    typename _Key,
#if __cplusplus >= 202002
    C_KeyCompare<_Key> Compare = default_compare_t<_Key>,
#else
    typename Compare = default_compare_t<_Key>,
#endif // __cplusplus >= 202002
    typename Alloc = default_allocato_t<_Key,void>>
using multiset2 = generic_set<_Key,true,false,Compare,Alloc>;

template<
    typename _Key,
#if __cplusplus >= 202002
    C_KeyCompare<_Key> Compare = default_compare_t<_Key>,
#else
    typename Compare = default_compare_t<_Key>,
#endif // __cplusplus >= 202002
    typename Alloc = default_allocato_t<_Key,void>>
using pmultiset = generic_set<_Key,true,true,Compare,Alloc>;


template<
    typename _Key, typename _Value,
#if __cplusplus >= 202002
    C_KeyCompare<_Key> Compare = default_compare_t<_Key>,
#else
    typename Compare = default_compare_t<_Key>,
#endif // __cplusplus >= 202002
    typename Alloc = default_allocato_t<_Key,_Value>>
using multimap2 = generic_map<_Key,_Value,true,false,Compare,Alloc>;

template<
    typename _Key, typename _Value,
#if __cplusplus >= 202002
    C_KeyCompare<_Key> Compare = default_compare_t<_Key>,
#else
    typename Compare = default_compare_t<_Key>,
#endif // __cplusplus >= 202002
    typename Alloc = default_allocato_t<_Key,_Value>>
using pmultimap = generic_map<_Key,_Value,true,true,Compare,Alloc>;

template<
    typename _Key, typename _Value,
#if __cplusplus >= 202002
    C_KeyCompare<_Key> Compare = default_compare_t<_Key>,
#else
    typename Compare = default_compare_t<_Key>,
#endif // __cplusplus >= 202002
    typename Alloc = default_allocato_t<_Key,_Value>>
using map2 = generic_unimap<_Key,_Value,false,Compare,Alloc>;

template<
    typename _Key, typename _Value,
#if __cplusplus >= 202002
    C_KeyCompare<_Key> Compare = default_compare_t<_Key>,
#else
    typename Compare = default_compare_t<_Key>,
#endif // __cplusplus >= 202002
    typename Alloc = default_allocato_t<_Key,_Value>>
using pmap = generic_unimap<_Key,_Value,true,Compare,Alloc>;


#if __cplusplus >= 201703
namespace pmr {
    template <class Key, class Compare = std::less<Key>>
    using pset = pset<Key, Compare, std::pmr::polymorphic_allocator<Key>>;

    template <class Key, class Compare = std::less<Key>>
    using pmultiset = pmultiset<Key, Compare, std::pmr::polymorphic_allocator<Key>>;

    template <class Key, class Value, class Compare = std::less<Key>>
    using pmap = pmap<Key, Value, Compare, ::std::pmr::polymorphic_allocator<Key>>;

    template <class Key, class Value, class Compare = std::less<Key>>
    using pmultimap = pmultimap<Key, Value, Compare, ::std::pmr::polymorphic_allocator<Key>>;
}
#endif // __cplusplus >= 201703
}
