#pragma once

class cFlowUniqueID final
{
	uint64_t mID=0;
public:
	cFlowUniqueID(uint64_t id): mID(id) {}
	cFlowUniqueID(const std::string &id);
	cFlowUniqueID()=default;
	cFlowUniqueID(const cFlowUniqueID &Other)=default;
	cFlowUniqueID(cFlowUniqueID &&Other)=default;
	cFlowUniqueID &operator=(const cFlowUniqueID &Other)=default;
	cFlowUniqueID &operator=(cFlowUniqueID &&Other)=default;
	bool operator<(const cFlowUniqueID &Other) const { return mID<Other.mID; }
	bool operator==(const cFlowUniqueID &Other) const { return mID==Other.mID; }
	bool operator!=(const cFlowUniqueID &Other) const { return mID!=Other.mID; }
	bool IsValid() const { return mID!=0ull; }
	void Invalidate() { mID=0ull; }
	std::string toString() const;

	void ToStream(cMemoryStream &Stream) const;
	void FromStream(const cMemoryStream &Stream);

	uint64_t GetRawID() const { return mID; }

	template<class T> static cFlowUniqueID Generate_If(T Checker) // Checker has to return true if ID is valid.
	{
		for(;;)
		{
			cFlowUniqueID ID=UnverifiedGenerate();
			if(Checker(ID))
				return ID;
		}
	}
	static cFlowUniqueID UnverifiedGenerate();
};

namespace std
{
	template<> struct hash<cFlowUniqueID>
	{
		typedef cFlowUniqueID argument_type;
		typedef std::size_t result_type;
		result_type operator()(argument_type const &UserID) const
		{
			return std::hash<uint64_t>()(UserID.GetRawID());
		}
	};
}

using cFlowUserID = cFlowUniqueID;
