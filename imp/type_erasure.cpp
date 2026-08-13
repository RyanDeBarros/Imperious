#include "type_erasure.hpp"

#include <unordered_map>

namespace imp
{
	static std::unordered_map<std::type_index, type_erasure> type_erasures;

	type_erasure erase_type(std::type_index ti)
	{
		auto it = type_erasures.find(ti);
		if (it != type_erasures.end())
			return it->second;
		else
			return type_erasures.emplace(ti, type_erasures.size() + 1).first->second;
	}
}
