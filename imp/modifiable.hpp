#pragma once

#include <utility>

namespace imp
{
    template<typename ty>
    class modifiable
    {
        bool dirty_ = false;
        ty obj_;

    public:
        modifiable(ty ini = ty())
            : obj_(ini)
        {
        }

        modifiable(const modifiable& o)
        {
            set(o.obj_);
        }

        modifiable(modifiable&& o) noexcept
            : dirty_(o.dirty_)
            , obj_(std::move(o.obj_))
        {
        }

        modifiable& operator=(const modifiable& o)
        {
            if (this != &o)
                set(o.obj_);

            return *this;
        }

        modifiable& operator=(modifiable&& o)
        {
            if (this != &o)
            {
                dirty_ = o.dirty_;
                obj_ = std::move(o.obj_);
            }

            return *this;
        }

        modifiable& operator=(ty val)
        {
            set(val);
            return *this;
        }

        operator ty() const
        {
            return get();
        }

        ty get() const
        {
            return obj_;
        }

        void set(ty val)
        {
            if (obj_ != val)
            {
                obj_ = val;
                dirty_ = true;
            }
        }

        bool modified()
        {
            return dirty_;
        }

        bool consume_modified()
        {
            if (dirty_)
            {
                dirty_ = false;
                return true;
            }
            else
                return false;
        }
    };
}
