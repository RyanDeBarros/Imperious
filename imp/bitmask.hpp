#pragma once

#define IMP_IMPLEMENT_BITMASK(enum_ty) \
inline enum_ty operator|(enum_ty lhs, enum_ty rhs) { return static_cast<enum_ty>(static_cast<int>(lhs) | static_cast<int>(rhs)); } \
inline enum_ty operator&(enum_ty lhs, enum_ty rhs) { return static_cast<enum_ty>(static_cast<int>(lhs) & static_cast<int>(rhs)); } \
inline enum_ty operator^(enum_ty lhs, enum_ty rhs) { return static_cast<enum_ty>(static_cast<int>(lhs) ^ static_cast<int>(rhs)); } \
inline enum_ty operator~(enum_ty value) {	return static_cast<enum_ty>(~static_cast<int>(value)); } \
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
