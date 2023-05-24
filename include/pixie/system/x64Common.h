#pragma once

inline int Low32(const __int64 Value)
{
	return (*(const LARGE_INTEGER *)&Value).LowPart;
}

inline int High32(const __int64 Value)
{
	return (*(const LARGE_INTEGER *)&Value).HighPart;
}
