#pragma once

template<class T> concept cStringContainer = requires(T t) 
{ 
	t.begin(); 
	t.end(); 
	*t.begin(); 
	std::string(*t.begin()); 
};

//template<class StoredType>
class cStringVector: public std::vector<std::string>
{
	template<class T> void addFields(const T& source, const std::string& delimeters, bool emptyFieldsAllowed);
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

template<class T> void cStringVector::addFields(const T& source, const std::string& delimeters, bool emptyFieldsAllowed)
{
	std::string::size_type start = 0;
    std::string::size_type end = source.find_first_of(delimeters);
    while(end != std::string::npos)
    {
        if(emptyFieldsAllowed || end > start)
            emplace_back(source.substr(start, end - start));
        start = end + 1;
        end = source.find_first_of(delimeters, start);
    }
    if(emptyFieldsAllowed || start < source.size())
        emplace_back(source.substr(start));
}



//using cStringVector = tStringVector<std::string>;
//using cStringViewVector = tStringVector<std::string_view>;

template<cStringContainer T> cStringVector::cStringVector(const T& source)
{
    for(auto &s: source)
        emplace_back(s);
}