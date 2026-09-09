#pragma once

#include <unordered_set>

namespace imp
{
	template<typename derived>
	class instance_tracker
	{
		inline static std::unordered_set<derived*> _instances = {};

	public:
		instance_tracker()
		{
			_instances.insert(static_cast<derived*>(this));
		}

		instance_tracker(const instance_tracker&)
		{
			_instances.insert(static_cast<derived*>(this));
		}

		instance_tracker(instance_tracker&&)
		{
			_instances.insert(static_cast<derived*>(this));
		}

		~instance_tracker()
		{
			_instances.erase(static_cast<derived*>(this));
		}

		static const std::unordered_set<derived*>& instances()
		{
			return _instances;
		}
	};
}
