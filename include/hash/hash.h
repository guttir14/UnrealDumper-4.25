#pragma once
#include <types.h>

constexpr uint64 Prime = 1099511628211;
constexpr uint64 Basis = 14695981039346656037;

template< typename Type >
constexpr uint64 HashCompute(uint64 hash, const Type* const data, uint64 size) {
	const auto element = (uint64)(data[0]);
	return (size == 0) ? hash : HashCompute((hash * Prime) ^ element, data + 1, size - 1);
}

template< typename Type >
constexpr uint64 Hash(const Type* const data, uint64 size){
	return HashCompute(Basis, data, size);
}

#define HASH( Data ) \
	[ & ]() \
	{ \
		constexpr auto hash = Hash( Data, sizeof(Data) - 1 );	\
		return hash; \
	}()