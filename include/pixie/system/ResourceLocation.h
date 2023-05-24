#pragma once

struct cResourceLocation: public cStringVector
{
public:
	cResourceLocation() {}
	cResourceLocation(const std::string &Location): cStringVector(Location,".",false) {}
	cResourceLocation(const char *Location): cStringVector(Location,".",false) {}
	void FromString(const std::string &Location)
	{
		cStringVector::FromString(Location, ".", false);
	}
	bool IsValid() const { return !empty(); }
	cResourceLocation &operator+=(const std::string &SubLocation)
	{
		cStringVector SubLocationTokens(SubLocation, ".", false);
		std::move(SubLocationTokens.begin(), SubLocationTokens.end(), std::back_inserter(*this));
		return *this;
	}
};


