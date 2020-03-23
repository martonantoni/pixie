#pragma once

#undef GetFreeSpace

#define ARRAY_COUNT(array) (sizeof(array)/sizeof((array)[0]))

#define SAFEDEL(x) do { delete x; x=nullptr; } while(false)
#define SAFEMULTIDEL(x) do { delete[] x; x=nullptr; } while(false)

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

template <class T> inline const T &Min(const T &a,const T &b) { return a<b?a:b; }
template <class T> inline const T &Max(const T &a,const T &b) {	return a>b?a:b; }
template <class T> inline const T Abs(const T &a) { return a>=0?a:-a; }

#define EPSILON 1.0e-10
