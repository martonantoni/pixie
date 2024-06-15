#pragma once

template<class T> concept cStringContainer = requires(T t) 
{ 
	t.begin(); 
	t.end(); 
	*t.begin(); 
	std::string(*t.begin()); 
};

template<class T> concept cStringVectorSupportedSource = 
	std::same_as<std::decay_t<T>, std::string> ||
	std::same_as<std::decay_t<T>, std::string_view> ||
	std::is_convertible_v<T, const char*>;

//template<class StoredType>
class cStringVector: public std::vector<std::string>
{
	template<cStringVectorSupportedSource T> void addFields(const T& source, const std::string& delimeters, bool emptyFieldsAllowed);
public:
	cStringVector() {}
	template<cStringContainer T> cStringVector(const T& source);
	template<cStringVectorSupportedSource T> cStringVector(const T &SourceString, const std::string &Delimeters, bool EmptyFieldsAllowed=true);
	template<cStringVectorSupportedSource T> void FromString(const T &source, const std::string &Delimeters, bool EmptyFieldsAllowed=true);
	void TrimAll();
	std::string ToString(const std::string &Separator) const;
	int FindIndex(const std::string &Token, int From=0) const; // returns -1 if not found
	void FromIntVector(const cIntVector &IntVector);
	cIntVector ToIntVector() const;
	cStringVector operator+(const std::string &ExtraField) const;
};

template<cStringVectorSupportedSource T> void cStringVector::addFields(const T& source, const std::string& delimeters, bool emptyFieldsAllowed)
{
	using UsedSourceType = typename std::conditional<std::is_same_v<T, std::string>, const std::string&, std::string_view>::type;
	UsedSourceType sourceView(source);
	std::string::size_type start = 0;
    std::string::size_type end = sourceView.find_first_of(delimeters);
    while(end != std::string::npos)
    {
        if(emptyFieldsAllowed || end > start)
            emplace_back(sourceView.substr(start, end - start));
        start = end + 1;
        end = sourceView.find_first_of(delimeters, start);
    }
    if(emptyFieldsAllowed || start < sourceView.size())
        emplace_back(sourceView.substr(start));
}

template<cStringVectorSupportedSource T>
cStringVector::cStringVector(const T& source, const std::string& delimeters, bool emptyFieldsAllowed)
{
	reserve(4);
	addFields(source, delimeters, emptyFieldsAllowed);
}

template<cStringVectorSupportedSource T>
void cStringVector::FromString(const T& source, const std::string& delimeters, bool emptyFieldsAllowed)
{
	clear(); reserve(4);
	addFields(source, delimeters, emptyFieldsAllowed);
}


//using cStringVector = tStringVector<std::string>;
//using cStringViewVector = tStringVector<std::string_view>;

template<cStringContainer T> cStringVector::cStringVector(const T& source)
{
    for(auto &s: source)
        emplace_back(s);
}