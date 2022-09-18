#pragma once
#include <utility>
#include <iterator>
#include <type_traits>
#include <memory>
#include <stdexcept>
using namespace std;

// TODO just for development
#define DEBUG 1

#ifdef DEBUG
#include <assert.h>
#include <queue>
#define RB_ASSERT(x) assert(x)
#else
#define RB_ASSERT(x)
#endif


template<typename K, typename V>
struct RBTreeValue {
    const K first;
    V second;

    inline bool operator<(const RBTreeValue<K, V>& o) const { return this->first < o.first; }
    inline bool operator==(const RBTreeValue<K, V>& o) const { return this->first == o.first; }

    template<typename Kx, typename Vx>
    RBTreeValue(Kx&& k, Vx&& v): first(std::forward<Kx>(k)), second(std::forward<Vx>(v)) {}
};

template<typename S>
struct RBTreeNode {
    using storage_type = S;

    RBTreeNode *left, *right, *parent;
    bool black;
    size_t num_nodes;
    storage_type value;

    RBTreeNode(RBTreeNode&& oth):
        left(oth.left), right(oth.right), parent(oth.parent),
        black(oth.black), num_nodes(oth.num_nodes), value(std::move(oth.value))
    {
        oth.parent = oth.right = oth.left = nullptr;
        oth.num_nodes = 0;
    }

    RBTreeNode(const RBTreeNode&) = delete;
    RBTreeNode& operator=(const RBTreeNode&) = delete;

    RBTreeNode& operator=(RBTreeNode&& oth) {
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
        this->num_nodes = oth.num_nodes;
        this->black = oth.black;
        oth.num_nodes = 0;
    }

    template<typename St, std::enable_if_t<std::is_same<std::remove_reference_t<std::remove_const_t<St>>,storage_type>::value, bool> = true>
    RBTreeNode(St&& val):
        left(nullptr), right(nullptr), parent(nullptr),
        black(false), num_nodes(1), value(std::forward<St>(val))
    {}

    ~RBTreeNode() {
        RB_ASSERT(this->left == nullptr);
        RB_ASSERT(this->right == nullptr);
    }
};

#if __cplusplus >= 201703L
template<typename _Key, typename _Value>
using rbtree_storage_type = typename std::conditional<std::is_same<_Value,void>::value,_Key,std::pair<const _Key, _Value>>::type;
template<typename _Key, typename _Value>
using rbtree_compare_type = _Key;
#else
template<typename _Key, typename _Value>
using rbtree_storage_type = typename std::conditional<std::is_same<_Value,void>::value,_Key,RBTreeValue<_Key,_Value>>::type;
template<typename _Key, typename _Value>
using rbtree_compare_type = rbtree_storage_type<_Key,_Value>;
#endif // __cplusplus >= 201703L

template<
    typename _Key, typename _Value, bool multi,
    typename Compare = std::less<rbtree_compare_type<_Key,_Value>>,
    typename Alloc = std::allocator<RBTreeNode<rbtree_storage_type<_Key,_Value>>>>
class RBTreeImpl {
    public:
        using storage_type = rbtree_storage_type<_Key,_Value>;
        using node_type = RBTreeNode<storage_type>;
#if __cplusplus >= 202002L
        using storage_allocator_ = std::allocator_traits<Alloc>::rebind_alloc<node_type>;
#else
        using storage_allocator_ = typename Alloc::template rebind<node_type>::other;
#endif // __cplusplus >= 202002L

    private:
        node_type* root;
        Compare cmp;
        storage_allocator_ allocator;
        size_t _version;

        template<typename St>
        inline node_type* construct_node(St&& val) {
            auto ptr = this->allocator.allocate(1);
            return new (ptr) node_type(std::forward<St>(val));
        }

        inline void delete_node(node_type* node) {
#if __cplusplus >= 201703L
            std::destroy_n(node, 1);
            this->allocator.deallocate(node, 1);
#else
            node->~node_type();
            this->allocator.deallocate(node, 1);
#endif // __cplusplus >= 201703L
        }

        inline bool comp(const storage_type& a, const storage_type& b) const
        {
#if __cplusplus >= 201703L
            if constexpr (std::is_same<_Value,void>::value) {
                return this->cmp(a, b);
            } else {
                return this->cmp(a.first, b.first);
            }
#else
            return this->cmp(a, b);
#endif // __cplusplus >= 201703L
        }

        inline void update_num_nodes(node_type* node, node_type* end) const
        {
            for (;node!=end;node=node->parent) {
                size_t n = 1;
                if (node->left) n += node->left->num_nodes;
                if (node->right) n += node->right->num_nodes;

                if (node->num_nodes == n)
                    break;
                node->num_nodes = n;
            }
        }

        inline void be_left_child(node_type* parent, node_type* child) const {
            parent->left = child;
            if (child) child->parent = parent;
            this->update_num_nodes(parent, parent->parent);
        }

        inline void be_right_child(node_type* parent, node_type* child) const {
            parent->right = child;
            if (child) child->parent = parent;
            this->update_num_nodes(parent, parent->parent);
        }

        inline node_type* right_rotate(node_type* node) {
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

        inline node_type* right_left_rotate(node_type* node) {
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

        inline node_type* left_rotate(node_type* node) {
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

        inline node_type* left_right_rotate(node_type* node) {
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

        inline void fix_redred(node_type* node) {
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

        inline bool is_black_node(node_type* node) const {
            return node == nullptr || node->black;
        }

        inline void fix_delete(node_type* extra_parent, node_type* extra_black, bool is_left_child) {
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

        inline node_type* minimum(node_type* node) const {
            for (;node->left;node=node->left);

            return node;
        }

        inline void swap_node(node_type* n1, node_type* n2) const {
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
        template<typename Sx>
        std::pair<node_type*,bool> insert(node_type* hint, Sx&& val)
        {
            this->_version++;
            auto node = this->construct_node(std::forward<Sx>(val));
            if (this->root == nullptr) {
                this->root = node;
                this->root->black = true;
                return std::make_pair(this->root, true);
            }

            auto cn = this->root;
            if (hint != nullptr) {
                bool left_is_ok = false, right_is_ok = false;
                auto left_node = hint, right_node = hint;

                for (;(!left_is_ok && left_node) || (!right_is_ok && right_node);) {
                    if (!left_is_ok && left_node) {
                        if (this->comp(left_node->value, val)) {
                            left_is_ok = true;
                        } else {
                            left_node = left_node->left;
                        }
                    }

                    if (!right_is_ok && right_node) {
                        if (this->comp(val, right_node->value)) {
                            right_is_ok = true;
                        } else {
                            right_node = right_node->right;
                        }
                    }
                }

                for (auto tp=hint->parent;(!left_is_ok || !right_is_ok) && tp;tp=tp->parent) {
                    if (!left_is_ok && this->comp(tp->value, val)) {
                        hint = tp;
                        left_is_ok = true;
                    }

                    if (!right_is_ok && this->comp(val, tp->value)) {
                        hint = tp;
                        right_is_ok = true;
                    }
                }

                if (left_is_ok && right_is_ok) {
                    cn = hint;
                }
            }

            for(;;) {
                if (this->comp(node->value, cn->value)) {
                    if (cn->left == nullptr) {
                        cn->left = node;
                        node->parent = cn;
                        break;
                    } else {
                        cn = cn->left;
                    }
                } else if (!multi && node->value == cn->value) {
                    cn->value = std::move(node->value);
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

            if (cn->black) {
                this->update_num_nodes(cn, nullptr);
            } else {
                this->fix_redred(node);
            }
            return std::make_pair(node, true);
        }

        template<typename Sx>
        std::pair<node_type*,bool> insert(Sx&& val) {
            return this->insert(nullptr, std::forward<Sx>(val));
        }

#ifdef DEBUG
        void check_consistency() const {
            RB_ASSERT(this->is_black_node(this->root));
            if (!this->root) return;

            std::queue<std::pair<node_type*,size_t>> queue;
            queue.push(std::make_pair(this->root, 0));
            size_t black_depth = 0;

            for (;!queue.empty();queue.pop()) {
                auto front = queue.front();
                auto node = front.first;
                auto bdepth = front.second + (node->black ? 1 : 0);
                auto left_n = node->left ? node->left->num_nodes : 0;
                auto right_n = node->right ? node->right->num_nodes : 0;

                black_depth = black_depth > bdepth ? black_depth : bdepth;

                RB_ASSERT(node->num_nodes == left_n + right_n + 1);
                if (!node->black) {
                    RB_ASSERT(this->is_black_node(node->left));
                    RB_ASSERT(this->is_black_node(node->right));
                }

                if (node->left) {
                    RB_ASSERT(node->left->parent == node);
                    RB_ASSERT(this->comp(node->left->value, node->value) || (multi && node->left->value == node->value));
                    queue.push(std::make_pair(node->left, bdepth));
                }
                if (node->right) {
                    RB_ASSERT(node->right->parent == node);
                    RB_ASSERT(!this->comp(node->right->value, node->value) || (multi && node->right->value == node->value));
                    queue.push(std::make_pair(node->right, bdepth));
                }

                if (!node->left && !node->right) {
                    RB_ASSERT(bdepth == black_depth);
                }
            }
        }
#endif // DEBUG

        node_type* erase(node_type* node, bool return_next_node)
        {
            RB_ASSERT(node != nullptr);
            node_type* extra_black = nullptr;
            node_type* extra_parent = nullptr;
            bool extra_is_left_child = true;
            this->_version++;

            node_type* next_node = nullptr;

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

        size_t indexof(const node_type* node) const {
            size_t ans = 0;
            if (node == nullptr)
                return this->size();

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

        node_type* lower_bound(const storage_type& val) const {
            auto root = this->root;
            node_type* ans = nullptr;
            if (!root) return ans;

            auto comp = [this](const storage_type& a, const storage_type& b) { return !this->comp(b, a); };

            for (auto node=root;node!=nullptr;) {
                if (comp(val, node->value)) {
                    if (ans == nullptr || comp(node->value, ans->value) || node->value == ans->value) {
                        ans = node;
                    }

                    node = node->left;
                } else {
                    node = node->right;
                }
            }

            return ans;
        }

        node_type* upper_bound(const storage_type& val) const {
            auto root = this->root;
            node_type* ans = nullptr;
            if (!root) return ans;

            auto comp = [this](const storage_type& a, const storage_type& b) { return this->comp(a, b); };

            for (auto node=root;node!=nullptr;) {
                if (this->comp(val, node->value)) {
                    if (ans == nullptr || comp(node->value, ans->value) || node->value == ans->value) {
                        ans = node;
                    }

                    node = node->left;
                } else {
                    node = node->right;
                }
            }

            return ans;
        }

        node_type* find(const storage_type& val) {
            auto node = this->lower_bound(val);
            return node->value == val ? node : nullptr;
        }

        const node_type* find(const storage_type& val) const {
            auto node = this->lower_bound(val);
            return node->value == val ? node : nullptr;
        }

        node_type* begin() {
            if (!this->root) return nullptr;

            return this->minimum(this->root);
        }

        const node_type* begin() const {
            if (!this->root) return nullptr;

            return this->minimum(this->root);
        }

        // TODO prototype
        node_type* advance(node_type* node, long n) const {
            // node == nullptr represent end
            if (node == nullptr) {
                // TODO why this->root (shoud be const qualified) can assign to node
                node = this->root;
                n += node && node->right ? node->right->num_nodes + 1 : 1;
            }

            for (;n!=0 && node!=nullptr;) {
                if (n < 0) { 
                    long k = 0;
                    if (node->left) {
                        k = node->left->num_nodes;
                    }

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
                    long k = 0;
                    if (node->right) {
                        k = node->right->num_nodes;
                    }

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

        inline size_t size() const {
            return this->root ? this->root->num_nodes : 0;
        }

        inline size_t version() const {
            return this->_version;
        }

        void copy_to(RBTreeImpl& target) const {
            target.~RBTreeImpl();
            target._version++;
            if (!this->root) return;

            // preorder traversing
            node_type* parent_node = nullptr;
            node_type** pptr = &target.root;
            for (const node_type* node=this->root;node!=nullptr;) {
                auto val = node->value;
                node_type* new_node = target.construct_node(val);
                new_node->black = node->black;;
                new_node->parent = parent_node;
                new_node->num_nodes = node->num_nodes;
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
        }

        RBTreeImpl(): root(nullptr), _version(0) {
        }

        ~RBTreeImpl() {
            this->clear();
        }
};

// TODO just for making intellisense work in development
template class RBTreeImpl<int,void,true>;

template<typename T>
struct IsRBTreeImpl : std::false_type {};
template<typename T1, typename T2, bool V1, typename T4, typename T5>
struct IsRBTreeImpl<RBTreeImpl<T1,T2,V1,T4,T5>> : std::true_type {};

template<bool reverse, bool const_iterator, typename RBTreeType, typename = std::enable_if<IsRBTreeImpl<RBTreeType>::value>>
class RBTreeImplIterator {
    public:
        using rbtree_t = RBTreeType;
        using storage_type = typename rbtree_t::storage_type;
        using node_type = typename rbtree_t::node_type;

        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = storage_type;
        using difference_type = long;
        using pointer = value_type*;
        using reference = typename std::conditional<const_iterator, const value_type&, value_type&>::type;
        using const_reference = const value_type&;

    private:
        std::weak_ptr<rbtree_t> tree;
        node_type* node;
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

    public:
        // TODO
        node_type* nodeptr() { return this->node; }

        const pointer operator->() const {
            this->check_version();
            if (this->node == nullptr) {
                throw std::out_of_range("dereference end of a container");
            }
            return &node->value;
        }

        pointer operator->() {
            this->check_version();
            if (this->node == nullptr) {
                throw std::out_of_range("dereference end of a container");
            }
            return &node->value;
        }

        const_reference operator*() const {
            this->check_version();
            if (this->node == nullptr) {
                throw std::out_of_range("dereference end of a container");
            }
            return node->value;
        }

        reference& operator*() {
            this->check_version();
            if (this->node == nullptr) {
                throw std::out_of_range("dereference end of a container");
            }
            return node->value;
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
                throw std::out_of_range("out of range by adding '" + std::to_string(n) + "'");
            }
        }

        RBTreeImplIterator& operator-=(int n) {
            return this->operator+=(-n);
        }

        RBTreeImplIterator operator+(int n) const {
            auto ans = *this;
            return ans.operator+=(n);
        }

        RBTreeImplIterator operator-(int n) const {
            auto ans = *this;
            return ans.operator-=(n);
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

        RBTreeImplIterator(std::weak_ptr<rbtree_t> tree, node_type* node): tree(tree), node(node) {}
};


namespace std {
    // TODO
    template<bool reverse, bool const_iterator, typename RBTreeType>
    typename std::iterator_traits<RBTreeImplIterator<reverse,const_iterator,RBTreeType>>::difference_type
    distance(RBTreeImplIterator<reverse,const_iterator,RBTreeType> iter1, RBTreeImplIterator<reverse,const_iterator,RBTreeType> iter2) {
        return 0;
    }
}


template<
    typename _Key, bool multi,
    typename Compare = std::less<rbtree_compare_type<_Key,void>>,
    typename Alloc = std::allocator<RBTreeNode<rbtree_storage_type<_Key,void>>>>
class generic_set {
    private:
        using rbtree_t = RBTreeImpl<_Key,void,multi,Compare,Alloc>;
        using iterator_t = RBTreeImplIterator<false,false,rbtree_t>;
        using const_iterator_t = RBTreeImplIterator<false,true,rbtree_t>;
        using reverse_iterator_t = RBTreeImplIterator<true,false,rbtree_t>;
        using reverse_const_iterator_t = RBTreeImplIterator<true,true,rbtree_t>;
        std::shared_ptr<rbtree_t> rbtree;

    public:
        generic_set(): rbtree(std::make_shared<rbtree_t>()) {}
        generic_set(const generic_set& oth): rbtree(std::make_shared<rbtree_t>())
        {
            oth.rbtree->copy_to(*this->rbtree);
        }
        generic_set(generic_set&& oth): rbtree(oth.rbtree)
        {
            oth.rbtree =std::make_shared<rbtree_t>();
        }

        generic_set& operator=(const generic_set& oth) {
            oth.rbtree->copy_to(*this->rbtree);
            return *this;
        }
        generic_set& operator=(generic_set&& oth) {
            this->rbtree->clear();
            // TODO increment version
            std::swap(this->rbtree, oth.rbtree);
            return *this;
        }

        inline iterator_t begin() { return iterator_t(this->rbtree, this->rbtree->begin()); }
        inline iterator_t end() { return iterator_t(this->rbtree, nullptr); }

        inline const_iterator_t begin() const { return const_iterator_t(this->rbtree, this->rbtree->begin()); }
        inline const_iterator_t end() const { return const_iterator_t(this->rbtree, nullptr); }

        inline const_iterator_t cbegin() const { return const_iterator_t(this->rbtree, this->rbtree->begin()); }
        inline const_iterator_t cend() const { return const_iterator_t(this->rbtree, nullptr); }

        inline reverse_iterator_t rbegin() { return reverse_iterator_t(this->rbtree, this->rbtree->begin()); }
        inline reverse_iterator_t rend() { return reverse_iterator_t(this->rbtree, nullptr); }

        inline reverse_const_iterator_t rbegin() const { return reverse_const_iterator_t(this->rbtree, this->rbtree->begin()); }
        inline reverse_const_iterator_t rend() const { return reverse_const_iterator_t(this->rbtree, nullptr); }

        inline reverse_const_iterator_t crbegin() const { return reverse_const_iterator_t(this->rbtree, this->rbtree->begin()); }
        inline reverse_const_iterator_t crend() const { return reverse_const_iterator_t(this->rbtree, nullptr); }

        iterator_t find(const _Key& key) {
            auto node = this->rbtree->find(key);
            return iterator_t(this->rbtree, node);
        }

        const_iterator_t find(const _Key& key) const {
            auto node = this->rbtree->find(key);
            return const_iterator_t(this->rbtree, node);
        }

        inline size_t size() const { return this->rbtree->size(); }

        template<typename ValType>
        std::pair<iterator_t,bool> insert(ValType&& val)
        {
            auto result = this->rbtree->insert(std::forward<ValType>(val));
            return make_pair(iterator_t(this->rbtree, result.first), result.second);
        };

        template<typename ValType>
        iterator_t insert(iterator_t hint, ValType&& val)
        {
            // TODO check iterator
            auto result = this->rbtree->insert(hint.nodeptr(), std::forward<ValType>(val));
            return iterator_t(this->rbtree, result.first);
        };

        iterator_t erase(iterator_t pos) {
            // TODO check iterator
            auto  node= pos.nodeptr();
            if (node == nullptr) {
                throw std::logic_error("erase end iterator");
            }
            auto next_ptr = this->rbtree->erase(node, true);
            return iterator_t(this->rbtree, next_ptr);
        }

        iterator_t erase(const_iterator_t pos) {
            // TODO check iterator
            auto  node= pos.nodeptr();
            if (node == nullptr) {
                throw std::logic_error("erase end iterator");
            }
            auto next_ptr = this->rbtree->erase(node, true);
            return iterator_t(this->rbtree, next_ptr);
        }

        void clear() {
            this->rbtree->clear();
        }
};


template<
    typename _Key,
    typename Compare = std::less<rbtree_compare_type<_Key,void>>,
    typename Alloc = std::allocator<RBTreeNode<rbtree_storage_type<_Key,void>>>>
using set2 = generic_set<_Key,false,Compare,Alloc>;

template<
    typename _Key,
    typename Compare = std::less<rbtree_compare_type<_Key,void>>,
    typename Alloc = std::allocator<RBTreeNode<rbtree_storage_type<_Key,void>>>>
using multiset2 = generic_set<_Key,true,Compare,Alloc>;


// TODO just for making intellisense work in development
template class generic_set<int, true>;

