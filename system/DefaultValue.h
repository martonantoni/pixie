#pragma once

template<class T> class tDefaultValue
{
	T Value;
	int Set=false;
public:
	tDefaultValue()=default;
	tDefaultValue(const T &pValue): Set(true), Value(pValue) {}
	int IsSet() const { return Set; }
	const T &GetValue() const { ASSERT(IsSet()); return Value; }
};