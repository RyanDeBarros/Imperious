#include "type_erasure.hpp"

#include <unordered_map>

namespace imp
{
	static std::unordered_map<std::type_index, size_t> type_erasures;

	type_erasure::type_erasure(std::type_index ti)
	{
		auto it = type_erasures.find(ti);
		if (it != type_erasures.end())
			_index = it->second;
		else
			_index = type_erasures.emplace(ti, type_erasures.size() + 1).first->second;
	}
}
