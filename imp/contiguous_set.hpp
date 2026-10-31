#pragma once

#include <vector>

namespace imp
{
    template<typename ty>
    class contiguous_set
    {
        std::vector<ty> set_;

        using iterator = std::vector<ty>::iterator;
        using const_iterator = std::vector<ty>::const_iterator;
        using reverse_iterator = std::vector<ty>::reverse_iterator;
        using const_reverse_iterator = std::vector<ty>::const_reverse_iterator ;

    public:
        bool insert(const ty& el)
        {
            if (contains(el))
                return false;

            set_.push_back(el);
            return true;
        }

        bool insert(ty&& el)
        {
            if (contains(el))
                return false;

            set_.push_back(std::move(el));
            return true;
        }

        bool erase(const ty& el)
        {
            auto it = find(el);
            if (it != set_.end())
            {
                set_.erase(it);
                return true;
            }
            else
                return false;
        }

        const_iterator erase(const const_iterator& where)
        {
            return set_.erase(where);
        }

        const_iterator remove(size_t i)
        {
            return set_.erase(set_.begin() + i);
        }

        template<typename uy>
        bool replace(const ty& existing, uy&& with)
        {
            if (existing == with)
                return false;

            auto it_existing = find_mut(existing);
            if (it_existing != set_.end())
            {
                if (contains(with))
                    set_.erase(it_existing);
                else
                    *it_existing = std::forward<uy>(with);
                return true;
            }
            else
                return false;
        }

        const_iterator find(const ty& el) const
        {
            return std::find(set_.begin(), set_.end(), el);
        }

        bool contains(const ty& el) const
        {
            return find(el) != set_.end();
        }

        void clear()
        {
            set_.clear();
        }

        size_t size() const
        {
            return set_.size();
        }

        bool empty() const
        {
            return set_.empty();
        }

        const ty* data() const
        {
            return set_.data();
        }

        const ty& operator[](size_t i) const
        {
            return set_[i];
        }

        const ty& front() const
        {
            return set_.front();
        }

        const ty& back() const
        {
            return set_.back();
        }

        ty pop()
        {
            ty popped = std::move(set_.back());
            set_.pop_back();
            return popped;
        }

        const_iterator begin() const
        {
            return set_.begin();
        }

        const_iterator end() const
        {
            return set_.end();
        }

        const_iterator cbegin() const
        {
            return set_.cbegin();
        }

        const_iterator cend() const
        {
            return set_.cend();
        }

        const_reverse_iterator rbegin() const
        {
            return set_.begin();
        }

        const_reverse_iterator rend() const
        {
            return set_.end();
        }

        const_reverse_iterator crbegin() const
        {
            return set_.crbegin();
        }

        const_reverse_iterator crend() const
        {
            return set_.crend();
        }

    private:
        iterator find_mut(const ty& el)
        {
            return std::find(set_.begin(), set_.end(), el);
        }
    };
}
