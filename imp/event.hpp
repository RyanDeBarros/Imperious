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
		void* _event = nullptr;
		size_t _h = 0;
		void (*_resubscribe)(event_listener&, event_listener*) = nullptr;

		template<typename... args>
		friend class event;

		template<typename... args>
		event_listener(event<args...>* event_, size_t h) : _event(event_), _h(h)
		{
			_resubscribe = [](event_listener& from, event_listener* to) {
				if (to)
				{
					to->_event = from._event;
					to->_h = from._h;
					to->_resubscribe = from._resubscribe;
				}

				if (auto e = static_cast<event<args...>*>(from._event))
				{
					if (to)
					{
						auto it = e->_listeners.find(from._h);
						if (it != e->_listeners.end())
							it->second.event_listener_ = to;

						from._event = nullptr;
						from._h = 0;
						from._resubscribe = nullptr;
					}
					else
						e->unsubscribe(from);
				}
			};
		}

	public:
		event_listener() = default;

		event_listener(const event_listener&) = delete;
		event_listener(event_listener&& o) noexcept
		{
			if (o._resubscribe)
				o._resubscribe(o, this);
		}

		~event_listener()
		{
			if (_resubscribe)
				_resubscribe(*this, nullptr);
		}

		event_listener& operator=(event_listener&& o) noexcept
		{
			if (this != &o)
			{
				if (_resubscribe)
					_resubscribe(*this, nullptr);

				if (o._resubscribe)
					o._resubscribe(o, this);
			}

			return *this;
		}
	};

	template<typename... args>
	class event
	{
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

		std::unordered_map<size_t, internal_listener> _listeners;
		std::stack<size_t> _free_event_listeners;

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
			if (_free_event_listeners.empty())
				h = _listeners.size();
			else
			{
				h = _free_event_listeners.top();
				_free_event_listeners.pop();
			}

			event_listener event_listener(this, h);
			_listeners.emplace(h, listener(&event_listener, std::move(callback)));
			return event_listener;
		}

		bool unsubscribe(event_listener& event_listener)
		{
			if (this == event_listener._event)
			{
				auto it = _listeners.find(event_listener._h);
				if (it == _listeners.end())
					return false;

				_listeners.erase(it);
				_free_event_listeners.push(event_listener._h);

				event_listener._event = nullptr;
				event_listener._h = 0;
				event_listener._resubscribe = nullptr;
				return true;
			}
			else
				return false;
		}

		void invoke(args... args_)
		{
			for (auto& [h, listener] : _listeners)
				listener.callback_(args_...);
		}

		void clear()
		{
			for (auto& [h, listener] : _listeners)
			{
				listener.event_listener_->_event = nullptr;
				listener.event_listener_->_h = 0;
				listener.event_listener_->_resubscribe = nullptr;
			}

			_listeners.clear();

			while (!_free_event_listeners.empty())
				_free_event_listeners.pop();
		}
	};
}
