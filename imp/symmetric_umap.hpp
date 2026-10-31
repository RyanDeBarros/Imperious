#pragma once

#include <optional>
#include <unordered_map>
#include <unordered_set>

namespace imp
{
    template<typename half_key_ty, typename value_ty>
    class symmetric_umap
    {
    public:
        struct pair
        {
            half_key_ty k1;
            half_key_ty k2;

            bool operator==(const pair& other) const
            {
                return (k1 == other.k1 && k2 == other.k2) || (k1 == other.k2 && k2 == other.k1);
            }
        };

        struct pair_hash
        {
            size_t operator()(const pair& pair) const
            {
                return std::hash<const void*>{}(pair.k1) ^ std::hash<const void*>{}(pair.k2);
            }
        };

        using map_ty = std::unordered_map<pair, value_ty, pair_hash>;

    private:
        map_ty map_;
        std::unordered_map<half_key_ty, std::unordered_set<half_key_ty>> lut_;

    public:
        void clear()
        {
            map_.clear();
            lut_.clear();
        }

        template<typename kty1, typename kty2>
            requires (std::convertible_to<kty1, half_key_ty> && std::convertible_to<kty2, half_key_ty>)
        std::optional<value_ty> get(kty1&& k1, kty2&& k2) const
        {
            return get(pair{ std::forward<kty1>(k1), std::forward<kty2>(k2) });
        }

        std::optional<value_ty> get(const pair& key) const
        {
            auto it = map_.find(key);
            if (it != map_.end())
                return it->second;
            else
                return std::nullopt;
        }

        template<typename kty1, typename kty2, typename vty>
            requires (std::convertible_to<kty1, half_key_ty>
                && std::convertible_to<kty2, half_key_ty>
                && std::convertible_to<vty, value_ty>)
        value_ty get_or(kty1&& k1, kty2&& k2, vty&& default_value)
        {
            return get_or(pair{ std::forward<kty1>(k1), std::forward<kty2>(k2) }, std::forward<vty>(default_value));
        }

        template<typename pty, typename vty>
            requires (std::convertible_to<pty, pair> && std::convertible_to<vty, value_ty>)
        value_ty get_or(pty&& key, vty&& default_value)
        {
            auto it = map_.find(key);
            if (it != map_.end())
                return it->second;
            else
            {
                set(std::forward<pty>(key), default_value);
                return std::forward<vty>(default_value);
            }
        }

        template<typename kty1, typename kty2, typename vty>
            requires (std::convertible_to<kty1, half_key_ty>
                && std::convertible_to<kty2, half_key_ty>
                && std::convertible_to<vty, value_ty>)
        void set(kty1&& k1, kty2&& k2, vty&& value)
        {
            set(pair{ std::forward<kty1>(k1), std::forward<kty2>(k2) }, std::forward<vty>(value));
        }

        template<typename pty, typename vty>
            requires (std::convertible_to<pty, pair>&& std::convertible_to<vty, value_ty>)
        void set(pty&& key, vty&& value)
        {
            lut_[key.k1].insert(key.k2);
            lut_[key.k2].insert(key.k1);
            map_insert(std::forward<pty>(key), std::forward<vty>(value));
        }

        void copy_all(const half_key_ty& from, const half_key_ty& to)
        {
            auto it = lut_.find(from);
            if (it != lut_.end())
            {
                auto& lut_og = it->second;
                auto& lut_copy = lut_[to];
                for (const half_key_ty& k : lut_og)
                {
                    lut_copy.insert(k);
                    map_insert(pair{ to, k }, map_.find({ from, k })->second);
                }
            }
        }

        void replace_all(const half_key_ty& at, const half_key_ty& with)
        {
            auto it = lut_.find(with);
            if (it != lut_.end())
            {
                auto& lut_og = it->second;
                auto& lut_copy = lut_[at];
                for (const half_key_ty& k : lut_og)
                {
                    lut_copy.insert(k);
                    map_insert(pair{ at, k }, map_.find({ with, k })->second);
                    map_.erase({ with, k });
                }
                lut_.erase(it);
            }
        }

        void erase_all(const half_key_ty& k)
        {
            auto it = lut_.find(k);
            if (it != lut_.end())
            {
                while (!it->second.empty())
                {
                    auto node = it->second.extract(it->second.begin());
                    map_.erase({ k, std::move(node.value()) });
                }
                lut_.erase(it);
            }
        }

    private:
        template<typename pty, typename vty>
            requires (std::convertible_to<pty, pair>&& std::convertible_to<vty, value_ty>)
        void map_insert(pty&& key, vty&& value)
        {
            if constexpr (std::is_copy_assignable_v<value_ty>)
                map_[std::forward<pty>(key)] = std::forward<vty>(value);
            else
            {
                auto er = map_.find(key);
                if (er != map_.end())
                    map_.erase(er);
                map_.emplace(std::forward<pty>(key), std::forward<vty>(value));
            }
        }
    };
}
