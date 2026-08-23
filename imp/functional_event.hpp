#pragma once

#include <functional>
#include <unordered_map>
#include <stack>

namespace imp
{
	template<typename... args>
	class functional_event
	{
	public:
		class handle
		{
			functional_event<args...>* _event = nullptr;
			size_t _h = 0;

			friend class functional_event<args...>;

			handle(functional_event<args...>* event, size_t h) : _event(event), _h(h) {}

		public:
			handle() = default;

			handle(const handle&) = delete;
			handle(handle&& o) noexcept
				: _event(o._event), _h(o._h)
			{
				o._event = nullptr;

				if (_event)
				{
					auto it = _event->_listeners.find(_h);
					if (it != _event->_listeners.end())
						it->second.handle_ = this;
				}
			}

			~handle()
			{
				if (_event)
					_event->unsubscribe(*this);
			}

			handle& operator=(handle&& o) noexcept
			{
				if (this != &o)
				{
					if (_event)
						_event->unsubscribe(*this);

					_event = o._event;
					_h = o._h;

					o._event = nullptr;

					if (_event)
					{
						auto it = _event->_listeners.find(_h);
						if (it != _event->_listeners.end())
							it->second.handle_ = this;
					}
				}

				return *this;
			}
		};

		using callback = std::function<void(args...)>;

	private:
		struct listener
		{
			handle* handle_;
			callback callback_;

			listener(handle* handle, callback callback)
				: handle_(handle), callback_(std::move(callback))
			{
			}
		};

		std::unordered_map<size_t, listener> _listeners;
		std::stack<size_t> _free_handles;

	public:
		functional_event() = default;
		functional_event(const functional_event&) = delete;
		functional_event(functional_event&&) = delete;

		~functional_event()
		{
			clear();
		}

		handle subscribe(callback callback)
		{
			size_t h;
			if (_free_handles.empty())
				h = _listeners.size();
			else
			{
				h = _free_handles.top();
				_free_handles.pop();
			}

			handle handle(this, h);
			_listeners.emplace(h, listener(&handle, std::move(callback)));
			return handle;
		}

		bool unsubscribe(handle& handle)
		{
			if (this == handle._event)
			{
				auto it = _listeners.find(handle._h);
				if (it == _listeners.end())
					return false;

				_listeners.erase(it);
				_free_handles.push(handle._h);

				handle._event = nullptr;
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
				listener.handle_->_event = nullptr;

			_listeners.clear();

			while (!_free_handles.empty())
				_free_handles.pop();
		}
	};
}
