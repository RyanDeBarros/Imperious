#pragma once

#include <imp/soft_singleton.hpp>

#include <unordered_set>

namespace imp
{
    template<typename ty>
    class auto_trackable;

    template<typename ty>
    class auto_tracker final
        : public soft_singleton<auto_tracker<ty>>
    {
        friend class soft_singleton<auto_tracker<ty>>;
        friend class auto_trackable<ty>;

    private:
        std::unordered_set<ty*> tracked_;

    public:
        void clear()
        {
            tracked_.clear();
        }

        const std::unordered_set<ty*>& tracked() const
        {
            return tracked_;
        }
    };

    template<typename ty>
    class auto_trackable
    {
    public:
        auto_trackable()
        {
            tracker().tracked_.insert(static_cast<ty*>(this));
        }

        auto_trackable(const auto_trackable&)
        {
            tracker().tracked_.insert(static_cast<ty*>(this));
        }

        auto_trackable(auto_trackable&&) noexcept
        {
            tracker().tracked_.insert(static_cast<ty*>(this));
        }

        ~auto_trackable()
        {
            tracker().tracked_.erase(static_cast<ty*>(this));
        }

        static auto_tracker<ty>& tracker()
        {
            return auto_tracker<ty>::instance();
        }

        static const std::unordered_set<ty*>& tracked()
        {
            return tracker().tracked();
        }
    };
}
