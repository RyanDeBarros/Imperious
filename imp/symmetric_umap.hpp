#pragma once

#include <unordered_map>
#include <unordered_set>
#include <optional>

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

            bool operator==(const pair& other) const { return (k1 == other.k1 && k2 == other.k2) || (k1 == other.k2 && k2 == other.k1); }
        };

        struct pair_hash
        {
            size_t operator()(const pair& pair) const { return std::hash<const void*>{}(pair.k1) ^ std::hash<const void*>{}(pair.k2); }
        };

        using map_ty = std::unordered_map<pair, value_ty, pair_hash>;

    private:
        map_ty _map;
        std::unordered_map<half_key_ty, std::unordered_set<half_key_ty>> _lut;

        template<typename pty, typename vty>
            requires (std::convertible_to<pty, pair>&& std::convertible_to<vty, value_ty>)
        void map_insert(pty&& key, vty&& value)
        {
            if constexpr (std::is_copy_assignable_v<value_ty>)
                _map[std::forward<pty>(key)] = std::forward<vty>(value);
            else
            {
                auto er = _map.find(key);
                if (er != _map.end())
                    _map.erase(er);
                _map.emplace(std::forward<pty>(key), std::forward<vty>(value));
            }
        }

    public:
        void clear()
        {
            _map.clear();
            _lut.clear();
        }

        template<typename kty> requires (std::convertible_to<kty, half_key_ty>)
            std::optional<value_ty> get(kty&& k1, kty&& k2) const
        {
            return get(pair{ std::forward<kty>(k1), std::forward<kty>(k2) });
        }

        std::optional<value_ty> get(const pair& key) const
        {
            auto it = _map.find(key);
            if (it != _map.end())
                return it->second;
            else
                return std::nullopt;
        }

        template<typename kty, typename vty>
            requires (std::convertible_to<kty, half_key_ty>&& std::convertible_to<vty, value_ty>)
        value_ty get_or(kty&& k1, kty&& k2, vty&& default_value)
        {
            return get_or(pair{ std::forward<kty>(k1), std::forward<kty>(k2) }, std::forward<vty>(default_value));
        }

        template<typename pty, typename vty>
            requires (std::convertible_to<pty, pair>&& std::convertible_to<vty, value_ty>)
        value_ty get_or(pty&& key, vty&& default_value)
        {
            auto it = _map.find(key);
            if (it != _map.end())
                return it->second;
            else
            {
                set(std::forward<pty>(key), default_value);
                return std::forward<vty>(default_value);
            }
        }

        template<typename kty, typename vty>
            requires (std::convertible_to<kty, half_key_ty>&& std::convertible_to<vty, value_ty>)
        void set(kty&& k1, kty&& k2, vty&& value)
        {
            set(pair{ std::forward<kty>(k1), std::forward<kty>(k2) }, std::forward<vty>(value));
        }

        template<typename pty, typename vty>
            requires (std::convertible_to<pty, pair>&& std::convertible_to<vty, value_ty>)
        void set(pty&& key, vty&& value)
        {
            _lut[key.k1].insert(key.k2);
            _lut[key.k2].insert(key.k1);
            map_insert(std::forward<pty>(key), std::forward<vty>(value));
        }

        void copy_all(const half_key_ty& from, const half_key_ty& to)
        {
            auto it = _lut.find(from);
            if (it != _lut.end())
            {
                auto& lut_og = it->second;
                auto& lut_copy = _lut[to];
                for (const half_key_ty& k : lut_og)
                {
                    lut_copy.insert(k);
                    map_insert(pair{ to, k }, _map.find({ from, k })->second);
                }
            }
        }

        void replace_all(const half_key_ty& at, const half_key_ty& with)
        {
            auto it = _lut.find(with);
            if (it != _lut.end())
            {
                auto& lut_og = it->second;
                auto& lut_copy = _lut[at];
                for (const half_key_ty& k : lut_og)
                {
                    lut_copy.insert(k);
                    map_insert(pair{ at, k }, _map.find({ with, k })->second);
                    _map.erase({ with, k });
                }
                _lut.erase(it);
            }
        }

        void erase_all(const half_key_ty& k)
        {
            auto it = _lut.find(k);
            if (it != _lut.end())
            {
                while (!it->second.empty())
                {
                    auto node = it->second.extract(it->second.begin());
                    _map.erase({ k, std::move(node.value()) });
                }
                _lut.erase(it);
            }
        }
    };
}
