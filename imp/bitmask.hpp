#pragma once

#include <concepts>

#define IMP_IMPLEMENT_BITMASK(enum_ty) \
	inline enum_ty operator|(enum_ty lhs, enum_ty rhs) \
	{ \
		using sub_ty = std::underlying_type_t<enum_ty>; \
		return static_cast<enum_ty>(static_cast<sub_ty>(lhs) | static_cast<sub_ty>(rhs)); \
	} \
	inline enum_ty operator&(enum_ty lhs, enum_ty rhs) \
	{ \
		using sub_ty = std::underlying_type_t<enum_ty>; \
		return static_cast<enum_ty>(static_cast<sub_ty>(lhs) & static_cast<sub_ty>(rhs)); \
	} \
	inline enum_ty operator^(enum_ty lhs, enum_ty rhs) \
	{ \
		using sub_ty = std::underlying_type_t<enum_ty>; \
		return static_cast<enum_ty>(static_cast<sub_ty>(lhs) ^ static_cast<sub_ty>(rhs)); \
	} \
	inline enum_ty operator~(enum_ty value) \
	{ \
		using sub_ty = std::underlying_type_t<enum_ty>; \
		return static_cast<enum_ty>(~static_cast<sub_ty>(value)); \
	} \
	inline enum_ty& operator|=(enum_ty& lhs, enum_ty rhs) { lhs = lhs | rhs; return lhs; } \
	inline enum_ty& operator&=(enum_ty& lhs, enum_ty rhs) { lhs = lhs & rhs; return lhs; } \
	inline enum_ty& operator^=(enum_ty& lhs, enum_ty rhs) { lhs = lhs ^ rhs; return lhs; }

#define IMP_DECLARE_NESTED_BITMASK(enum_ty) \
	friend enum_ty operator|(enum_ty, enum_ty); \
	friend enum_ty operator&(enum_ty, enum_ty); \
	friend enum_ty operator^(enum_ty, enum_ty); \
	friend enum_ty operator~(enum_ty); \
	friend enum_ty& operator|=(enum_ty&, enum_ty); \
	friend enum_ty& operator&=(enum_ty&, enum_ty); \
	friend enum_ty& operator^=(enum_ty&, enum_ty);
