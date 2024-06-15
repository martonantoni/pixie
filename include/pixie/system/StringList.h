#pragma once

// Can store either:
// - std::string, the name of the type is then cStringVector
// - std::string_view, the name of the type is then cStringViewVector

// Can be constructed from:
// - std::string
// - std::string_view
// - const char*
// - from a container of strings (e.g. std::vector<std::string>)

template<class T> concept cStringVectorSupportedStoredTypes =
	std::same_as<T, std::string> ||
	std::same_as<T, std::string_view>;

template<class T> concept cStringVectorSupportedSource =
	std::same_as<std::decay_t<T>, std::string> ||
	std::same_as<std::decay_t<T>, std::string_view> ||
	std::is_convertible_v<T, const char*>;

template<class T> concept cStringContainer = requires(T t) 
{ 
	t.begin(); 
	t.end(); 
	*t.begin(); 
	std::string(*t.begin()); 
};

template<cStringVectorSupportedStoredTypes StoredType>
class tStringVector : public std::vector<StoredType>
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
	void FromIntVector(const cIntVector &IntVector) requires std::same_as<StoredType, std::string>;
	cIntVector ToIntVector() const;
};

using cStringVector = tStringVector<std::string>;
using cStringViewVector = tStringVector<std::string_view>;

template<cStringVectorSupportedStoredTypes StoredType>
template<cStringVectorSupportedSource T> 
void tStringVector<StoredType>::addFields(const T& source, const std::string& delimeters, bool emptyFieldsAllowed)
{
	using UsedSourceType = typename std::conditional<
			std::is_same_v<T, std::string> && std::is_same_v<StoredType, std::string>,
			const std::string&, 
			std::string_view>::type; // substr would create a temporary string, that would be bad if we are storing string_views
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

template<cStringVectorSupportedStoredTypes StoredType>
template<cStringVectorSupportedSource T>
tStringVector<StoredType>::tStringVector(const T& source, const std::string& delimeters, bool emptyFieldsAllowed)
{
	this->reserve(4);
	addFields(source, delimeters, emptyFieldsAllowed);
}

template<cStringVectorSupportedStoredTypes StoredType>
template<cStringVectorSupportedSource T>
void tStringVector<StoredType>::FromString(const T& source, const std::string& delimeters, bool emptyFieldsAllowed)
{
	this->clear(); this->reserve(4);
	addFields(source, delimeters, emptyFieldsAllowed);
}

template<cStringVectorSupportedStoredTypes StoredType>
int tStringVector<StoredType>::FindIndex(const std::string& Token, int From) const
{
	for (int i = From, iend = (int)this->size(); i < iend; ++i)
		if ((*this)[i] == Token)
			return i;
	return -1;
}

template<cStringVectorSupportedStoredTypes StoredType>
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

template<cStringVectorSupportedStoredTypes StoredType>
void tStringVector<StoredType>::FromIntVector(const cIntVector& IntVector) requires std::same_as<StoredType, std::string>
{
	this->resize(IntVector.size());
	for (int i = 0, iend = (int)this->size(); i != iend; ++i)
		(*this)[i] = fmt::sprintf("%d", IntVector[i]);
}

template<cStringVectorSupportedStoredTypes StoredType>
cIntVector tStringVector<StoredType>::ToIntVector() const
{
	cIntVector IntVector;
	IntVector.resize(this->size());
	for (auto&& [idx, s] : std::views::enumerate(*this))
	{
		if constexpr (std::is_same_v<StoredType, std::string>)
		{
			IntVector[idx] = std::stoi(s);
		}
		else
		{
			IntVector[idx] = 0;
			std::string_view sv(s);
			trim(sv);
			if (sv.empty())
				continue;
			int isNegative = sv[0] == '-';
			if (isNegative)
                sv.remove_prefix(1);
			if (sv.empty())
				continue;
			if(sv[0] < '0' || sv[0] > '9')
                throw std::runtime_error("Invalid number format");
			for(auto c: sv)
            {
				if (c < '0' || c > '9')
				{
					break;
				}
                IntVector[idx] = IntVector[idx] * 10 + c - '0';
            }
			if (isNegative)
				IntVector[idx] = -IntVector[idx];
		}
	}
	return IntVector;
}

template<cStringVectorSupportedStoredTypes StoredType>
void tStringVector<StoredType>::trim(StoredType& s)
{
	auto left = std::find_if_not(s.begin(), s.end(), [](int c) { return std::isspace(c); });
	auto right = std::find_if_not(s.rbegin(), s.rend(), [](int c) { return std::isspace(c); });
	s = s.substr(left - s.begin(), right.base() - left);
}

template<cStringVectorSupportedStoredTypes StoredType>
void tStringVector<StoredType>::TrimAll()
{
	for(auto& s: *this)
        trim(s);
}

template<cStringVectorSupportedStoredTypes StoredType>
template<cStringContainer T> tStringVector<StoredType>::tStringVector(const T& source)
{
    for(auto &s: source)
		this->emplace_back(s);
}