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

template<class StoredType>
class tStringVector: public std::vector<StoredType>
{
	using base = std::vector<StoredType>;
	template<cStringVectorSupportedSource T> void addFields(const T& source, const std::string& delimeters, bool emptyFieldsAllowed);
	static void trim(StoredType& s);
public:
	tStringVector() {}
	template<cStringContainer T> tStringVector(const T& source);
	template<cStringVectorSupportedSource T> tStringVector(const T &SourceString, const std::string &Delimeters, bool EmptyFieldsAllowed=true);
	template<cStringVectorSupportedSource T> void FromString(const T &source, const std::string &Delimeters, bool EmptyFieldsAllowed=true);
	void TrimAll();
	std::string ToString(const std::string &Separator) const;
	int FindIndex(const std::string &Token, int From=0) const; // returns -1 if not found
	void FromIntVector(const cIntVector &IntVector);
	cIntVector ToIntVector() const;
};

using cStringVector = tStringVector<std::string>;

template<class StoredType>
template<cStringVectorSupportedSource T> 
void tStringVector<StoredType>::addFields(const T& source, const std::string& delimeters, bool emptyFieldsAllowed)
{
	using UsedSourceType = typename std::conditional<std::is_same_v<T, std::string>, const std::string&, std::string_view>::type;
	UsedSourceType sourceView(source);
	std::string::size_type start = 0;
    std::string::size_type end = sourceView.find_first_of(delimeters);
    while(end != std::string::npos)
    {
        if(emptyFieldsAllowed || end > start)
			this->emplace_back(sourceView.substr(start, end - start));
        start = end + 1;
        end = sourceView.find_first_of(delimeters, start);
    }
    if(emptyFieldsAllowed || start < sourceView.size())
		this->emplace_back(sourceView.substr(start));
}

template<class StoredType>
template<cStringVectorSupportedSource T>
tStringVector<StoredType>::tStringVector(const T& source, const std::string& delimeters, bool emptyFieldsAllowed)
{
	this->reserve(4);
	addFields(source, delimeters, emptyFieldsAllowed);
}

template<class StoredType>
template<cStringVectorSupportedSource T>
void tStringVector<StoredType>::FromString(const T& source, const std::string& delimeters, bool emptyFieldsAllowed)
{
	this->clear(); this->reserve(4);
	addFields(source, delimeters, emptyFieldsAllowed);
}

template<class StoredType>
int tStringVector<StoredType>::FindIndex(const std::string& Token, int From) const
{
	for (int i = From, iend = (int)this->size(); i < iend; ++i)
		if ((*this)[i] == Token)
			return i;
	return -1;
}

template<class StoredType>
std::string tStringVector<StoredType>::ToString(const std::string& Separator) const
{
	std::string Result;
	for (typename base::const_iterator i = this->begin(), iend = this->end(); i != iend; ++i)
	{
		if (Result.length())
			Result += fmt::sprintf("%s%s", Separator, *i);
		else
			Result = *i;
	}
	return Result;
}

template<class StoredType>
void tStringVector<StoredType>::FromIntVector(const cIntVector& IntVector)
{
	this->resize(IntVector.size());
	for (int i = 0, iend = (int)this->size(); i != iend; ++i)
		(*this)[i] = fmt::sprintf("%d", IntVector[i]);
}

template<class StoredType>
cIntVector tStringVector<StoredType>::ToIntVector() const
{
	cIntVector IntVector;
	IntVector.resize(this->size());
	for (int i = 0, iend = (int)this->size(); i != iend; ++i)
		IntVector[i] = atoi((*this)[i].c_str());
	return IntVector;
}

template<class StoredType>
void tStringVector<StoredType>::trim(StoredType& s)
{
	auto left = std::find_if_not(s.begin(), s.end(), [](int c) { return std::isspace(c); });
	auto right = std::find_if_not(s.rbegin(), s.rend(), [](int c) { return std::isspace(c); });
	s = s.substr(left - s.begin(), right.base() - left);
}

template<class StoredType>
void tStringVector<StoredType>::TrimAll()
{
	for(auto& s: *this)
        trim(s);
}

template<class StoredType>
template<cStringContainer T> tStringVector<StoredType>::tStringVector(const T& source)
{
    for(auto &s: source)
		this->emplace_back(s);
}