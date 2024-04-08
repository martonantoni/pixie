#pragma once

#undef GetFreeSpace

inline unsigned int NumberOfSetBits(unsigned int i)
{
	i = i - ((i >> 1) & 0x55555555u);
	i = (i & 0x33333333u) + ((i >> 2) & 0x33333333u);
	return (((i + (i >> 4)) & 0x0F0F0F0Fu) * 0x01010101u) >> 24;
}

template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, bool>::type IsPowerOfTwo(T x)
{
	return ((x != 0) && ((x & (~x + 1)) == x));
}

#define EPSILON 1.0e-10
