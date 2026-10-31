#pragma once

#include <vector>

namespace imp
{
    template<typename key_ty, typename value_ty>
    class contiguous_map
    {
    private:
        struct pair
        {
            key_ty key;
            mutable value_ty value;
        };

    public:
        using const_iterator = std::vector<pair>::const_iterator;
        using iterator = std::vector<pair>::iterator;

    private:
        std::vector<pair> map_;

    public:
        template<typename key_ty_, typename value_ty_>
        const_iterator assign(key_ty_&& key, value_ty_&& value)
        {
            for (auto it = map_.begin(); it != map_.end(); ++it)
            {
                if (it->key == key)
                {
                    it->value = std::forward<value_ty_>(value);
                    return it;
                }
            }

            map_.push_back({ std::forward<key_ty_>(key), std::forward<value_ty_>(value) });
            return map_.end() - 1;
        }

        const value_ty* try_get(const key_ty& key) const
        {
            for (const pair& pair : map_)
            {
                if (pair.key == key)
                    return &pair.value;
            }

            return nullptr;
        }

        value_ty* try_get(const key_ty& key)
        {
            for (const pair& pair : map_)
            {
                if (pair.key == key)
                    return &pair.value;
            }

            return nullptr;
        }

        const_iterator erase(const key_ty& key)
        {
            return map_.erase(find(key));
        }

        const_iterator erase(const const_iterator& where)
        {
            return map_.erase(where);
        }

        const_iterator find(const key_ty& key) const
        {
            for (auto it = map_.begin(); it != map_.end(); ++it)
            {
                if (it->key == key)
                    return it;
            }

            return map_.end();
        }

        const_iterator begin() const
        {
            return map_.begin();
        }

        const_iterator end() const
        {
            return map_.end();
        }

        bool contains(const key_ty& key) const
        {
            for (auto it = map_.begin(); it != map_.end(); ++it)
            {
                if (it->key == key)
                    return true;
            }

            return false;
        }

        bool empty() const
        {
            return map_.empty();
        }

        size_t size() const
        {
            return map_.size();
        }

        void clear()
        {
            map_.clear();
        }
    };
}
