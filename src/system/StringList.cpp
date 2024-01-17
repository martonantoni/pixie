#include "StdAfx.h"

namespace
{
	template<class T> void AddFields(T &Container, const std::string &SourceString, const std::string &Delimeters, int EmptyFieldsAllowed)
	{
		const char *SourcePos=SourceString.c_str();
		if(Delimeters.length()>1)
		{
			for(;;)
			{
				const char *DelimeterPos=strpbrk(SourcePos, Delimeters.c_str());
				if(!DelimeterPos)
					DelimeterPos=SourceString.c_str()+SourceString.length();
				if(EmptyFieldsAllowed||DelimeterPos-SourcePos>0)
					Container.push_back(std::string(SourcePos, (int)(DelimeterPos-SourcePos)));
				if(!*DelimeterPos)
					break;
				SourcePos=DelimeterPos+1;
			}
		}
		else
		{
			char DelimeterChar=Delimeters[0];
			for(;;)
			{
				const char *DelimeterPos=strchr(SourcePos, DelimeterChar);
				if(!DelimeterPos)
					DelimeterPos=SourceString.c_str()+SourceString.length();
				if(EmptyFieldsAllowed||DelimeterPos-SourcePos>0)
					Container.push_back(std::string(SourcePos, (int)(DelimeterPos-SourcePos)));
				if(!*DelimeterPos)
					break;
				SourcePos=DelimeterPos+1;
			}
		}
	}
}

cStringVector::cStringVector(const std::string &SourceString, const std::string &Delimeters, bool EmptyFieldsAllowed)
{
	reserve(4);
	AddFields(*this, SourceString, Delimeters, EmptyFieldsAllowed);
}

void cStringVector::FromString(const std::string &SourceString, const std::string &Delimeters, bool EmptyFieldsAllowed)
{
	clear();
	reserve(4);
	AddFields(*this, SourceString, Delimeters, EmptyFieldsAllowed);
}

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

