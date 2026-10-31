#pragma once

#include "imp/hash.hpp"

#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace imp
{
    template<typename ty>
    struct internship_handle
    {
        size_t id = 0;

        internship_handle() = default;
        explicit internship_handle(size_t id)
            : id(id)
        {
        }

        bool operator==(const internship_handle&) const = default;
        bool operator!=(const internship_handle&) const = default;

        bool valid() const
        {
            return id > 0;
        }
    };

    template<typename ty>
    class internship
    {
    public:
        using handle = internship_handle<ty>;

    private:
        std::vector<ty> data_;
        std::unordered_multimap<size_t, handle> lut_;

    public:
        template<
            typename view_ty,
            typename view_hash = std::hash<view_ty>,
            typename view_equals = std::equal_to<view_ty>,
            typename converter = void
        >
        handle intern(const view_ty& view)
        {
            size_t hash = hasher().with_hash<view_hash>(view);
            auto range = lut_.equal_range(hash);

            for (auto it = range.first; it != range.second; ++it)
            {
                auto h = it->second;
                if (view_equals{}(get(h), view))
                    return h;
            }

            handle h = handle(data_.size() + 1);

            if constexpr (std::is_same_v<converter, void>)
                data_.emplace_back(view);
            else
                data_.emplace_back(converter{}(view));

            lut_.emplace(hash, h);
            return h;
        }

        const ty& get(const handle h) const
        {
            if (auto ptr = try_get(h))
                return *ptr;
            else
                throw std::logic_error("Invalid handle"); // TODO imp::error ?
        }

        const ty* try_get(const handle h) const
        {
            if (h.id > 0 && h.id <= data_.size())
                return data_.data() + h.id - 1;
            else
                return nullptr;
        }
    };
}

template<typename ty>
struct std::hash<imp::internship_handle<ty>>
{
    size_t operator()(const imp::internship_handle<ty>& handle) const
    {
        return handle.id;
    }
};
