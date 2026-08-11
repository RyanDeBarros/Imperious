#include "hash.h"

namespace imp
{
	void hasher::hash_combine(size_t& hash, size_t with)
	{
		hash ^= with + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2);
	}
}
