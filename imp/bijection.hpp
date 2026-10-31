#pragma once

#include <unordered_map>

namespace imp
{
    template<
        typename left_ty,
        typename right_ty,
        typename left_hash = std::hash<left_ty>,
        typename right_hash = std::hash<right_ty>,
        typename left_equals = std::equal_to<left_ty>,
        typename right_equals = std::equal_to<right_ty>
    >
    class bijection
    {
        std::unordered_map<left_ty, right_ty, left_hash, left_equals> lhs_map_;
        std::unordered_map<right_ty, left_ty, right_hash, right_equals> rhs_map_;

    public:
        bijection() = default;
        bijection(const bijection&) = default;
        bijection(bijection&&) noexcept = default;
        bijection& operator=(const bijection&) = default;
        bijection& operator=(bijection&&) noexcept = default;

    public:
        void set(const left_ty& lhs, const right_ty& rhs)
        {
            auto forward_it = lhs_map_.find(lhs);
            if (forward_it != lhs_map_.end())
            {
                if (right_equals{}(forward_it->second, rhs))
                    return;
                rhs_map_.erase(forward_it->second);
            }

            auto backward_it = rhs_map_.find(rhs);
            if (backward_it != rhs_map_.end())
            {
                if (left_equals{}(backward_it->second, lhs))
                    return;
                lhs_map_.erase(backward_it->second);
            }

            map_insert(lhs_map_, lhs, rhs);
            map_insert(rhs_map_, rhs, lhs);
        }

        void left_erase(const left_ty& lhs)
        {
            auto it = lhs_map_.find(lhs);
            if (it != lhs_map_.end())
            {
                rhs_map_.erase(it->second);
                lhs_map_.erase(it);
            }
        }

        void left_erase(const typename decltype(lhs_map_)::const_iterator& where)
        {
            if (where != lhs_map_.end())
            {
                rhs_map_.erase(where->second);
                lhs_map_.erase(where);
            }
        }

        void right_erase(const right_ty& rhs)
        {
            auto it = rhs_map_.find(rhs);
            if (it != rhs_map_.end())
            {
                lhs_map_.erase(it->second);
                rhs_map_.erase(it);
            }
        }

        void right_erase(const typename decltype(rhs_map_)::const_iterator& where)
        {
            if (where != rhs_map_.end())
            {
                lhs_map_.erase(where->second);
                rhs_map_.erase(where);
            }
        }

        void clear()
        {
            lhs_map_.clear();
            rhs_map_.clear();
        }

        bool left_exists(const left_ty& lhs) const
        {
            return lhs_map_.count(lhs);
        }

        bool right_exists(const right_ty& rhs) const
        {
            return rhs_map_.count(rhs);
        }

        auto left_begin() const
        {
            return lhs_map_.begin();
        }

        auto right_begin() const
        {
            return rhs_map_.begin();
        }

        auto left_end() const
        {
            return lhs_map_.end();
        }

        auto right_end() const
        {
            return rhs_map_.end();
        }

        auto left_find(const left_ty& lhs) const
        {
            return lhs_map_.find(lhs);
        }

        auto right_find(const right_ty& rhs) const
        {
            return rhs_map_.find(rhs);
        }

        const right_ty* left_get(const left_ty& lhs) const
        {
            auto it = lhs_map_.find(lhs);
            return it != lhs_map_.end() ? &it->second : nullptr;
        }

        const left_ty* right_get(const right_ty& rhs) const
        {
            auto it = rhs_map_.find(rhs);
            return it != rhs_map_.end() ? &it->second : nullptr;
        }

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
    };
}
