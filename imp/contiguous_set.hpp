#pragma once

#include <vector>

namespace imp
{
    template<typename ty>
    class contiguous_set
    {
        std::vector<ty> _set;

        typedef std::vector<ty>::iterator               iterator;
        typedef std::vector<ty>::const_iterator         const_iterator;
        typedef std::vector<ty>::reverse_iterator       reverse_iterator;
        typedef std::vector<ty>::const_reverse_iterator const_reverse_iterator;

    public:
        bool insert(const ty& el)
        {
            if (contains(el))
                return false;
            _set.push_back(el);
            return true;
        }

        bool insert(ty&& el)
        {
            if (contains(el))
                return false;
            _set.push_back(std::move(el));
            return true;
        }

        bool erase(const ty& el)
        {
            auto it = find(el);
            if (it != _set.end())
            {
                _set.erase(it);
                return true;
            }
            else
                return false;
        }

        const_iterator erase(const const_iterator& where)
        {
            return _set.erase(where);
        }

        const_iterator remove(size_t i)
        {
            return _set.erase(_set.begin() + i);
        }

        template<typename uy>
        bool replace(const ty& existing, uy&& with)
        {
            if (existing == with)
                return false;

            auto it_existing = _find(existing);
            if (it_existing != _set.end())
            {
                if (contains(with))
                    _set.erase(it_existing);
                else
                    *it_existing = std::forward<uy>(with);
                return true;
            }
            else
                return false;
        }

    private:
        iterator _find(const ty& el)
        {
            return std::find(_set.begin(), _set.end(), el);
        }

    public:
        const_iterator find(const ty& el) const
        {
            return std::find(_set.begin(), _set.end(), el);
        }

        bool contains(const ty& el) const
        {
            return find(el) != _set.end();
        }

        void clear()
        {
            _set.clear();
        }

        size_t size() const
        {
            return _set.size();
        }

        bool empty() const
        {
            return _set.empty();
        }

        const ty* data() const
        {
            return _set.data();
        }

        const ty& operator[](size_t i) const
        {
            return _set[i];
        }

        const ty& front() const
        {
            return _set.front();
        }

        const ty& back() const
        {
            return _set.back();
        }

        ty pop()
        {
            ty popped = std::move(_set.back());
            _set.pop_back();
            return popped;
        }

        const_iterator begin() const
        {
            return _set.begin();
        }

        const_iterator end() const
        {
            return _set.end();
        }

        const_iterator cbegin() const
        {
            return _set.cbegin();
        }

        const_iterator cend() const
        {
            return _set.cend();
        }

        const_reverse_iterator rbegin() const
        {
            return _set.begin();
        }

        const_reverse_iterator rend() const
        {
            return _set.end();
        }

        const_reverse_iterator crbegin() const
        {
            return _set.crbegin();
        }

        const_reverse_iterator crend() const
        {
            return _set.crend();
        }
    };
}
