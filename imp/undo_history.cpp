#include "undo_history.hpp"

#include <stack>

namespace imp
{
	undo_history::undo_history(size_t count_limit, size_t size_limit)
		: _count_limit(count_limit), _size_limit(size_limit)
	{
	}

	undo_history& undo_history::active_instance()
	{
		return active_undo_history::instance().uh_instance;
	}

	void undo_history::execute(std::unique_ptr<undo_action>&& action)
	{
		if (action->forward())
			push(std::move(action));
		else
			clear();
	}

	void undo_history::push(std::unique_ptr<undo_action>&& action)
	{
		_undo_stack_size += action->empirical_size();
		_redo_stack_size = 0;

		_undo.push_back(std::move(action));
		_redo.clear();

		prune();
	}

	void undo_history::undo()
	{
		if (!_undo.empty())
		{
			const size_t sz = _undo.back()->empirical_size();
			_undo_stack_size -= sz;
			_redo_stack_size += sz;

			std::unique_ptr<undo_action> action = std::move(_undo.back());
			_undo.pop_back();

			if (action->backward())
			{
				_redo.push_back(std::move(action));
				on_undo_post_success();
			}
			else
			{
				on_undo_pre_fail();
				_undo_stack_size = 0;
				_undo.clear();
			}

			prune();
		}
	}

	void undo_history::redo()
	{
		if (!_redo.empty())
		{
			const size_t sz = _redo.back()->empirical_size();
			_redo_stack_size -= sz;
			_undo_stack_size += sz;

			std::unique_ptr<undo_action> action = std::move(_redo.back());
			_redo.pop_back();

			if (action->forward())
			{
				_undo.push_back(std::move(action));
				on_redo_post_success();
			}
			else
			{
				on_redo_pre_fail();
				_redo_stack_size = 0;
				_redo.clear();
			}

			prune();
		}
	}

	void undo_history::prune()
	{
		if (_redo.size() >= _count_limit)
			prune_undo_count(0);
		else
			prune_undo_count(_count_limit - _redo.size());

		if (_redo_stack_size >= _size_limit)
			prune_undo_size(0);
		else
			prune_undo_size(_size_limit - _redo_stack_size);
	}

	void undo_history::clear()
	{
		_undo_stack_size = 0;
		_undo.clear();

		_redo_stack_size = 0;
		_redo.clear();
	}

	void undo_history::set_limits(size_t count_limit, size_t size_limit)
	{
		_count_limit = count_limit;
		_size_limit = size_limit;
		prune();
	}

	void undo_history::prune_undo_count(size_t count_limit)
	{
		if (count_limit == 0)
		{
			_undo_stack_size = 0;
			_undo.clear();
		}
		else
		{
			if (_undo.size() > count_limit)
			{
				const size_t amount = _undo.size() - count_limit;

				for (size_t i = 0; i < amount; ++i)
					_undo_stack_size -= _undo[i]->empirical_size();

				_undo.erase(_undo.begin(), _undo.begin() + amount);
			}
		}
	}

	void undo_history::prune_undo_size(size_t size_limit)
	{
		if (size_limit == 0)
		{
			_undo_stack_size = 0;
			_undo.clear();
		}
		else
		{
			auto it = _undo.begin();
			while (_undo_stack_size > size_limit && it != _undo.end())
				_undo_stack_size -= (*it++)->empirical_size();

			if (it != _undo.begin())
				_undo.erase(_undo.begin(), it);
		}
	}

	size_t undo_history::undo_count() const
	{
		return _undo.size();
	}

	active_undo_history::active_undo_history(undo_history& undo_history)
		: uh_instance(undo_history)
	{
	}

	void checkpoint_undo_history::push(std::unique_ptr<undo_action>&& action)
	{
		if (_clean_marker && *_clean_marker > undo_count())
			_clean_marker.reset();

		undo_history::push(std::move(action));
	}

	void checkpoint_undo_history::prune()
	{
		const size_t initial_undo_count = undo_count();

		undo_history::prune();

		if (_clean_marker)
		{
			size_t delta = initial_undo_count - undo_count();

			if (*_clean_marker >= delta)
				_clean_marker = *_clean_marker - delta;
			else
				_clean_marker.reset();
		}
	}

	void checkpoint_undo_history::clear()
	{
		undo_history::clear();

		if (_clean_marker && *_clean_marker > 0)
			_clean_marker.reset();
	}

	void checkpoint_undo_history::mark_clean()
	{
		_clean_marker = undo_count();
	}

	void checkpoint_undo_history::on_undo_post_success()
	{
		if (_clean_marker && (undo_count() == *_clean_marker || undo_count() + 1 == *_clean_marker))
			on_potential_clean.invoke();
	}

	void checkpoint_undo_history::on_undo_pre_fail()
	{
		if (_clean_marker)
		{
			if (*_clean_marker <= undo_count())
				_clean_marker.reset();
			else
				_clean_marker = *_clean_marker - (undo_count() + 1);
		}
	}

	void checkpoint_undo_history::on_redo_post_success()
	{
		if (_clean_marker && (undo_count() == *_clean_marker || undo_count() == *_clean_marker + 1))
			on_potential_clean.invoke();
	}

	void checkpoint_undo_history::on_redo_pre_fail()
	{
		if (_clean_marker && *_clean_marker > undo_count())
			_clean_marker.reset();
	}
}
