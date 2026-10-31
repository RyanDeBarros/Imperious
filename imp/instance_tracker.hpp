#pragma once

#include <unordered_set>

namespace imp
{
    template<typename derived>
    class instance_tracker
    {
        inline static std::unordered_set<derived*> instances_ = {};

    public:
        instance_tracker()
        {
            instances_.insert(static_cast<derived*>(this));
        }

        instance_tracker(const instance_tracker&)
        {
            instances_.insert(static_cast<derived*>(this));
        }

        instance_tracker(instance_tracker&&)
        {
            instances_.insert(static_cast<derived*>(this));
        }

        ~instance_tracker()
        {
            instances_.erase(static_cast<derived*>(this));
        }

        static const std::unordered_set<derived*>& instances()
        {
            return instances_;
        }
    };
}
