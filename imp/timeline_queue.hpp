#pragma once

#include <optional>
#include <vector>

namespace imp
{
	template<typename ty>
	class timeline_queue
	{
		size_t _limit;
		std::vector<ty> _past;
		std::optional<ty> _present;
		std::vector<ty> _future;

	public:
		timeline_queue(size_t limit)
			: _limit(limit)
		{
		}

		void push(ty obj)
		{
			if (_present)
				_past.push_back(std::move(*_present));
			_present = std::move(obj);
			_future.clear();
			prune();
		}

		void move_backward()
		{
			if (_present)
			{
				_future.push_back(std::move(*_present));
				_present.reset();
			}

			if (!_past.empty())
			{
				_present = std::move(_past.back());
				_past.pop_back();
			}
		}

		void move_forward()
		{
			if (_present)
			{
				_past.push_back(std::move(*_present));
				_present.reset();
			}

			if (!_future.empty())
			{
				_present = std::move(_future.back());
				_future.pop_back();
			}
		}

		const ty* get_present() const
		{
			return _present ? &*_present : nullptr;
		}

		ty* get_present()
		{
			return _present ? &*_present : nullptr;
		}

		void set_limit(size_t limit)
		{
			_limit = limit;
			prune();
		}

		void prune()
		{
			if (_future.size() + 1 >= _limit)
				_past.clear();
			else
			{
				const size_t past_limit = _limit - _future.size() - 1;
				if (_past.size() > past_limit)
					_past.erase(_past.begin(), _past.end() - past_limit);
			}
		}

		void clear()
		{
			_past.clear();
			_present.reset();
			_future.clear();
		}

		bool empty_backwards() const
		{
			return _past.empty();
		}

		bool empty_forwards() const
		{
			return _future.empty();
		}
	};
}
