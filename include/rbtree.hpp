#pragma once
#include <utility>
#include <iterator>
#include <type_traits>
#include <memory>
#include <stdexcept>
#include <limits>

// TODO just for development
#define DEBUG 1
#define THROW(msg) throw std::logic_error(msg)
#include <iostream>

#ifdef DEBUG
#include <assert.h>
#include <queue>
#define RB_ASSERT(x) assert(x)
#else
#define RB_ASSERT(x)
#endif


template<typename T1, typename T2>
struct is_same_value_type:
    std::is_same<
        std::remove_reference_t<std::remove_const_t<std::remove_reference_t<T1>>>,
        std::remove_reference_t<std::remove_const_t<std::remove_reference_t<T2>>>
    > {};

template<typename K, typename V>
struct RBTreeValueKV: public std::pair<const K,V> {
public:
    using storage_type = std::pair<const K,V>;

    RBTreeValueKV() = delete;

    template<typename T, std::enable_if_t<!std::is_convertible<T,RBTreeValueKV>::value,bool> = true>
    RBTreeValueKV(T&& v): std::pair<const K,V>(std::forward<T>(v)) {}

    template<typename T1, typename T2>
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

    const storage_type& get() const {
        return *this;
    }

    storage_type& get() {
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
    using storage_type = const K;

    RBTreeValueK() = delete;
    template<typename T, std::enable_if_t<std::is_convertible<T,K>::value,bool> = true>
    RBTreeValueK(T&& k): key(std::forward<T>(k)) {}

    template<typename T1, typename T2>
    RBTreeValueK(T1&& k, T2&& v2): key(std::forward<T1>(k), std::forward<T2>(v2)) {}

    template<typename T1, typename T2, typename ... Args>
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

    storage_type& get() const {
        return this->key;
    }
};
template<typename T>
struct IsRBTreeValueK: std::false_type {};
template<typename K>
struct IsRBTreeValueK<RBTreeValueK<K>>: std::true_type {};


template<typename T, typename Compare, typename std::enable_if<IsRBTreeValueKV<T>::value,bool>::type = true>
inline bool rbvalue_compare(Compare cmp, const T& v1, const T& v2) {
    return cmp(v1.first,v2.first);
}
template<
    typename T1, typename T2, typename Compare,
    typename std::enable_if<IsRBTreeValueKV<T1>::value && !IsRBTreeValueKV<T2>::value,bool>::type = true>
inline bool rbvalue_compare(Compare cmp, const T1& v1, const T2& v2) {
    return cmp(v1.first,v2);
}
template<
    typename T1, typename T2, typename Compare,
    typename std::enable_if<!IsRBTreeValueKV<T1>::value && IsRBTreeValueKV<T2>::value,bool>::type = true>
inline bool rbvalue_compare(Compare cmp, const T1& v1, const T2& v2) {
    return cmp(v1,v2.first);
}
template<typename T, typename Compare, typename std::enable_if<IsRBTreeValueK<T>::value,bool>::type = true>
inline bool rbvalue_compare(Compare cmp, const T& v1, const T& v2) {
    return cmp(v1.key,v2.key);
}
template<
    typename T1, typename T2, typename Compare,
    typename std::enable_if<IsRBTreeValueK<T1>::value && !IsRBTreeValueK<T2>::value,bool>::type = true>
inline bool rbvalue_compare(Compare cmp, const T1& v1, const T2& v2) {
    return cmp(v1.key,v2);
}
template<
    typename T1, typename T2, typename Compare,
    typename std::enable_if<!IsRBTreeValueK<T1>::value && IsRBTreeValueK<T2>::value,bool>::type = true>
inline bool rbvalue_compare(Compare cmp, const T1& v1, const T2& v2) {
    return cmp(v1,v2.key);
}
template<typename T, typename std::enable_if<IsRBTreeValueKV<T>::value,bool>::type = true>
inline bool rbvalue_equal(const T& v1, const T& v2) {
    return v1.first == v2.first;
}
template<
    typename T1, typename T2,
    typename std::enable_if<IsRBTreeValueKV<T1>::value && !IsRBTreeValueKV<T2>::value,bool>::type = true>
inline bool rbvalue_equal(const T1& v1, const T2& v2) {
    return v1.first == v2;
}
template<
    typename T1, typename T2,
    typename std::enable_if<!IsRBTreeValueKV<T1>::value && IsRBTreeValueKV<T2>::value,bool>::type = true>
inline bool rbvalue_equal(const T1& v1, const T2& v2) {
    return v1 == v2.first;
}
template<typename T, typename std::enable_if<IsRBTreeValueK<T>::value,bool>::type = true>
inline bool rbvalue_equal(const T& v1, const T& v2) {
    return v1.key == v2.key;
}
template<
    typename T1, typename T2,
    typename std::enable_if<IsRBTreeValueK<T1>::value && !IsRBTreeValueK<T2>::value,bool>::type = true>
inline bool rbvalue_equal(const T1& v1, const T2& v2) {
    return v1.key == v2;
}
template<
    typename T1, typename T2,
    typename std::enable_if<!IsRBTreeValueK<T1>::value && IsRBTreeValueK<T2>::value,bool>::type = true>
inline bool rbvalue_equal(const T1& v1, const T2& v2) {
    return v1 == v2.key;
}


template<typename S, typename N>
struct RBTreeNodeBasic {
public:
    using storage_type = S;
    using nodeptr_t = N;
    using node_type = typename std::remove_pointer<nodeptr_t>::type;
    using const_nodeptr_t = const node_type*;

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
        RB_ASSERT(false && "never call this function");
        throw std::logic_error("not implement");
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

    size_t indexof() const {
        auto root = this->root();
        auto begin = root->minimum();

        size_t i=0;
        for(;begin && begin!=this;i++, begin=begin->advance(1));

        return i;
    }

    template<typename St, std::enable_if_t<!is_same_value_type<St,RBTreeNodeBasic>::value, bool> = true>
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

    template<typename St, std::enable_if_t<!is_same_value_type<St,RBTreeNode>::value, bool> = true>
    RBTreeNode(St&& val): base_type(std::forward<St>(val)) {}
};

template<typename S>
struct RBTreeNodePosInfo: public RBTreeNodeBasic<S,RBTreeNodePosInfo<S>*> {
private:
    size_t num_nodes;

public:
    using base_type = RBTreeNodeBasic<S,RBTreeNodePosInfo<S>*>;
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

    size_t indexof() const {
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

    template<typename St, std::enable_if_t<!is_same_value_type<St,RBTreeNodePosInfo>::value, bool> = true>
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

template<
    typename _Key, typename _Value, bool multi, bool keep_position_info=true,
    typename Compare = default_compare_t<_Key>,
    typename Alloc = default_allocato_t<_Key,_Value>>
class RBTreeImpl {
    public:
        using storage_type = rbtree_storage_type<_Key,_Value>;
        using nodeptr_t = node_pointer<storage_type,keep_position_info>;
        using const_nodeptr_t = const_node_pointer<storage_type,keep_position_info>;
        using node_type = typename std::remove_pointer<nodeptr_t>::type;
        constexpr static bool PositionInformation = keep_position_info;
#if __cplusplus >= 202002L
        using storage_allocator_ = std::allocator_traits<Alloc>::rebind_alloc<node_type>;
#else
        using storage_allocator_ = typename Alloc::template rebind<node_type>::other;
#endif // __cplusplus >= 202002L

    private:
        nodeptr_t root;
        size_t _version, _size;
        Compare cmp;
        storage_allocator_ allocator;

        template<typename ... Args>
        inline nodeptr_t construct_node(Args&& ... args) {
            auto ptr = this->allocator.allocate(1);
            return new (ptr) node_type(std::forward<Args>(args)...);
        }

        inline void delete_node(nodeptr_t node) {
#if __cplusplus >= 201703L
            std::destroy_n(node, 1);
            this->allocator.deallocate(node, 1);
#else
            node->~node_type();
            this->allocator.deallocate(node, 1);
#endif // __cplusplus >= 201703L
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
            const auto& val = node->value;
            if (this->root == nullptr) {
                this->root = node;
                this->root->black = true;
                RB_ASSERT(this->_size == 0);
                this->_size++;
                return std::make_pair(this->root, true);
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
                    this->delete_node(node);
                    return std::make_pair(cn, false);
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
            return std::make_pair(node, true);
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

        nodeptr_t erase(nodeptr_t node, bool return_next_node)
        {
            RB_ASSERT(node != nullptr);
            nodeptr_t extra_black = nullptr;
            nodeptr_t extra_parent = nullptr;
            bool extra_is_left_child = true;
            this->_version++;

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
            this->delete_node(node);
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

            return next_node;
        }

        size_t indexof(nodeptr_t node) const {
            size_t ans = 0;
            if (node == nullptr)
                return this->size();
            return node->indexof();
        }

        size_t indexof(const_nodeptr_t node) const {
            size_t ans = 0;
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
        size_t count(const _K& val) const {
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

        inline size_t size() const {
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

        Alloc allocator_object() const {
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

// TODO just for making intellisense work in development
template class RBTreeImpl<int,void,true,true>;
template class RBTreeImpl<int,void,true,false>;

template<typename T>
struct IsRBTreeImpl : std::false_type {};
template<typename T1, typename T2, bool V1, bool V2, typename T4, typename T5>
struct IsRBTreeImpl<RBTreeImpl<T1,T2,V1,V2,T4,T5>> : std::true_type {};


template<bool reverse, bool const_iterator, typename RBTreeType>
struct DummyIterator {
    using rbtree_t = RBTreeType;
    using nodeptr_t = typename rbtree_t::nodeptr_t;

    DummyIterator(std::weak_ptr<rbtree_t> tree, nodeptr_t node, size_t version) {}
};

template<bool reverse, bool const_iterator, typename RBTreeType, typename std::enable_if<IsRBTreeImpl<RBTreeType>::value,bool>::type = true>
class RBTreeImplIterator {
    public:
        using rbtree_t = RBTreeType;
        using storage_type = typename rbtree_t::storage_type;
        using nodeptr_t = typename rbtree_t::nodeptr_t;
        using const_nodeptr_t = typename rbtree_t::const_nodeptr_t;
        using const_iterator_alt_t = typename std::conditional<const_iterator,DummyIterator<reverse,true,RBTreeType>,RBTreeImplIterator<reverse,true,RBTreeType>>::type;

        using iterator_category = typename std::conditional<RBTreeType::PositionInformation, std::random_access_iterator_tag, std::bidirectional_iterator_tag>::type;
        using value_type = typename storage_type::storage_type;
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
            typename _Key, typename _Value, bool multi, bool kepp_position_info,
            typename Compare,
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


template<bool reverse, bool const_iterator, typename RBTreeType, typename std::enable_if<IsRBTreeImpl<RBTreeType>::value,bool>::type = true>
RBTreeImplIterator<reverse,const_iterator,RBTreeType> 
operator+(
        typename  std::iterator_traits<RBTreeImplIterator<reverse,const_iterator,RBTreeType>>::difference_type n,
        RBTreeImplIterator<reverse,const_iterator,RBTreeType> iter)
{
    return iter + n;
}


template<
    typename _Key, typename _Value, bool multi, bool kepp_position_info,
    typename Compare = default_compare_t<_Key>,
    typename Alloc = default_allocato_t<_Key,_Value>>
class generic_container {
    protected:
        using rbtree_t = RBTreeImpl<_Key,_Value,multi,kepp_position_info,Compare,Alloc>;
        std::shared_ptr<rbtree_t> rbtree;

    public:
        using rbtree_storage_type = typename rbtree_t::storage_type;
        using rbtree_storage_type_base = typename rbtree_t::storage_type::storage_type;
        using iterator_t = RBTreeImplIterator<false,false,rbtree_t>;
        using const_iterator_t = RBTreeImplIterator<false,true,rbtree_t>;
        using reverse_iterator_t = RBTreeImplIterator<true,false,rbtree_t>;
        using reverse_const_iterator_t = RBTreeImplIterator<true,true,rbtree_t>;

        generic_container(): rbtree(std::make_shared<rbtree_t>()) {}
        explicit generic_container(const Compare& cmp, const Alloc& alloc = Alloc()): rbtree(std::make_shared<rbtree_t>(cmp, alloc)) {
        }
        explicit generic_container(const Alloc& alloc): rbtree(std::make_shared<rbtree_t>(alloc)) {
        }

        generic_container(const generic_container& oth): rbtree(std::make_shared<rbtree_t>(oth.rbtree->cmp_object(), oth.rbtree->allocator_object()))
        {
            oth.rbtree->copy_to(*this->rbtree);
        }
        generic_container(const generic_container& oth, const Alloc& alloc): rbtree(std::make_shared<rbtree_t>(oth.rbtree->cmp_object(), alloc))
        {
            oth.rbtree->copy_to(*this->rbtree);
        }

        generic_container(generic_container&& oth): rbtree(std::make_shared<rbtree_t>(oth.rbtree->cmp_object(), oth.rbtree->allocator_object()))
        {
            std::swap(oth.rbtree, this->rbtree);
        }
        generic_container(generic_container&& oth, const Alloc& alloc): rbtree(std::make_shared<rbtree_t>(oth.rbtree->cmp_object(), alloc))
        {
            std::swap(oth.rbtree, this->rbtree);
        }

        template<
            typename InputIt, 
            typename std::enable_if<
                std::is_convertible<typename std::iterator_traits<InputIt>::value_type,rbtree_storage_type_base>::value &&
                std::is_convertible<typename std::iterator_traits<InputIt>::iterator_category,std::input_iterator_tag>::value,
                bool>::type = true>
        generic_container(InputIt begin, InputIt end, const Compare& cmp = Compare(), const Alloc& alloc = Alloc()):
            rbtree(std::make_shared<rbtree_t>(cmp, alloc))
        {
            this->insert(begin, end);
        }

        template<
            typename InputIt, 
            typename std::enable_if<
                std::is_convertible<typename std::iterator_traits<InputIt>::value_type,rbtree_storage_type_base>::value &&
                std::is_convertible<typename std::iterator_traits<InputIt>::iterator_category,std::input_iterator_tag>::value,
                bool>::type = true>
        generic_container(InputIt begin, InputIt end, const Alloc& alloc):
            rbtree(std::make_shared<rbtree_t>(alloc))
        {
            this->insert(begin, end);
        }

        template<typename T, typename std::enable_if<std::is_convertible<T,rbtree_storage_type_base>::value, bool>::type = true>
        generic_container(std::initializer_list<T> init, const Compare& cmp = {}, const Alloc& alloc = {}): rbtree(std::make_shared<rbtree_t>(cmp, alloc)) {
            this->insert(init);
        }
        template<typename T, typename std::enable_if<std::is_convertible<T,rbtree_storage_type_base>::value, bool>::type = true>
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
            return this->rbtree->allocator_object();
        }
        Compare key_comp() const {
            return this->rbtree->cmp_object();
        }
        Compare value_comp() const {
            return this->rbtree->cmp_object();
        }

        inline iterator_t begin() { return iterator_t(this->rbtree, this->rbtree->begin(), this->rbtree->version()); }
        inline iterator_t end() { return iterator_t(this->rbtree, nullptr, this->rbtree->version()); }

        inline const_iterator_t begin() const { return const_iterator_t(this->rbtree, this->rbtree->begin(), this->rbtree->version()); }
        inline const_iterator_t end() const { return const_iterator_t(this->rbtree, nullptr, this->rbtree->version()); }

        inline const_iterator_t cbegin() const { return const_iterator_t(this->rbtree, this->rbtree->begin(), this->rbtree->version()); }
        inline const_iterator_t cend() const { return const_iterator_t(this->rbtree, nullptr, this->rbtree->version()); }

        inline reverse_iterator_t rbegin() { return reverse_iterator_t(this->rbtree, this->rbtree->rbegin(), this->rbtree->version()); }
        inline reverse_iterator_t rend() { return reverse_iterator_t(this->rbtree, nullptr, this->rbtree->version()); }

        inline reverse_const_iterator_t rbegin() const { return reverse_const_iterator_t(this->rbtree, this->rbtree->rbegin(), this->rbtree->version()); }
        inline reverse_const_iterator_t rend() const { return reverse_const_iterator_t(this->rbtree, nullptr, this->rbtree->version()); }

        inline reverse_const_iterator_t crbegin() const { return reverse_const_iterator_t(this->rbtree, this->rbtree->rbegin(), this->rbtree->version()); }
        inline reverse_const_iterator_t crend() const { return reverse_const_iterator_t(this->rbtree, nullptr, this->rbtree->version()); }

        template<typename _K>
        iterator_t lower_bound(const _K& key) {
            auto lb = this->rbtree->lower_bound(key);
            return iterator_t(this->rbtree, lb, this->rbtree->version());
        }

        template<typename _K>
        const_iterator_t lower_bound(const _K& key) const {
            auto lb = this->rbtree->lower_bound(key);
            return const_iterator_t(this->rbtree, lb, this->rbtree->version());
        }

        template<typename _K>
        iterator_t upper_bound(const _K& key) {
            auto ub = this->rbtree->upper_bound(key);
            return iterator_t(this->rbtree, ub, this->rbtree->version());
        }

        template<typename _K>
        const_iterator_t upper_bound(const _K& key) const {
            auto ub = this->rbtree->upper_bound(key);
            return const_iterator_t(this->rbtree, ub, this->rbtree->version());
        }

        template<typename _K>
        iterator_t find(const _K& key) {
            auto node = this->rbtree->find(key);
            return iterator_t(this->rbtree, node, this->rbtree->version());
        }

        template<typename _K>
        const_iterator_t find(const _K& key) const {
            auto node = this->rbtree->find(key);
            return const_iterator_t(this->rbtree, node, this->rbtree->version());
        }

        template<typename _K>
        std::pair<iterator_t,iterator_t> equal_range(const _K& key) {
            return std::make_pair(this->lower_bound(key), this->upper_bound(key));
        }

        template<typename _K>
        std::pair<const_iterator_t,const_iterator_t> equal_range(const _K& key) const {
            return std::make_pair(this->lower_bound(key), this->upper_bound(key));
        }

        template<typename _K>
        size_t count(const _K& key) const {
            return this->rbtree->count(key);
        }

        inline size_t size() const { return this->rbtree->size(); }
        inline bool empty() const { return this->size() == 0; }
        inline size_t max_size() const noexcept { return std::numeric_limits<size_t>::max(); }

        template<typename ValType>
        std::pair<iterator_t,bool> insert(ValType&& val)
        {
            auto result = this->rbtree->insert(std::forward<ValType>(val));
            return make_pair(iterator_t(this->rbtree, result.first, this->rbtree->version()), result.second);
        }

        template<typename ValType>
        iterator_t insert(iterator_t hint, ValType&& val)
        {
            return this->emplace_hint(hint, std::forward<ValType>(val));
        }

        template<typename ValType>
        iterator_t insert(const_iterator_t hint, ValType&& val)
        {
            return this->emplace_hint(hint, std::forward<ValType>(val));
        }

        template<
            typename InputIt, 
            typename std::enable_if<
                std::is_convertible<typename std::iterator_traits<InputIt>::value_type,rbtree_storage_type_base>::value &&
                std::is_convertible<typename std::iterator_traits<InputIt>::iterator_category,std::input_iterator_tag>::value,
                bool>::type = true>
        void insert(InputIt first, InputIt last) {
            for(;first != last;first++) this->insert(*first);
        }

        template<typename T, typename std::enable_if<std::is_convertible<T,rbtree_storage_type_base>::value, bool>::type = true>
        inline void insert(std::initializer_list<T> list) {
            this->insert(list.begin(), list.end());
        }

        template< class... Args >
        inline std::pair<iterator_t,bool> emplace( Args&&... args ){
            auto result = this->rbtree->emplace(nullptr, std::forward<Args>(args)...);
            return std::make_pair(iterator_t(this->rbtree, result.first, this->rbtree->version()), result.second);
        }

        template< class... Args >
        iterator_t emplace_hint(const_iterator_t hint,  Args&&... args ){
            if (!hint.treeid()) {
                throw std::logic_error("hint is an invalid iterator");
            }

            auto result = this->rbtree->emplace(hint.nodeptr(), std::forward<Args>(args)...);
            return iterator_t(this->rbtree, result.first, this->rbtree->version());
        }

        iterator_t erase(iterator_t pos) {
            if (!pos.treeid()) {
                throw std::logic_error("erase an invalid iterator");
            }

            auto  node= pos.nodeptr();
            if (node == nullptr) {
                throw std::logic_error("erase end iterator");
            }
            auto next_ptr = this->rbtree->erase(node, true);
            return iterator_t(this->rbtree, next_ptr, this->rbtree->version());
        }

        iterator_t erase(const_iterator_t pos) {
            if (!pos.treeid()) {
                throw std::logic_error("erase an invalid iterator");
            }

            auto  node= pos.nodeptr();
            if (node == nullptr) {
                throw std::logic_error("erase end iterator");
            }
            auto next_ptr = this->rbtree->erase(node, true);
            return iterator_t(this->rbtree, next_ptr, this->rbtree->version());
        }

        iterator_t erase(iterator_t first, iterator_t last) {
            return this->erase(const_iterator_t(first), const_iterator_t(last));
        }

        iterator_t erase(const_iterator_t first, const_iterator_t last) {
            if (first > last) {
                throw std::logic_error("invalid range");
            }

            iterator_t ans = this->end();
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

        void swap(generic_container& oth) noexcept {
            std::swap(this->rbtree, oth.rbtree);
        }

        void clear() {
            this->rbtree->clear();
        }
};


template<typename _Key, typename _Value, bool multi, bool kepp_position_info, typename Compare, typename Alloc>
bool operator==(const generic_container<_Key,_Value,multi,kepp_position_info,Compare,Alloc>& lhs,
                const generic_container<_Key,_Value,multi,kepp_position_info,Compare,Alloc>& rhs)
{
    if (lhs.size() != rhs.size()) return false;

    for (auto lhs_begin=lhs.begin(),rhs_begin=rhs.begin();lhs_begin!=lhs.end();lhs_begin++,rhs_begin++) {
        if (*lhs_begin != *rhs_begin) {
            return false;
        }
    }

    return true;
}

template<typename _Key, typename _Value, bool multi, bool kepp_position_info, typename Compare, typename Alloc>
bool operator!=(const generic_container<_Key,_Value,multi,kepp_position_info,Compare,Alloc>& lhs,
                const generic_container<_Key,_Value,multi,kepp_position_info,Compare,Alloc>& rhs)
{
    return !operator==(lhs, rhs);
}


template<
    typename _Key, typename _Value, bool multi, bool kepp_position_info,
    typename Compare = default_compare_t<_Key>,
    typename Alloc = default_allocato_t<_Key,_Value>,
    typename std::enable_if<!std::is_same<_Value,void>::value,bool>::type = true>
class generic_map: public generic_container<_Key,_Value,multi,kepp_position_info,Compare,Alloc> {
    private:
        using base_t = generic_container<_Key,_Value,multi,kepp_position_info,Compare,Alloc>;

    public:
        using rbtree_storage_type = typename base_t::rbtree_storage_type;
        using rbtree_storage_type_base = typename base_t::rbtree_storage_type_base;
        using iterator_t = typename base_t::iterator_t;
        using const_iterator_t = typename base_t::const_iterator_t;
        using reverse_iterator_t = typename base_t::reverse_const_iterator_t;
        using reverse_const_iterator_t = typename base_t::reverse_const_iterator_t;

        generic_map() = default;
        template<typename ...Args>
        generic_map(Args&& ...args): base_t(std::forward<Args>(args)...) {}

        generic_map(std::initializer_list<rbtree_storage_type_base> init, const Compare& cmp = {}, const Alloc& alloc = {}): base_t(std::move(init), cmp, alloc) {
        }

        generic_map(std::initializer_list<rbtree_storage_type_base> init, const Alloc& alloc): base_t(std::move(init), alloc) {
        }
};


template<
    typename _Key, bool multi, bool kepp_position_info,
    typename Compare = default_compare_t<_Key>,
    typename Alloc = default_allocato_t<_Key,void>>
class generic_set: public generic_container<_Key,void,multi,kepp_position_info,Compare,Alloc> {
    private:
        using base_t = generic_container<_Key,void,multi,kepp_position_info,Compare,Alloc>;

    public:
        using rbtree_storage_type = typename base_t::rbtree_storage_type;
        using rbtree_storage_type_base= typename base_t::rbtree_storage_type_base;
        using iterator_t = typename base_t::iterator_t;
        using const_iterator_t = typename base_t::const_iterator_t;
        using reverse_iterator_t = typename base_t::reverse_const_iterator_t;
        using reverse_const_iterator_t = typename base_t::reverse_const_iterator_t;

        generic_set() = default;
        template<typename ...Args>
        generic_set(Args&& ...args): base_t(std::forward<Args>(args)...) {}

        generic_set(std::initializer_list<rbtree_storage_type_base> init, const Compare& cmp = {}, const Alloc& alloc = {}): base_t(std::move(init), cmp, alloc) {
        }

        generic_set(std::initializer_list<rbtree_storage_type_base> init, const Alloc& alloc): base_t(std::move(init), alloc) {
        }
};


template<
    typename _Key, typename _Value, bool kepp_position_info,
    typename Compare = default_compare_t<_Key>,
    typename Alloc = default_allocato_t<_Key,_Value>>
class generic_unimap: public generic_map<_Key,_Value,false,kepp_position_info,Compare,Alloc> {
    private:
        using base_t = generic_map<_Key,_Value,false,kepp_position_info,Compare,Alloc>;

    public:
        using rbtree_storage_type = typename base_t::rbtree_storage_type;
        using rbtree_storage_type_base = typename base_t::rbtree_storage_type_base;
        using iterator_t = typename base_t::iterator_t;
        using const_iterator_t = typename base_t::const_iterator_t;
        using reverse_iterator_t = typename base_t::reverse_const_iterator_t;
        using reverse_const_iterator_t = typename base_t::reverse_const_iterator_t;

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
    typename Compare = default_compare_t<_Key>,
    typename Alloc = default_allocato_t<_Key,void>>
using set2 = generic_set<_Key,false,false,Compare,Alloc>;

template<
    typename _Key,
    typename Compare = default_compare_t<_Key>,
    typename Alloc = default_allocato_t<_Key,void>>
using pset = generic_set<_Key,false,true,Compare,Alloc>;

template<
    typename _Key,
    typename Compare = default_compare_t<_Key>,
    typename Alloc = default_allocato_t<_Key,void>>
using multiset2 = generic_set<_Key,true,false,Compare,Alloc>;

template<
    typename _Key,
    typename Compare = default_compare_t<_Key>,
    typename Alloc = default_allocato_t<_Key,void>>
using pmultiset = generic_set<_Key,true,true,Compare,Alloc>;


template<
    typename _Key, typename _Value,
    typename Compare = default_compare_t<_Key>,
    typename Alloc = default_allocato_t<_Key,_Value>>
using multimap2 = generic_map<_Key,_Value,true,false,Compare,Alloc>;

template<
    typename _Key, typename _Value,
    typename Compare = default_compare_t<_Key>,
    typename Alloc = default_allocato_t<_Key,_Value>>
using pmultimap = generic_map<_Key,_Value,true,true,Compare,Alloc>;

template<
    typename _Key, typename _Value,
    typename Compare = default_compare_t<_Key>,
    typename Alloc = default_allocato_t<_Key,_Value>>
using map2 = generic_unimap<_Key,_Value,false,Compare,Alloc>;

template<
    typename _Key, typename _Value,
    typename Compare = default_compare_t<_Key>,
    typename Alloc = default_allocato_t<_Key,_Value>>
using pmap = generic_unimap<_Key,_Value,true,Compare,Alloc>;


// TODO just for making intellisense work in development
template class generic_container<int,void,true,true>;
template class generic_container<int,int,true,true>;

