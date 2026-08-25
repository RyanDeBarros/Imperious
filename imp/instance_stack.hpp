#pragma once

#include <stack>
#include <stdexcept>

namespace imp
{
	template<typename derived>
	struct instance_stack
	{
	private:
		static inline std::stack<derived*> _instances;

	public:
		instance_stack()
		{
			_instances.push(static_cast<derived*>(this));
		}

		instance_stack(const instance_stack&) = delete;
		instance_stack(instance_stack&&) = delete;
		instance_stack& operator=(const instance_stack&) = delete;
		instance_stack& operator=(instance_stack&&) = delete;

		~instance_stack()
		{
			_instances.pop();
		}

		static derived& instance()
		{
			if (!_instances.empty())
				return *_instances.top();
			else
				throw std::logic_error("no active instance"); // TODO class name prefix + imp::error ?
		}

		static size_t instance_count()
		{
			return _instances.size();
		}
	};
}
