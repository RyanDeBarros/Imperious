#pragma once

#include <concepts>
#include <stdexcept>
#include <vector>

namespace imp
{
    template<typename derived>
    class tree_node
    {
        tree_node* parent_ = nullptr;
        tree_node* left_sibling_ = nullptr;
        tree_node* right_sibling_ = nullptr;
        tree_node* children_root_ = nullptr;
        size_t children_size_ = 0;

    public:
        tree_node() = default;

        tree_node(const tree_node&) = delete;

        tree_node(tree_node&& o) noexcept
            : parent_(o.parent_)
            , left_sibling_(o.left_sibling_)
            , right_sibling_(o.right_sibling_)
            , children_root_(o.children_root_)
            , children_size_(o.children_size_)
        {
            o.parent_ = nullptr;
            o.left_sibling_ = nullptr;
            o.right_sibling_ = nullptr;
            o.children_root_ = nullptr;
            o.children_size_ = 0;

            if (left_sibling_)
                left_sibling_->right_sibling_ = this;

            if (right_sibling_)
                right_sibling_->left_sibling_ = this;

            set_parent_of_children(this);
        }

        virtual ~tree_node()
        {
            detach();
            clear_children();
        }

        tree_node& operator=(const tree_node&) = delete;

        tree_node& operator=(tree_node&& o) noexcept
        {
            if (this != &o)
            {
                detach();
                set_parent_of_children(nullptr);

                parent_ = o.parent_;
                left_sibling_ = o.left_sibling_;
                right_sibling_ = o.right_sibling_;
                children_root_ = o.children_root_;
                children_size_ = o.children_size_;

                o.parent_ = nullptr;
                o.left_sibling_ = nullptr;
                o.right_sibling_ = nullptr;
                o.children_root_ = nullptr;
                o.children_size_ = 0;

                if (left_sibling_)
                    left_sibling_->right_sibling_ = this;

                if (right_sibling_)
                    right_sibling_->left_sibling_ = this;

                set_parent_of_children(this);
            }
            return *this;
        }

    private:
        void set_parent_of_children(tree_node* parent)
        {
            if (children_root_)
            {
                children_root_->parent_ = parent;
                tree_node* sibling = children_root_->right_sibling_;
                while (sibling != children_root_)
                {
                    sibling->parent_ = parent;
                    sibling = sibling->right_sibling_;
                }
            }
        }

    protected:
        virtual void on_attach(derived* old_parent, derived* new_parent) {}

    private:
        void on_attach_call(tree_node* old_parent)
        {
            on_attach(static_cast<derived*>(old_parent), static_cast<derived*>(parent_));
        }

    public:
        void attach(derived* parent)
        {
            if (parent_ == parent)
                return;

            detach();
            tree_node* old_parent = parent_;
            parent_ = parent;
            if (!parent_)
                return;

            if (parent_->children_root_)
            {
                tree_node* last = parent_->children_root_->left_sibling_;
                if (!last)
                    last = parent_->children_root_;

                last->right_sibling_ = this;
                left_sibling_ = last;
                parent_->children_root_->left_sibling_ = this;
                right_sibling_ = parent_->children_root_;
            }
            else
                parent_->children_root_ = this;

            ++parent_->children_size_;
            on_attach_call(old_parent);
        }

        void attach_right(derived& left_sibling)
        {
            if (this == &left_sibling)
                throw std::logic_error("circular reference"); // TODO imp error ?

            if (parent_ == left_sibling.parent_)
            {
                swap_with_sibling(*left_sibling.right_sibling_);
                return;
            }

            detach();
            tree_node* old_parent = parent_;
            parent_ = left_sibling.parent_;
            if (!parent_)
                return;

            tree_node* right_sibling = left_sibling.right_sibling_;
            if (!right_sibling)
                right_sibling = &left_sibling;

            left_sibling.right_sibling_ = this;
            left_sibling_ = &left_sibling;
            right_sibling->left_sibling_ = this;
            right_sibling_ = right_sibling;

            ++parent_->children_size_;
            on_attach_call(old_parent);
        }

        void attach_left(derived& right_sibling)
        {
            if (this == &right_sibling)
                throw std::logic_error("circular reference"); // TODO imp error ?

            if (parent_ == right_sibling.parent_)
            {
                swap_with_sibling(*right_sibling.left_sibling_);
                return;
            }

            detach();
            tree_node* old_parent = parent_;
            parent_ = right_sibling.parent_;
            if (!parent_)
                return;

            tree_node* left_sibling = right_sibling.left_sibling_;
            if (!left_sibling)
                left_sibling = &right_sibling;

            right_sibling.left_sibling_ = this;
            right_sibling_ = &right_sibling;
            left_sibling->right_sibling_ = this;
            left_sibling_ = left_sibling;

            if (parent_->children_root_ == &right_sibling)
                parent_->children_root_ = this;

            ++parent_->children_size_;
            on_attach_call(old_parent);
        }

        void detach()
        {
            if (!parent_)
                return;

            if (parent_->children_root_ == this)
                parent_->children_root_ = right_sibling_;

            if (left_sibling_)
            {
                if (left_sibling_ == right_sibling_)
                {
                    left_sibling_->right_sibling_ = nullptr;
                    right_sibling_->left_sibling_ = nullptr;
                }
                else
                {
                    left_sibling_->right_sibling_ = right_sibling_;
                    right_sibling_->left_sibling_ = left_sibling_;
                }

                left_sibling_ = nullptr;
                right_sibling_ = nullptr;
            }

            --parent_->children_size_;

            tree_node* old_parent = parent_;
            parent_ = nullptr;
            on_attach_call(old_parent);
        }

        void clear_children()
        {
            if (!children_root_)
                return;

            tree_node* sibling = children_root_->right_sibling_;

            children_root_->parent_ = nullptr;
            children_root_->on_attach_call(this);
            children_root_->left_sibling_ = nullptr;
            children_root_->right_sibling_ = nullptr;

            while (sibling && sibling != children_root_)
            {
                tree_node* old = sibling;
                sibling = sibling->right_sibling_;

                old->parent_ = nullptr;
                old->on_attach_call(this);
                old->left_sibling_ = nullptr;
                old->right_sibling_ = nullptr;
            }
            children_root_ = nullptr;
            children_size_ = 0;
        }

        size_t children_size() const
        {
            return children_size_;
        }

        bool sibling_comes_before(const tree_node& sibling) const
        {
            if (this == &sibling || parent_ == nullptr || parent_ != sibling.parent_)
                return false;

            if (this == parent_->children_root_)
                return true;
            else if (&sibling == parent_->children_root_)
                return false;
            else
            {
                const tree_node* checker = left_sibling_;
                while (checker != parent_->children_root_)
                {
                    if (checker == &sibling)
                        return false;
                    checker = checker->left_sibling_;
                }
                return true;
            }
        }

        bool sibling_comes_after(const tree_node& sibling) const
        {
            if (this == &sibling || parent_ == nullptr || parent_ != sibling.parent_)
                return false;

            if (this == parent_->children_root_)
                return false;
            else if (&sibling == parent_->children_root_)
                return true;
            else
            {
                const tree_node* checker = left_sibling_;
                while (checker != parent_->children_root_)
                {
                    if (checker == &sibling)
                        return true;
                    checker = checker->left_sibling_;
                }
                return false;
            }
        }

        void swap_with_sibling(tree_node& sibling)
        {
            if (this == &sibling || !parent_ || parent_ != sibling.parent_)
                return;

            if (parent_->children_root_ == this)
                parent_->children_root_ = &sibling;
            else if (parent_->children_root_ == &sibling)
                parent_->children_root_ = this;

            if (&sibling == left_sibling_)
            {
                if (&sibling != right_sibling_)
                {
                    // We are not the only children of parent
                    sibling.right_sibling_ = right_sibling_;
                    left_sibling_ = sibling.left_sibling_;
                    sibling.left_sibling_ = this;
                    right_sibling_ = &sibling;
                }
            }
            else if (&sibling == right_sibling_)
            {
                sibling.left_sibling_ = left_sibling_;
                right_sibling_ = sibling.right_sibling_;
                sibling.right_sibling_ = this;
                left_sibling_ = &sibling;
            }
            else
            {
                tree_node* my_left = left_sibling_;
                tree_node* my_right = right_sibling_;
                tree_node* their_left = sibling.left_sibling_;
                tree_node* their_right = sibling.right_sibling_;

                left_sibling_ = their_left;
                right_sibling_ = their_right;
                sibling.left_sibling_ = my_left;
                sibling.right_sibling_ = my_right;
            }
        }

        const derived* get_parent() const
        {
            return static_cast<const derived*>(parent_);
        }

        derived* get_parent()
        {
            return static_cast<derived*>(parent_);
        }

        const derived* get_left_sibling() const
        {
            return static_cast<const derived*>(left_sibling_);
        }

        derived* get_left_sibling()
        {
            return static_cast<derived*>(left_sibling_);
        }

        const derived* get_right_sibling() const
        {
            return static_cast<const derived*>(right_sibling_);
        }

        derived* get_right_sibling()
        {
            return static_cast<derived*>(right_sibling_);
        }

        // This method can be slow. It's recommended to manipulate the order of nodes by using
        // attach_left()/attach_right()/move_left()/move_right()/swap_sibling()
        size_t get_position_in_parent() const
        {
            if (!parent_)
                throw std::runtime_error("null pointer"); // TODO imp error ?

            size_t index = 0;
            tree_node* distance = this;
            while (distance != parent_->children_root_)
            {
                ++index;
                distance = distance->left_sibling_;
            }

            return index;
        }

        // This method can be slow. It's recommended to manipulate the order of nodes by using
        // attach_left()/attach_right()/move_left()/move_right()/swap_sibling()
        void set_position_in_parent(size_t index)
        {
            if (!parent_)
                throw std::runtime_error("null pointer"); // TODO imp error ?

            if (index >= parent_->children_size_)
                throw std::out_of_range(); // TODO imp error ?

            size_t current_index = get_position_in_parent();
            if (current_index == index)
                return;

            if (current_index < index)
            {
                while (current_index < index)
                {
                    ++current_index;
                    swap_with_sibling(right_sibling_);
                }
            }
            else
            {
                while (current_index > index)
                {
                    ++index;
                    swap_with_sibling(left_sibling_);
                }
            }
        }

        class iterator
        {
            friend class tree_node;
            friend class const_iterator;

        private:
            const tree_node* node_;
            tree_node* child_;

        private:
            iterator(const tree_node* node, tree_node* child)
                : node_(node)
                , child_(child)
            {
            }

        public:
            iterator(const iterator&) = default;
            iterator(iterator&&) = default;
            iterator& operator=(const iterator&) = default;
            iterator& operator=(iterator&&) = default;

            bool operator==(const iterator&) const = default;
            bool operator!=(const iterator&) const = default;

            const derived& operator*() const
            {
                if (!child_)
                    throw std::logic_error("invalid iterator"); // TODO imp error ?

                return *static_cast<const derived*>(child_);
            }

            derived& operator*()
            {
                if (!child_)
                    throw std::logic_error("invalid iterator"); // TODO imp error ?

                return *static_cast<derived*>(child_);
            }

            const derived* operator->() const
            {
                if (!child_)
                    throw std::logic_error("invalid iterator"); // TODO imp error ?

                return static_cast<const derived*>(child_);
            }

            derived* operator->()
            {
                if (!child_)
                    throw std::logic_error("invalid iterator"); // TODO imp error ?

                return static_cast<derived*>(child_);
            }

            iterator& operator++()
            {
                if (!child_)
                    throw std::logic_error("invalid iterator"); // TODO imp error ?

                tree_node* next = child_->right_sibling_;
                if (next != node_->children_root_)
                    child_ = next;
                else
                    child_ = nullptr;

                return *this;
            }

            iterator operator++(int)
            {
                iterator it(*this);
                ++*this;
                return it;
            }
        };

        class const_iterator
        {
            friend class tree_node;

        private:
            const tree_node* node_;
            const tree_node* child_;

        private:
            const_iterator(const tree_node* node, const tree_node* child)
                : node_(node)
                , child_(child)
            {
            }

        public:
            const_iterator(const iterator& it)
                : node_(it.node_)
                , child_(it.child_)
            {
            }
            
            const_iterator(iterator&& it) noexcept
                : node_(it.node_)
                , child_(it.child_)
            {
            }

            const_iterator(const const_iterator&) = default;
            const_iterator(const_iterator&&) = default;
            const_iterator& operator=(const const_iterator&) = default;
            const_iterator& operator=(const_iterator&&) = default;

            bool operator==(const const_iterator&) const = default;
            bool operator!=(const const_iterator&) const = default;

            const derived& operator*() const
            {
                if (!child_)
                    throw std::logic_error("invalid iterator"); // TODO imp error ?

                return *static_cast<const derived*>(child_);
            }

            const derived* operator->() const
            {
                if (!child_)
                    throw std::logic_error("invalid iterator"); // TODO imp error ?

                return static_cast<const derived*>(child_);
            }

            const_iterator& operator++()
            {
                if (!child_)
                    throw std::logic_error("invalid iterator"); // TODO imp error ?

                const tree_node* next = child_->right_sibling_;
                if (next != node_->children_root_)
                    child_ = next;
                else
                    child_ = nullptr;

                return *this;
            }

            const_iterator operator++(int)
            {
                const_iterator it(*this);
                ++*this;
                return it;
            }
        };

        iterator begin()
        {
            return iterator(this, children_root_);
        }

        iterator end()
        {
            return iterator(this, nullptr);
        }

        const_iterator cbegin() const
        {
            return const_iterator(this, children_root_);
        }

        const_iterator cend() const
        {
            return const_iterator(this, nullptr);
        }

        const_iterator begin() const
        {
            return const_iterator(this, children_root_);
        }

        const_iterator end() const
        {
            return const_iterator(this, nullptr);
        }
    };
}
