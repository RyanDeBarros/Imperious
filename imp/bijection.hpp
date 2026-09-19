#pragma once

#include <unordered_map>

namespace imp
{
    template<
        typename left_ty,
        typename right_ty,
        typename left_hash    = std::hash<left_ty>,
        typename right_hash   = std::hash<right_ty>,
        typename left_equals  = std::equal_to<left_ty>,
        typename right_equals = std::equal_to<right_ty>
    >
    class bijection
    {
        std::unordered_map<left_ty, right_ty, left_hash, left_equals>   _lhs_map;
        std::unordered_map<right_ty, left_ty, right_hash, right_equals> _rhs_map;

    public:
        bijection() = default;
        bijection(const bijection&) = default;
        bijection(bijection&&) noexcept = default;
        bijection& operator=(const bijection&) = default;
        bijection& operator=(bijection&&) noexcept = default;

    private:
        template<typename map_ty, typename key_ty, typename value_ty>
        static void map_insert(map_ty& map, key_ty&& key, value_ty&& value)
        {
            if constexpr (std::is_copy_assignable_v<value_ty>)
                map[std::forward<key_ty>(key)] = std::forward<value_ty>(value);
            else
            {
                auto it = map.find(key);
                if (it != map.end())
                    map.erase(it);
                map.emplace(std::forward<key_ty>(key), std::forward<value_ty>(value));
            }
        }

    public:
        void set(const left_ty& lhs, const right_ty& rhs)
        {
            auto forward_it = _lhs_map.find(lhs);
            if (forward_it != _lhs_map.end())
            {
                if (right_equals{}(forward_it->second, rhs))
                    return;
                _rhs_map.erase(forward_it->second);
            }

            auto backward_it = _rhs_map.find(rhs);
            if (backward_it != _rhs_map.end())
            {
                if (left_equals{}(backward_it->second, lhs))
                    return;
                _lhs_map.erase(backward_it->second);
            }

            map_insert(_lhs_map, lhs, rhs);
            map_insert(_rhs_map, rhs, lhs);
        }

        void left_erase(const left_ty& lhs)
        {
            auto it = _lhs_map.find(lhs);
            if (it != _lhs_map.end())
            {
                _rhs_map.erase(it->second);
                _lhs_map.erase(it);
            }
        }

        void left_erase(const typename decltype(_lhs_map)::const_iterator& where)
        {
            if (where != _lhs_map.end())
            {
                _rhs_map.erase(where->second);
                _lhs_map.erase(where);
            }
        }

        void right_erase(const right_ty& rhs)
        {
            auto it = _rhs_map.find(rhs);
            if (it != _rhs_map.end())
            {
                _lhs_map.erase(it->second);
                _rhs_map.erase(it);
            }
        }

        void right_erase(const typename decltype(_rhs_map)::const_iterator& where)
        {
            if (where != _rhs_map.end())
            {
                _lhs_map.erase(where->second);
                _rhs_map.erase(where);
            }
        }

        void clear()
        {
            _lhs_map.clear();
            _rhs_map.clear();
        }

        bool left_exists(const left_ty& lhs) const
        {
            return _lhs_map.count(lhs);
        }

        bool right_exists(const right_ty& rhs) const
        {
            return _rhs_map.count(rhs);
        }

        auto left_begin() const
        {
            return _lhs_map.begin();
        }

        auto right_begin() const
        {
            return _rhs_map.begin();
        }

        auto left_end() const
        {
            return _lhs_map.end();
        }

        auto right_end() const
        {
            return _rhs_map.end();
        }

        auto left_find(const left_ty& lhs) const
        {
            return _lhs_map.find(lhs);
        }

        auto right_find(const right_ty& rhs) const
        {
            return _rhs_map.find(rhs);
        }

        const right_ty* left_get(const left_ty& lhs) const
        {
            auto it = _lhs_map.find(lhs);
            return it != _lhs_map.end() ? &it->second : nullptr;
        }

        const left_ty* right_get(const right_ty& rhs) const
        {
            auto it = _rhs_map.find(rhs);
            return it != _rhs_map.end() ? &it->second : nullptr;
        }
    };
}
