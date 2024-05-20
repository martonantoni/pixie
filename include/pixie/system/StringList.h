#pragma once

template<class T> concept cStringContainer = requires(T t) 
{ 
	t.begin(); 
	t.end(); 
	*t.begin(); 
	std::string(*t.begin()); 
};

class cStringVector: public std::vector<std::string>
{
public:
	cStringVector() {}
	template<cStringContainer T> cStringVector(const T& source);
	cStringVector(const std::string &SourceString, const std::string &Delimeters, bool EmptyFieldsAllowed=true);
	void FromString(const std::string &SourceString, const std::string &Delimeters, bool EmptyFieldsAllowed=true);
	void TrimAll();
	std::string ToString(const std::string &Separator) const;
	int FindIndex(const std::string &Token, int From=0) const; // returns -1 if not found
	void FromIntVector(const cIntVector &IntVector);
	cIntVector ToIntVector() const;
	cStringVector operator+(const std::string &ExtraField) const;
};

template<cStringContainer T> cStringVector::cStringVector(const T& source)
{
    for(auto &s: source)
        emplace_back(s);
}