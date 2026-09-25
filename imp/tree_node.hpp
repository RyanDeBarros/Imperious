#pragma once

#include <concepts>
#include <stdexcept>
#include <vector>

namespace imp
{
    template<typename derived>
    class tree_node
    {
        tree_node* _parent = nullptr;
        tree_node* _left_sibling = nullptr;
        tree_node* _right_sibling = nullptr;
        tree_node* _children_root = nullptr;
        size_t _children_size = 0;

    public:
        tree_node() = default;

        tree_node(const tree_node&) = delete;

        tree_node(tree_node&& other) noexcept
            : _parent(other._parent), _left_sibling(other._left_sibling), _right_sibling(other._right_sibling), _children_root(other._children_root), _children_size(other._children_size)
        {
            other._parent = nullptr;
            other._left_sibling = nullptr;
            other._right_sibling = nullptr;
            other._children_root = nullptr;
            other._children_size = 0;

            if (_left_sibling)
                _left_sibling->_right_sibling = this;
            if (_right_sibling)
                _right_sibling->_left_sibling = this;

            set_parent_of_children(this);
        }

        virtual ~tree_node()
        {
            detach();
            clear_children();
        }

        tree_node& operator=(const tree_node&) = delete;

        tree_node& operator=(tree_node&& other) noexcept
        {
            if (this != &other)
            {
                detach();
                set_parent_of_children(nullptr);

                _parent = other._parent;
                _left_sibling = other._left_sibling;
                _right_sibling = other._right_sibling;
                _children_root = other._children_root;
                _children_size = other._children_size;

                other._parent = nullptr;
                other._left_sibling = nullptr;
                other._right_sibling = nullptr;
                other._children_root = nullptr;
                other._children_size = 0;

                if (_left_sibling)
                    _left_sibling->_right_sibling = this;
                if (_right_sibling)
                    _right_sibling->_left_sibling = this;

                set_parent_of_children(this);
            }
            return *this;
        }

    private:
        void set_parent_of_children(tree_node* parent)
        {
            if (_children_root)
            {
                _children_root->_parent = parent;
                tree_node* sibling = _children_root->_right_sibling;
                while (sibling != _children_root)
                {
                    sibling->_parent = parent;
                    sibling = sibling->_right_sibling;
                }
            }
        }

    protected:
        virtual void on_attach(derived* old_parent, derived* new_parent) {}

    private:
        void on_attach_call(tree_node* old_parent)
        {
            on_attach(static_cast<derived*>(old_parent), static_cast<derived*>(_parent));
        }

    public:
        void attach(derived* parent)
        {
            if (_parent == parent)
                return;

            detach();
            tree_node* old_parent = _parent;
            _parent = parent;
            if (!_parent)
                return;

            if (_parent->_children_root)
            {
                tree_node* last = _parent->_children_root->_left_sibling;
                if (!last)
                    last = _parent->_children_root;

                last->_right_sibling = this;
                _left_sibling = last;
                _parent->_children_root->_left_sibling = this;
                _right_sibling = _parent->_children_root;
            }
            else
                _parent->_children_root = this;

            ++_parent->_children_size;
            on_attach_call(old_parent);
        }

        void attach_right(derived& left_sibling)
        {
            if (this == &left_sibling)
                throw std::logic_error("imp::tree_node: circular reference"); // TODO imp error ?

            if (_parent == left_sibling._parent)
            {
                swap_with_sibling(*left_sibling._right_sibling);
                return;
            }

            detach();
            tree_node* old_parent = _parent;
            _parent = left_sibling._parent;
            if (!_parent)
                return;

            tree_node* right_sibling = left_sibling._right_sibling;
            if (!right_sibling)
                right_sibling = &left_sibling;

            left_sibling._right_sibling = this;
            _left_sibling = &left_sibling;
            right_sibling->_left_sibling = this;
            _right_sibling = right_sibling;

            ++_parent->_children_size;
            on_attach_call(old_parent);
        }

        void attach_left(derived& right_sibling)
        {
            if (this == &right_sibling)
                throw std::logic_error("imp::tree_node: circular reference"); // TODO imp error ?

            if (_parent == right_sibling._parent)
            {
                swap_with_sibling(*right_sibling._left_sibling);
                return;
            }

            detach();
            tree_node* old_parent = _parent;
            _parent = right_sibling._parent;
            if (!_parent)
                return;

            tree_node* left_sibling = right_sibling._left_sibling;
            if (!left_sibling)
                left_sibling = &right_sibling;

            right_sibling._left_sibling = this;
            _right_sibling = &right_sibling;
            left_sibling->_right_sibling = this;
            _left_sibling = left_sibling;

            if (_parent->_children_root == &right_sibling)
                _parent->_children_root = this;

            ++_parent->_children_size;
            on_attach_call(old_parent);
        }

        void detach()
        {
            if (!_parent)
                return;

            if (_parent->_children_root == this)
                _parent->_children_root = _right_sibling;

            if (_left_sibling)
            {
                if (_left_sibling == _right_sibling)
                {
                    _left_sibling->_right_sibling = nullptr;
                    _right_sibling->_left_sibling = nullptr;
                }
                else
                {
                    _left_sibling->_right_sibling = _right_sibling;
                    _right_sibling->_left_sibling = _left_sibling;
                }

                _left_sibling = nullptr;
                _right_sibling = nullptr;
            }

            --_parent->_children_size;

            tree_node* old_parent = _parent;
            _parent = nullptr;
            on_attach_call(old_parent);
        }

        void clear_children()
        {
            if (!_children_root)
                return;

            tree_node* sibling = _children_root->_right_sibling;

            _children_root->_parent = nullptr;
            _children_root->on_attach_call(this);
            _children_root->_left_sibling = nullptr;
            _children_root->_right_sibling = nullptr;

            while (sibling && sibling != _children_root)
            {
                tree_node* old = sibling;
                sibling = sibling->_right_sibling;

                old->_parent = nullptr;
                old->on_attach_call(this);
                old->_left_sibling = nullptr;
                old->_right_sibling = nullptr;
            }
            _children_root = nullptr;
            _children_size = 0;
        }

        size_t children_size() const
        {
            return _children_size;
        }

        bool sibling_comes_before(const tree_node& sibling) const
        {
            if (this == &sibling || _parent == nullptr || _parent != sibling._parent)
                return false;

            if (this == _parent->_children_root)
                return true;
            else if (&sibling == _parent->_children_root)
                return false;
            else
            {
                const tree_node* checker = _left_sibling;
                while (checker != _parent->_children_root)
                {
                    if (checker == &sibling)
                        return false;
                    checker = checker->_left_sibling;
                }
                return true;
            }
        }

        bool sibling_comes_after(const tree_node& sibling) const
        {
            if (this == &sibling || _parent == nullptr || _parent != sibling._parent)
                return false;

            if (this == _parent->_children_root)
                return false;
            else if (&sibling == _parent->_children_root)
                return true;
            else
            {
                const tree_node* checker = _left_sibling;
                while (checker != _parent->_children_root)
                {
                    if (checker == &sibling)
                        return true;
                    checker = checker->_left_sibling;
                }
                return false;
            }
        }

        void swap_with_sibling(tree_node& sibling)
        {
            if (this == &sibling || !_parent || _parent != sibling._parent)
                return;

            if (_parent->_children_root == this)
                _parent->_children_root = &sibling;
            else if (_parent->_children_root == &sibling)
                _parent->_children_root = this;

            if (&sibling == _left_sibling)
            {
                if (&sibling != _right_sibling)
                {
                    // We are not the only children of parent
                    sibling._right_sibling = _right_sibling;
                    _left_sibling = sibling._left_sibling;
                    sibling._left_sibling = this;
                    _right_sibling = &sibling;
                }
            }
            else if (&sibling == _right_sibling)
            {
                sibling._left_sibling = _left_sibling;
                _right_sibling = sibling._right_sibling;
                sibling._right_sibling = this;
                _left_sibling = &sibling;
            }
            else
            {
                tree_node* my_left = _left_sibling;
                tree_node* my_right = _right_sibling;
                tree_node* their_left = sibling._left_sibling;
                tree_node* their_right = sibling._right_sibling;

                _left_sibling = their_left;
                _right_sibling = their_right;
                sibling._left_sibling = my_left;
                sibling._right_sibling = my_right;
            }
        }

        const derived* get_parent() const
        {
            return static_cast<const derived*>(_parent);
        }

        derived* get_parent()
        {
            return static_cast<derived*>(_parent);
        }

        const derived* get_left_sibling() const
        {
            return static_cast<const derived*>(_left_sibling);
        }

        derived* get_left_sibling()
        {
            return static_cast<derived*>(_left_sibling);
        }

        const derived* get_right_sibling() const
        {
            return static_cast<const derived*>(_right_sibling);
        }

        derived* get_right_sibling()
        {
            return static_cast<derived*>(_right_sibling);
        }

        // This method can be slow. It's recommended to manipulate the order of nodes by using attach_left()/attach_right()/move_left()/move_right()/swap_sibling()
        size_t get_position_in_parent() const
        {
            if (!_parent)
                throw std::runtime_error("imp::tree_node: null pointer"); // TODO imp error ?

            size_t index = 0;
            tree_node* distance = this;
            while (distance != _parent->_children_root)
            {
                ++index;
                distance = distance->_left_sibling;
            }
            return index;
        }

        // This method can be slow. It's recommended to manipulate the order of nodes by using attach_left()/attach_right()/move_left()/move_right()/swap_sibling()
        void set_position_in_parent(size_t index)
        {
            if (!_parent)
                throw std::runtime_error("imp::tree_node: null pointer"); // TODO imp error ?

            if (index >= _parent->_children_size)
                throw std::out_of_range(); // TODO imp error ?

            size_t current_index = get_position_in_parent();
            if (current_index == index)
                return;

            if (current_index < index)
            {
                while (current_index < index)
                {
                    ++current_index;
                    swap_with_sibling(_right_sibling);
                }
            }
            else
            {
                while (current_index > index)
                {
                    ++index;
                    swap_with_sibling(_left_sibling);
                }
            }
        }

        class iterator
        {
            const tree_node* node;
            tree_node* child;

            friend class tree_node;
            friend class const_iterator;
            iterator(const tree_node* node, tree_node* child) : node(node), child(child) {}

        public:
            iterator(const iterator&) = default;
            iterator(iterator&&) = default;
            iterator& operator=(const iterator&) = default;
            iterator& operator=(iterator&&) = default;

            const derived& operator*() const
            {
                if (!child)
                    throw std::logic_error("imp::tree_node: invalid iterator"); // TODO imp error ?
                return *static_cast<const derived*>(child);
            }

            derived& operator*()
            {
                if (!child)
                    throw std::logic_error("imp::tree_node: invalid iterator"); // TODO imp error ?
                return *static_cast<derived*>(child);
            }

            const derived* operator->() const
            {
                if (!child)
                    throw std::logic_error("imp::tree_node: invalid iterator"); // TODO imp error ?
                return static_cast<const derived*>(child);
            }

            derived* operator->()
            {
                if (!child)
                    throw std::logic_error("imp::tree_node: invalid iterator"); // TODO imp error ?
                return static_cast<derived*>(child);
            }

            iterator& operator++()
            {
                if (!child)
                    throw std::logic_error("imp::tree_node: invalid iterator"); // TODO imp error ?

                tree_node* next = child->_right_sibling;
                if (next != node->_children_root)
                    child = next;
                else
                    child = nullptr;

                return *this;
            }

            iterator operator++(int) { iterator it(*this); ++*this; return it; }

            bool operator==(const iterator&) const = default;
            bool operator!=(const iterator&) const = default;
        };

        class const_iterator
        {
            const tree_node* node;
            const tree_node* child;

            friend class tree_node;
            const_iterator(const tree_node* node, const tree_node* child) : node(node), child(child) {}

        public:
            const_iterator(const iterator& it) : node(it.node), child(it.child) {}
            const_iterator(iterator&& it) noexcept : node(it.node), child(it.child) {}

            const_iterator(const const_iterator&) = default;
            const_iterator(const_iterator&&) = default;
            const_iterator& operator=(const const_iterator&) = default;
            const_iterator& operator=(const_iterator&&) = default;

            const derived& operator*() const
            {
                if (!child)
                    throw std::logic_error("imp::tree_node: invalid iterator"); // TODO imp error ?

                return *static_cast<const derived*>(child);
            }

            const derived* operator->() const
            {
                if (!child)
                    throw std::logic_error("imp::tree_node: invalid iterator"); // TODO imp error ?

                return static_cast<const derived*>(child);
            }

            const_iterator& operator++()
            {
                if (!child)
                    throw std::logic_error("imp::tree_node: invalid iterator"); // TODO imp error ?

                const tree_node* next = child->_right_sibling;
                if (next != node->_children_root)
                    child = next;
                else
                    child = nullptr;

                return *this;
            }

            const_iterator operator++(int) { const_iterator it(*this); ++*this; return it; }

            bool operator==(const const_iterator&) const = default;
            bool operator!=(const const_iterator&) const = default;
        };

        iterator begin() { return iterator(this, _children_root); }
        iterator end() { return iterator(this, nullptr); }
        const_iterator cbegin() const { return const_iterator(this, _children_root); }
        const_iterator cend() const { return const_iterator(this, nullptr); }
        const_iterator begin() const { return const_iterator(this, _children_root); }
        const_iterator end() const { return const_iterator(this, nullptr); }
    };
}
