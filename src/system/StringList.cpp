#include "StdAfx.h"

int cStringVector::FindIndex(const std::string &Token, int From) const
{
	for(int i=From, iend=(int)size(); i<iend; ++i)
		if((*this)[i]==Token)
			return i;
	return -1;
}

std::string cStringVector::ToString(const std::string &Separator) const
{
	std::string Result;
	for(const_iterator i=begin(), iend=end(); i!=iend; ++i)
	{
		if(Result.length())
			Result+=fmt::sprintf("%s%s", Separator, *i);
		else
			Result=*i;
	}
	return Result;
}

void cStringVector::FromIntVector(const cIntVector &IntVector)
{
	resize(IntVector.size());
	for(int i=0, iend=(int)size(); i!=iend; ++i)
		(*this)[i]=fmt::sprintf("%d", IntVector[i]);
}

cIntVector cStringVector::ToIntVector() const
{
	cIntVector IntVector;
	IntVector.resize(size());
	for(int i=0, iend=(int)size(); i!=iend; ++i)
		IntVector[i]=atoi((*this)[i].c_str());
	return IntVector;
}

void cStringVector::TrimAll()
{
	for(size_t i=0, iend=size(); i!=iend; ++i)
		Trim((*this)[i]);
}

cStringVector cStringVector::operator+(const std::string &ExtraField) const
{
	cStringVector ExtendedVector=*this;
	ExtendedVector.push_back(ExtraField);
	return ExtendedVector;
}

