#pragma once

#include <vector>

namespace imp
{
    template<typename key_ty, typename value_ty>
    class contiguous_map
    {
        struct pair
        {
            key_ty key;
            mutable value_ty value;
        };

        std::vector<pair> _map;

    public:
        typedef std::vector<pair>::const_iterator const_iterator;
        typedef std::vector<pair>::iterator       iterator;

        template<typename key_ty_, typename value_ty_>
        const_iterator assign(key_ty_&& key, value_ty_&& value)
        {
            for (auto it = _map.begin(); it != _map.end(); ++it)
            {
                if (it->key == key)
                {
                    it->value = std::forward<value_ty_>(value);
                    return it;
                }
            }

            _map.push_back({ std::forward<key_ty_>(key), std::forward<value_ty_>(value) });
            return _map.end() - 1;
        }

        const value_ty* try_get(const key_ty& key) const
        {
            for (const pair& pair : _map)
            {
                if (pair.key == key)
                    return &pair.value;
            }

            return nullptr;
        }

        value_ty* try_get(const key_ty& key)
        {
            for (const pair& pair : _map)
            {
                if (pair.key == key)
                    return &pair.value;
            }

            return nullptr;
        }

        const_iterator erase(const key_ty& key)
        {
            return _map.erase(find(key));
        }

        const_iterator erase(const const_iterator& where)
        {
            return _map.erase(where);
        }

        const_iterator find(const key_ty& key) const
        {
            for (auto it = _map.begin(); it != _map.end(); ++it)
            {
                if (it->key == key)
                    return it;
            }

            return _map.end();
        }

        const_iterator begin() const
        {
            return _map.begin();
        }

        const_iterator end() const
        {
            return _map.end();
        }

        bool contains(const key_ty& key) const
        {
            for (auto it = _map.begin(); it != _map.end(); ++it)
            {
                if (it->key == key)
                    return true;
            }

            return false;
        }

        bool empty() const
        {
            return _map.empty();
        }

        size_t size() const
        {
            return _map.size();
        }

        void clear()
        {
            _map.clear();
        }
    };
}
