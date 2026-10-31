#pragma once

#include <unordered_map>
#include <vector>

namespace imp
{
    template<
        typename ty,
        typename hash = std::hash<ty>,
        typename equals = std::equal_to<ty>
    >
    class counter
    {
        std::unordered_map<ty, size_t, hash, equals> map_;

    public:
        void increment(const ty& obj, size_t by = 1)
        {
            auto it = map_.find(obj);
            if (it != map_.end())
                it->second += by;
            else if (by > 0)
                map_.emplace(obj, by);
        }

        void increment(ty&& obj, size_t by = 1)
        {
            auto it = map_.find(obj);
            if (it != map_.end())
                it->second += by;
            else if (by > 0)
                map_.emplace(std::move(obj), by);
        }

        template<typename U = ty>
        bool decrement(const U& obj)
        {
            auto it = map_.find(obj);
            if (it != map_.end())
            {
                if (--it->second == 0)
                    map_.erase(it);
                return true;
            }
            else
                return false;
        }

        size_t count(const ty& obj) const
        {
            auto it = map_.find(obj);
            return it != map_.end() ? it->second : 0;
        }

        bool contains(const ty& obj) const
        {
            return count(obj) > 0;
        }

        void clear()
        {
            map_.clear();
        }

        void accumulate(const std::vector<ty>& vec)
        {
            for (const ty& el : vec)
                increment(el);
        }

        void accumulate(const counter<ty, hash, equals>& other)
        {
            for (auto it = other.map_.begin(); it != other.map_.end(); ++it)
                increment(it->first, it->second);
        }

        void accumulate(counter<ty, hash, equals>&& other)
        {
            const auto begin = std::make_move_iterator(other.map_.begin());
            const auto end = std::make_move_iterator(other.map_.end());

            for (auto it = begin; it != end; ++it)
                increment(std::move(it->first), it->second);
        }
    };
}
