#pragma once

#include <functional>
#include <unordered_map>
#include <stack>

namespace imp
{
    template<typename... args>
    class event;

    class event_listener
    {
        template<typename... args>
        friend class event;

    private:
        void* event_ = nullptr;
        size_t h_ = 0;
        void (*resubscribe_)(event_listener&, event_listener*) = nullptr;

    private:
        template<typename... args>
        event_listener(event<args...>* e, size_t h)
            : event_(e)
            , h_(h)
        {
            resubscribe_ = &resubscribe<args...>;
        }

    public:
        event_listener() = default;

        event_listener(const event_listener&) = delete;
        event_listener(event_listener&& o) noexcept
        {
            if (o.resubscribe_)
                o.resubscribe_(o, this);
        }

        ~event_listener()
        {
            detach();
        }

        event_listener& operator=(event_listener&& o) noexcept
        {
            if (this != &o)
            {
                detach();
                if (o.resubscribe_)
                    o.resubscribe_(o, this);
            }

            return *this;
        }

        void detach()
        {
            if (resubscribe_)
                resubscribe_(*this, nullptr);
        }

    private:
        template<typename... args>
        static void resubscribe(event_listener& from, event_listener* to)
        {
            if (to)
            {
                to->event_ = from.event_;
                to->h_ = from.h_;
                to->resubscribe_ = from.resubscribe_;
            }

            if (auto e = static_cast<event<args...>*>(from.event_))
            {
                if (to)
                {
                    auto it = e->listeners_.find(from.h_);
                    if (it != e->listeners_.end())
                        it->second.event_listener_ = to;

                    from.event_ = nullptr;
                    from.h_ = 0;
                    from.resubscribe_ = nullptr;
                }
                else
                    e->unsubscribe(from);
            }
        }
    };

    template<typename... args>
    class event
    {
        friend class event_listener;

    public:
        using callback = std::function<void(args...)>;

    private:
        struct internal_listener
        {
            event_listener* event_listener_;
            callback callback_;

            internal_listener(event_listener* event_listener, callback callback)
                : event_listener_(event_listener), callback_(std::move(callback))
            {
            }
        };

    private:
        std::unordered_map<size_t, internal_listener> listeners_;
        std::stack<size_t> freed_listeners_;

    public:
        event() = default;
        event(const event&) = delete;
        event(event&&) = delete;

        ~event()
        {
            clear();
        }

        event_listener subscribe(callback callback)
        {
            size_t h;
            if (freed_listeners_.empty())
                h = listeners_.size();
            else
            {
                h = freed_listeners_.top();
                freed_listeners_.pop();
            }

            event_listener event_listener(this, h);
            listeners_.emplace(h, internal_listener(&event_listener, std::move(callback)));
            return event_listener;
        }

        bool unsubscribe(event_listener& listener)
        {
            if (this == listener.event_)
            {
                auto it = listeners_.find(listener.h_);
                if (it == listeners_.end())
                    return false;

                listeners_.erase(it);
                freed_listeners_.push(listener.h_);

                listener.event_ = nullptr;
                listener.h_ = 0;
                listener.resubscribe_ = nullptr;
                return true;
            }
            else
                return false;
        }

        void invoke(args... args_)
        {
            for (auto& [h, listener] : listeners_)
                listener.callback_(args_...);
        }

        void clear()
        {
            for (auto& [h, listener] : listeners_)
            {
                listener.event_listener_->event_ = nullptr;
                listener.event_listener_->h_ = 0;
                listener.event_listener_->resubscribe_ = nullptr;
            }

            listeners_.clear();

            while (!freed_listeners_.empty())
                freed_listeners_.pop();
        }
    };
}
