#pragma once

#include <stdexcept>

namespace imp
{
    template<typename derived>
    struct instance_guard
    {
    private:
        static inline derived* instance_ = nullptr;

    public:
        instance_guard()
        {
            if (instance_)
                throw std::logic_error("existing active instance"); // TODO class name prefix + imp::error ?

            instance_ = static_cast<derived*>(this);
        }

        instance_guard(const instance_guard&) = delete;
        instance_guard(instance_guard&&) = delete;
        instance_guard& operator=(const instance_guard&) = delete;
        instance_guard& operator=(instance_guard&&) = delete;

        ~instance_guard()
        {
            if (instance_ == static_cast<derived*>(this))
                instance_ = nullptr;
        }

        static derived& instance()
        {
            if (instance_)
                return *instance_;
            else
                throw std::logic_error("no active instance"); // TODO class name prefix + imp::error ?
        }

        static derived* active_instance()
        {
            return instance_;
        }
    };
}
