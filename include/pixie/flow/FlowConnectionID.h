#pragma once

class cFlowConnectionID
{
	uint64_t mID=0;
	cFlowConnectionID(uint64_t id): mID(id) {}
public:
	cFlowConnectionID()=default;
	cFlowConnectionID(const cFlowConnectionID &Other)=default;
	cFlowConnectionID(cFlowConnectionID &&Other)=default;
	cFlowConnectionID &operator=(const cFlowConnectionID &Other)=default;
	cFlowConnectionID &operator=(cFlowConnectionID &&Other)=default;
	bool operator<(const cFlowConnectionID &Other) const { return mID<Other.mID; }
	bool operator==(const cFlowConnectionID &Other) const { return mID==Other.mID; }
	bool operator!=(const cFlowConnectionID &Other) const { return mID!=Other.mID; }
	bool IsValid() const { return mID!=0; }

	void ToStream(cMemoryStream &Stream) const;
	void FromStream(const cMemoryStream &Stream);

	std::string toString() const;

	static cFlowConnectionID GenerateRandom();
};