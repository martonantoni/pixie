#pragma once

class cStringVector: public std::vector<std::string> // could be in its separate header file, but I don't care.
{
public:
	cStringVector() {}
	cStringVector(const std::string &SourceString, const std::string &Delimeters, bool EmptyFieldsAllowed=true);
	void FromString(const std::string &SourceString, const std::string &Delimeters, bool EmptyFieldsAllowed=true);
	void TrimAll();
	std::string ToString(const std::string &Separator) const;
	int FindIndex(const std::string &Token, int From=0) const; // returns -1 if not found
	void FromIntVector(const cIntVector &IntVector);
	cIntVector ToIntVector() const;
	cStringVector &&operator+(const std::string &ExtraField) const;
};
