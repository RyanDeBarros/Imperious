#pragma once

#include <stack>
#include <stdexcept>

namespace imp
{
    template<typename derived>
    struct instance_stack
    {
    private:
        static inline std::stack<derived*> instances_;

    public:
        instance_stack()
        {
            instances_.push(static_cast<derived*>(this));
        }

        instance_stack(const instance_stack&) = delete;
        instance_stack(instance_stack&&) = delete;
        instance_stack& operator=(const instance_stack&) = delete;
        instance_stack& operator=(instance_stack&&) = delete;

        ~instance_stack()
        {
            instances_.pop();
        }

        static derived& instance()
        {
            if (!instances_.empty())
                return *instances_.top();
            else
                throw std::logic_error("no active instance"); // TODO class name prefix + imp::error ?
        }

        static size_t instance_count()
        {
            return instances_.size();
        }
    };
}
