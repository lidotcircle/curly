#pragma once
#include <vector>
#include <algorithm>
#include <utility>
#include <iterator>
#include <type_traits>
using namespace std;

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
template<typename K>
struct RBTreeValue<K, void> {
    const K first;

    inline bool operator<(const RBTreeValue<K, void>& o) const { return this->first < o.first; }
    inline bool operator==(const RBTreeValue<K, void>& o) const { return this->first == o.first; }

    template<typename Kx, std::enable_if_t<std::is_same<std::remove_reference_t<std::remove_const_t<Kx>>,K>::value, bool> = true>
    RBTreeValue(Kx&& k): first(std::forward<Kx>(k)) {}
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
        if (this->left) {
            delete this->left;
            this->left = nullptr;
        }

        if (this->right) {
            delete this->right;
            this->right = nullptr;
        }
    }
};

template<typename S, bool multi, typename Compare = std::less<S>>
class RBTreeImpl {
    public:
        using storage_type = S;
        using node_type = RBTreeNode<storage_type>;

    private:
        node_type* root;
        Compare cmp;

        inline bool comp(const storage_type& a, const storage_type& b) const { return this->cmp(a, b); }

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
        void insert(Sx&& val)
        {
            auto node = new node_type(std::forward<Sx>(val));
            if (this->root == nullptr) {
                this->root = node;
                this->root->black = true;
                return;
            }

            auto cn = this->root;
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
                    delete node;
                    return;
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

        void erase(node_type* node)
        {
            RB_ASSERT(node != nullptr);
            node_type* extra_black = nullptr;
            node_type* extra_parent = nullptr;
            bool extra_is_left_child = true;

            if (node->left && node->right) {
                auto successor = this->minimum(node->right);
                RB_ASSERT(successor != nullptr);

                if (node == this->root) {
                    RB_ASSERT(node->parent == nullptr);
                    this->root = successor;
                }

                this->swap_node(successor, node);
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
            delete node;

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

        node_type* begin() {
            if (!this->root) return nullptr;

            return this->minimum(this->root);
        }

        const node_type* begin() const {
            if (!this->root) return nullptr;

            return this->minimum(this->root);
        }

        node_type* advance(node_type* node, long n) {
            if (node == nullptr) {
                node = this->root;
                n += node->right ? node->right->num_nodes + 1 : 1;
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

        RBTreeImpl(): root(nullptr) {
        }

        ~RBTreeImpl() {
            if (this->root) {
                delete this->root;
                this->root = nullptr;
            }
        }
};


template class RBTreeImpl<RBTreeValue<int,void>,false>;
template class RBTreeImpl<RBTreeValue<int,void>,true>;


template<typename S, bool multi, typename Compare>
class RBTreeImplIter {
    public:
        using rbtree_t = RBTreeImpl<S, multi, Compare>;
        using storage_type = typename rbtree_t::storage_type;
        using node_type = typename rbtree_t::node_type;
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = storage_type;
        using difference_type = long;
        using pointer = value_type*;
        using reference = value_type&;

    private:
        rbtree_t* tree;
        node_type* node;

    public:
        storage_type* operator->() {
            // TODO
            return &node->value;
        }

        reference operator*() const {
            // TODO
            return node->value;
        }

        reference operator*() {
            // TODO
            return node->value;
        }

        RBTreeImplIter& operator++() {
        }

        RBTreeImplIter operator++(int) {
            auto ans = *this;
            this->operator++();
            return ans;
        }

        RBTreeImplIter& operator--() {
        }

        RBTreeImplIter operator--(int) {
            auto ans = *this;
            this->operator--();
            return ans;
        }

        RBTreeImplIter operator+(int n) {
        }

        RBTreeImplIter operator-(int n) {
        }

        bool operator==(const RBTreeImplIter& oth) const {
            return oth.tree == this->tree && oth.node == this->node;
        }

        bool operator!=(const RBTreeImplIter& oth) const {
            return !this->operator==(oth);
        }

        bool operator<(const RBTreeImplIter& oth) const {
            // TODO
            return oth.tree == this->tree && oth.node == this->node;
        }
};
