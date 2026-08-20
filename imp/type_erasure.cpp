#include "type_erasure.hpp"

#include <unordered_map>

namespace imp
{
	static std::unordered_map<std::type_index, size_t> type_erasures;

	static size_t get_index(std::type_index ti)
	{
		auto it = type_erasures.find(ti);
		if (it != type_erasures.end())
			return it->second;
		else
			return type_erasures.emplace(ti, type_erasures.size() + 1).first->second;
	}
	
	type_erasure::type_erasure()
		: _index(get_index(typeid(void)))
	{
	}

	type_erasure::type_erasure(std::type_index ti)
		: _index(get_index(ti))
	{
	}

	std::string type_erasure::repr() const
	{
		return std::to_string(_index);
	}

	bool type_erasure::operator==(std::type_index ti) const
	{
		return get_index(ti) == _index;
	}

	bool type_erasure::operator!=(std::type_index ti) const
	{
		return get_index(ti) != _index;
	}
}
