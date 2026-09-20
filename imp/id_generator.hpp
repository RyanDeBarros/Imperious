#pragma once

#include <imp/issuer.hpp>

#include <limits>
#include <stack>
#include <stdexcept>
#include <string>

namespace imp
{
    template<std::unsigned_integral ty>
    class soft_id_generator
    {
        ty _next;
        ty _max;
        std::stack<ty> _yielded;

    public:
        soft_id_generator(ty initial = ty(0), ty _max = std::numeric_limits<ty>::max())
            : _next(initial), _max(_max)
        {
        }

        ty gen()
        {
            if (_yielded.empty())
            {
                if (_next > _max)
                    throw std::out_of_range(std::to_string(_next) + " > " + std::to_string(_max)); // TODO imp error ?
                return _next++;
            }

            ty _id = _yielded.top();
            _yielded.pop();
            return _id;
        }

        void yield(ty _id)
        {
            _yielded.push(_id);
        }
    };

    template<typename ty>
    class strict_id;

    namespace internal
    {
        template<std::unsigned_integral ty>
        class strict_id_generator : public issuer<strict_id_generator<ty>>
        {
            using super = issuer<strict_id_generator<ty>>;
            friend class strict_id<ty>;

            ty _next;
            ty _max;
            std::stack<ty> _yielded;

        public:
            strict_id_generator(ty initial, ty _max)
                : _next(initial), _max(_max)
            {
            }

            strict_id_generator(const strict_id_generator&) = delete;
            strict_id_generator(strict_id_generator&&) noexcept = default;
            strict_id_generator& operator=(strict_id_generator&&) noexcept = default;

            strict_id<ty> generate() { return super::issue(); }
        };
    }

    template<typename ty>
    class strict_id : public ticket<internal::strict_id_generator<ty>>
    {
        using super = ticket<internal::strict_id_generator<ty>>;
        friend class internal::strict_id_generator<ty>;

        ty _id = ty(-1);

        void init()
        {
            auto accessor = this->lock();
            if (internal::strict_id_generator<ty>* generator = accessor.get())
            {
                if (generator->_yielded.empty())
                {
                    if (generator->_next > generator->_max)
                        throw std::out_of_range(std::to_string(generator->_next) + " > " + std::to_string(generator->_max)); // TODO imp error ?
                    _id = generator->_next++;
                }
                else
                {
                    _id = generator->_yielded.top();
                    generator->_yielded.pop();
                }
            }
        }

        void del()
        {
            auto accessor = super::lock();
            if (internal::strict_id_generator<ty>* generator = accessor.get())
                generator->_yielded.push(_id);
            super::reset();
            _id = ty(-1);
        }

        strict_id(super&& o) noexcept
            : super(std::move(o))
        {
            init();
        }

    public:
        strict_id() = default;

        strict_id(const strict_id& o)
            : super(o)
        {
            init();
        }

        strict_id(strict_id&& o) noexcept
            : super(std::move(o)), _id(o._id)
        {
            o._id = ty(-1);
        }

        strict_id& operator=(const strict_id& o)
        {
            if (this != &o)
            {
                del();
                super::operator=(o);
                init();
            }
        }

        strict_id& operator=(strict_id&& o) noexcept
        {
            if (this != &o)
            {
                del();
                super::operator=(std::move(o));
                _id = o._id;
                o._id = ty(-1);
            }
            return *this;
        }

        ~strict_id()
        {
            del();
        }

        ty get() const
        {
            if (super::is_valid())
                return _id;
            else
                throw std::logic_error("invalid id"); // TODO imp error ?
        }

        void yield()
        {
            del();
        }
    };

    template<std::unsigned_integral ty>
    using strict_id_generator = issuer_instance<internal::strict_id_generator<ty>>;
}
